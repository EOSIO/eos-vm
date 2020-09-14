#pragma once

#include <eosio/vm/exceptions.hpp>
#include <eosio/vm/utils.hpp>
#include <eosio/vm/debug_info.hpp>
#include <vector>
#include <algorithm>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

namespace eosio::vm {

struct profile_data {
   // buffer_size is the size of the I/O write buffer.  The buffer must be at least large enough for one item.
   // hash_table_size is the maximum number of unique traces that can be stored in memory.  It must be a power of 2.
   template<typename Backend>
   profile_data(const std::string& file, Backend& bkend, const std::size_t buffer_size = 65536, std::size_t hash_table_size = 1024) :
      addr_map(bkend.get_debug()),
      outbuf_storage(buffer_size),
      items_storage(hash_table_size),
      table_storage(hash_table_size) // load factor of 1
   {
      init_backtrace(bkend.get_context());

      outbuf = outbuf_storage.data();
      outpos = 0;
      outsize = outbuf_storage.size();

      list_header* prev = &mru_list;
      for(auto& item : items_storage) {
         item.mru.prev = prev;
         item.mru.next = reinterpret_cast<list_header*>(&item + 1); // Avoid undefined behavior at the end
         prev = &item.mru;
      }
      items_storage.back().mru.next = &mru_list;
      mru_list.next = &items_storage.front().mru;
      mru_list.prev = &items_storage.back().mru;

      table = table_storage.data();
      table_size = table_storage.size();

      fd = open(file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0755);
      write_header();
   }
   ~profile_data() {
      flush_hash();
      write_trailer();
      flush();
      close(fd);
   }
   void write_header() {
      uint32_t header[] = {
         0,
         3,
         0,
         10000,
         0
      };
      write(reinterpret_cast<const char*>(header), sizeof(header));
   }
   void write_trailer() {
      uint32_t trailer[] = { 0, 1, 0 };
      write(reinterpret_cast<const char*>(trailer), sizeof(trailer));
   }
   void flush() {
      for(std::size_t i = 0; i < outpos;) {
         auto res = ::write(fd, outbuf + i, outpos - i);
         if(res == -1) {
            // report_error
            break;
         } else {
            i += res;
         }
      }
      outpos = 0;
   }

   void write(const char* data, std::size_t size) {
      if(size + outpos >= outsize) {
         flush();
      }
      std::memcpy(outbuf + outpos, data, size);
      outpos += size;
   }

   static constexpr std::size_t max_frames = 251;

   struct list_header {
      list_header* next;
      list_header* prev;
      void unlink() {
         next->prev = prev;
         prev->next = next;
      }
   };

   struct item {
      list_header mru;
      item* next;
      uint32_t bucket = 0xFFFFFFFF;
      uint32_t count = 0;
      uint32_t len;
      uint32_t frames[max_frames];
      // std::hash is not async-signal-safe
      std::size_t hash() const {
         // MurmurHash64A
         // Including len gives a multiple of 2.
         static_assert(max_frames % 2 == 1);
         // Not strictly necessary for correctness, but avoids unaligned loads
         static_assert(offsetof(item, len) % 8 == 0);
         constexpr std::uint64_t mul = 0xc6a4a7935bd1e995ull;
         constexpr std::uint64_t seed = 0xbadd00d00ull;
         constexpr auto shift_mix = [](std::uint64_t v) { return v ^ (v >> 47); };
         int word_len = len/2+1; // if len is even, add an extra 0 word.
         uint64_t hash = seed ^ (word_len * 8 * mul);
         const char* base_addr = reinterpret_cast<const char*>(&len);
         for(int i = 0; i < word_len; ++i) {
            std::uint64_t val;
            memcpy(&val, base_addr + 8*i, 8);
            hash = (hash ^ shift_mix(val * mul) * mul) * mul;
         }
         return shift_mix(shift_mix(hash) * mul);
      }
   };

   static bool traces_equal(const item* lhs, const item* rhs) {
      if(lhs->len != rhs->len) {
         return false;
      }
      for(uint32_t i = 0; i < lhs->len; ++i) {
         if(lhs->frames[i] != rhs->frames[i]) {
            return false;
         }
      }
      return true;
   }

   void write(const item* item) {
      write(reinterpret_cast<const char*>(&item->count), (2+item->len) * sizeof(std::uint32_t));
   }

   item* evict_oldest() {
      item* result = reinterpret_cast<item*>(mru_list.prev);
      if(result->bucket != 0xFFFFFFFFu) {
         write(result);
         for(item** entry = &table[result->bucket]; ; entry = &(*entry)->next) {
            if(*entry == result) {
               *entry = result->next;
               break;
            }
         }
         result->bucket = 0xFFFFFFFFu;
      }
      return result;
   }

   void move_to_head(list_header* new_head) {
      new_head->unlink();
      new_head->next = mru_list.next;
      new_head->prev = &mru_list;
      mru_list.next->prev = new_head;
      mru_list.next = new_head;
   }

   // Inserts an item into the hash table OR combines it with an existing entry
   // The new or modified item will be moved to the head of the MRU list.
   void insert_hash(item* new_item) {
      std::size_t hash = new_item->hash();
      std::size_t idx = hash & (table_size - 1);
      for(item* bucket_entry = table[idx]; bucket_entry; bucket_entry = bucket_entry->next) {
         if(traces_equal(new_item, bucket_entry)) {
            ++bucket_entry->count;
            move_to_head(&bucket_entry->mru);
            return;
         }
      }
      new_item->next = table[idx];
      new_item->bucket = idx;
      new_item->count = 1;
      table[idx] = new_item;
      move_to_head(&new_item->mru);
   }

   void flush_hash() {
      for(std::size_t i = 0; i < table_size; ++i) {
         for(item* entry = table[i]; entry; entry = entry->next) {
            write(entry);
            entry->bucket = 0xFFFFFFFF;
         }
         table[i] = nullptr;
      }
   }

   // The signal handler calling this must not use SA_NODEFER
   void handle_tick(void** data, int count) {
      item* entry = evict_oldest();

      int out = 0;
      // Translate addresses to wasm addresses; skip any frames that are outside wasm
      for(int i = 0; i < count && out < max_frames; ++i) {
         auto addr = addr_map.translate(data[i]);
         if(addr != 0xFFFFFFFFu) {
            if(out > 0) {
               ++addr;
            }
            entry->frames[out++] = addr;
         }
      }
      if(out != 0) {
         entry->len = out;
         if(out % 2 == 0) {
            entry->frames[out] = 0; // so hashing can align to a 64-bit boundary
         }
         insert_hash(entry);
      }
   }

   int fd;

   char* outbuf;
   std::size_t outpos;
   std::size_t outsize;

   list_header mru_list;
   item** table;
   std::size_t table_size; // must be a power of 2

   // Backend specific backtrace
   const profile_instr_map& addr_map;
   int (*get_backtrace_fn)(const void*, void**, int, void*);
   const void* exec_context;

   template<typename Context>
   void init_backtrace(const Context& context) {
      get_backtrace_fn = [](const void* ctx, void** data, int len, void* uc) {
         return static_cast<const Context*>(ctx)->backtrace(data, len, uc);
      };
      exec_context = &context;
   }

   // Unsafe to access from a signal handler
   std::vector<char> outbuf_storage;
   std::vector<item> items_storage;
   std::vector<item*> table_storage;
};

inline void profile_handler(int sig, siginfo_t *info, void *);

inline void register_profile_signal_handler_impl() {
   struct sigaction sa;
   sa.sa_sigaction = profile_handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_SIGINFO;
   sigaction(SIGPROF, &sa, nullptr);
}

inline void register_profile_signal_handler() {
   static int init_helper = (register_profile_signal_handler_impl(), 0);
   ignore_unused_variable_warning(init_helper);
}

#define get_backtrace_impl(data, count, uc) ptr->get_backtrace_fn(ptr->exec_context, data, count, uc)

#if USE_POSIX_TIMERS

inline void profile_handler(int sig, siginfo_t *info, void * uc) {
   static_assert(std::atomic<profile_data*>::is_always_lock_free);
   auto * ptr = std::atomic_load(static_cast<std::atomic<profile_data*>*>(info->si_value.sival_ptr));
   if(ptr) {
      int saved_errno = errno;
      void* data[profile_data::max_frames*2]; // Includes both wasm and native frames
      int count = get_backtrace_impl(data, sizeof(data)/sizeof(data[0]), uc);
      ptr->handle_tick(data, count);
      errno = saved_errno;
   }
}

struct profile_manager {
   profile_manager() {
      register_profile_signal_handler();
      sigevent event;
      event.sigev_notify = SIGEV_THREAD_ID;
      event.sigev_signo = SIGPROF;
      event.sigev_value.sival_ptr = &current_data;
      event._sigev_un._tid = gettid();
      int res = timer_create(CLOCK_MONOTONIC, &event, &timer);
      EOS_VM_ASSERT(res == 0, profile_exception, "Failed to start timer");
      struct itimerspec spec;
      spec.it_interval.tv_sec = 0;
      spec.it_interval.tv_nsec = 10000000;
      spec.it_value.tv_sec = 0;
      spec.it_value.tv_nsec = 10000000;
      res = timer_settime(timer, 0, &spec, nullptr);
      EOS_VM_ASSERT(res == 0, profile_exception, "Failed to start timer");
   }
   void start(profile_data* data) {
      EOS_VM_ASSERT(!current_data, profile_exception, "Already profiling in the current thread");
      current_data = data;
   }
   void stop() {
      current_data = nullptr;
   }
   ~profile_manager() {
      current_data = nullptr;
      timer_delete(timer);
   }
   timer_t timer;
   std::atomic<profile_data*> current_data;
};

__attribute__((visibility("default")))
inline std::unique_ptr<profile_manager> per_thread_profile_manager;

struct scoped_profile {
   explicit scoped_profile(profile_data& data) {
      if(!per_thread_profile_manager) {
         per_thread_profile_manager = std::make_unique<profile_manager>();
      }
      per_thread_profile_manager->start(&data);
   }
   ~scoped_profile() {
      per_thread_profile_manager->stop();
   }
};

#else

__attribute__((visibility("default")))
inline thread_local std::atomic<profile_data*> per_thread_profile_data = ATOMIC_VAR_INIT(nullptr);

inline void profile_handler(int sig, siginfo_t* info, void* uc) {
   static_assert(std::atomic<profile_data*>::is_always_lock_free);
   auto * ptr = std::atomic_load(&per_thread_profile_data);
   if(ptr) {
      int saved_errno = errno;
      void* data[profile_data::max_frames*2]; // Includes both wasm and native frames
      int count = get_backtrace_impl(data, sizeof(data)/sizeof(data[0]), uc);
      ptr->handle_tick(data, count);
      errno = saved_errno;
   }
}

struct profile_manager {
   profile_manager() {
      register_profile_signal_handler();
      timer_thread = std::thread([this]{
         auto lock = std::unique_lock(mutex);
         while(!timer_cond.wait_for(lock, std::chrono::milliseconds(10), [&]{ return done; })) {
            for(pthread_t notify : threads_to_notify) {
               pthread_kill(notify, SIGPROF);
            }
         }
      });
   }
   void start(profile_data* data) {
      auto lock = std::lock_guard(mutex);

      per_thread_profile_data = data;
      threads_to_notify.push_back(pthread_self());
   }
   void stop() {
      per_thread_profile_data = nullptr;
      auto lock = std::lock_guard(mutex);
      threads_to_notify.erase(std::find(threads_to_notify.begin(), threads_to_notify.end(), pthread_self()));
   }
   ~profile_manager() {
      {
         auto lock = std::unique_lock(mutex);
         done = true;
      }
      timer_cond.notify_one();
      timer_thread.join();
   }
   static profile_manager& instance() {
      static profile_manager result;
      return result;
   }
   std::mutex mutex;
   std::vector<pthread_t> threads_to_notify;

   std::thread timer_thread;
   bool done = false;
   std::condition_variable timer_cond;
};

class scoped_profile {
 public:
   explicit scoped_profile(profile_data* data) : data(data) {
      if(data) {
         profile_manager::instance().start(data);
      }
   }
   ~scoped_profile() {
      if(data) {
         profile_manager::instance().stop();
      }
   }
 private:
   profile_data* data;
};

#endif

}
