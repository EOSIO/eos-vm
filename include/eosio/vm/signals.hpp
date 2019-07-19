#pragma once

#include <eosio/vm/exceptions.hpp>

#include <atomic>
#include <cstdlib>
#include <signal.h>
#include <setjmp.h>

namespace eosio { namespace vm {

   inline thread_local std::atomic<sigjmp_buf*> signal_dest = ATOMIC_VAR_INIT(nullptr);
   template<int Sig>
   inline struct sigaction prev_signal_handler;

   inline void signal_handler(int sig, siginfo_t* info, void* uap) {
      sigjmp_buf* dest = std::atomic_load(&signal_dest);
      if (dest) {
         siglongjmp(*dest, sig);
      } else {
         struct sigaction* prev_action;
         switch(sig) {
            case SIGSEGV: prev_action = &prev_signal_handler<SIGSEGV>; break;
            case SIGBUS: prev_action = &prev_signal_handler<SIGBUS>; break;
            case SIGALRM: prev_action = &prev_signal_handler<SIGALRM>; break;
            default: std::abort();
         }
         if (!prev_action) std::abort();
         if (prev_action->sa_flags & SA_SIGINFO) {
            // FIXME: We need to be at least as strict as the original
            // flags and relax the mask as needed.
            prev_action->sa_sigaction(sig, info, uap);
         } else {
            if(prev_action->sa_handler == SIG_DFL) {
               // The default for all three signals is to terminate the process.
               sigaction(sig, prev_action, nullptr);
               raise(sig);
            } else if(prev_action->sa_handler == SIG_IGN) {
               // Do nothing
            } else {
               prev_action->sa_handler(sig);
            }
         }
      }
   }

   inline void setup_signal_handler_impl() {
      struct sigaction sa;
      sa.sa_sigaction = &signal_handler;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = SA_NODEFER | SA_SIGINFO;
      sigaction(SIGSEGV, &sa, &prev_signal_handler<SIGSEGV>);
      sigaction(SIGBUS, &sa, &prev_signal_handler<SIGBUS>);
      sigaction(SIGALRM, &sa, &prev_signal_handler<SIGALRM>);
   }

   inline void setup_signal_handler() {
      static int init_helper = (setup_signal_handler_impl(), 0);
      EOS_WB_ASSERT(signal_dest.is_lock_free(), wasm_interpreter_exception, "Atomic pointers must be lock-free to be async signal safe.");
   }

   /// Call a function with a signal handler installed.  If this thread is
   /// signalled during the execution of f, the function e will be called with
   /// the signal number as an argument.  If f creates any automatic variables
   /// with non-trivial destructors, then it must mask the relevant signals
   /// during the lifetime of these objects or the behavior is undefined.
   ///
   /// signals handled: SIGSEGV, SIGBUS, SIGALRM
   ///
   // Make this noinline to prevent possible corruption of the caller's local variables.
   // It's unlikely, but I'm not sure that it can definitely be ruled out if both
   // this and f are inlined and f modifies locals from the caller.
   template<typename F, typename E>
   [[gnu::noinline]] auto invoke_with_signal_handler(F&& f, E&& e) {
      setup_signal_handler();
      sigjmp_buf dest;
      sigjmp_buf* volatile old_signal_handler = nullptr;
      int sig;
      if((sig = sigsetjmp(dest, 1)) == 0) {
         // Note: Cannot use RAII, as non-trivial destructors w/ longjmp
         // have undefined behavior. [csetjmp.syn]
         //
         // Warning: The order of operations is critical here.
         // We also have to register signal_dest before unblocking
         // signals to make sure that only our signal handler is executed
         // if the caller has previously blocked signals.
         old_signal_handler = std::atomic_exchange(&signal_dest, &dest);
         sigset_t unblock_mask, old_sigmask; // Might not be preserved across longjmp
         sigemptyset(&unblock_mask);
         sigaddset(&unblock_mask, SIGSEGV);
         sigaddset(&unblock_mask, SIGBUS);
         sigaddset(&unblock_mask, SIGALRM);
         pthread_sigmask(SIG_UNBLOCK, &unblock_mask, &old_sigmask);
         try {
            f();
            pthread_sigmask(SIG_SETMASK, &old_sigmask, nullptr);
            std::atomic_store(&signal_dest, old_signal_handler);
         } catch(...) {
            sigset_t new_sigmask;
            pthread_sigmask(SIG_SETMASK, &old_sigmask, &new_sigmask);
            std::atomic_store(&signal_dest, old_signal_handler);
            // FIXME: Audit the code base for this condition and turn it on.
            // It isn't safe to throw without blocking SIGALRM,
            // but an actual failure is rare.  Trap errors here.
            if(!sigismember(&new_sigmask, SIGALRM) && false) {
               abort();
            }
            throw;
         }
      } else {
        if (old_signal_handler) {
            std::atomic_store(&signal_dest, old_signal_handler);
         }
         e(sig);
      }
   }

   // SIGALRM needs to be blocked:
   // - after the watchdog has started, but outside wasm execution.
   // - During the execution of host_functions that create objects
   //   with non-trivial destructors and when handling other exceptions.
   // The correct behavior is different in the two cases.  In the
   // former case, any pending SIGALRM from the watchdog will be canceled
   // on destruction.  In the latter case, SIGALRM will remain blocked
   // during stack unwinding, if an exception is thrown.
   template<bool CancelPendingOnExit, bool RestoreOnException>
   class block_sigalrm {
    public:
      block_sigalrm() {
         sigset_t block_mask;
         sigemptyset(&block_mask);
         sigaddset(&block_mask, SIGALRM);
         pthread_sigmask(SIG_BLOCK, &block_mask, &original_sigmask);
         if constexpr (!RestoreOnException) {
            original_uncaught_exceptions = std::uncaught_exceptions();
         }
      }
      ~block_sigalrm() {
         if constexpr (CancelPendingOnExit) {
            setup_signal_handler(); // If we haven't already set it up, do so now
            sigjmp_buf dest;
            sigjmp_buf* old_signal_handler;
            if(sigsetjmp(dest, 1) == 0) {
               old_signal_handler = std::atomic_exchange(&signal_dest, &dest);
               sigset_t unblock_mask, old_sigmask;
               sigemptyset(&unblock_mask);
               sigaddset(&unblock_mask, SIGALRM);
               pthread_sigmask(SIG_UNBLOCK, &unblock_mask, &old_sigmask);
               pthread_sigmask(SIG_SETMASK, &old_sigmask, nullptr);
            }
            std::atomic_store(&signal_dest, old_signal_handler);
         }
         if (RestoreOnException || original_uncaught_exceptions == std::uncaught_exceptions())
            pthread_sigmask(SIG_SETMASK, &original_sigmask, nullptr);
      }
   private:
      int original_uncaught_exceptions;
      sigset_t original_sigmask;
   };

   using block_sigalrm_outer = block_sigalrm<true, true>;
   using block_sigalrm_inner = block_sigalrm<false, false>;

}} // namespace eosio::vm
