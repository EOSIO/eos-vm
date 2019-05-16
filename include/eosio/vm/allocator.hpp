#pragma once

#include <eosio/vm/constants.hpp>
#include <eosio/vm/exceptions.hpp>
#include <eosio/vm/outcome.hpp>

#include <sys/mman.h>
#include <signal.h>
#include <cstring>
#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <tuple>
#include <setjmp.h>

namespace eosio { namespace vm {
    struct allocator_registration {
      using allocator_memory_range = std::tuple<uintptr_t, size_t, jmp_buf>;

      static std::unordered_set<allocator_memory_range> allocators_regions;

      template <typename Allocator>
      static void register_allocator( Allocator&& allocator ) {
        allocators_regions.emplace( allocator.get_base_address(), allocator.get_max_size(), jmp_buf{} );
      }

      template <typename Allocator>
      static void unregister_allocator( Allocator&& allocator ) {
        allocators_regions.erase( {allocator.get_base_address(), allocator.get_max_size(), jmp_buf{} );
      }

      static std::optional<allocator_memory_range&> get( uintptr_t address ) {
         for (auto& amr : allocators_regions )
           if ( std::get<0>(amr) <= address && address < (std::get<0>(amr) + std::get<1>(amr)) )
             return amr;
         return {};
      }
    };

    [[noreturn]] void default_wasm_segfault_handler(int sig, siginfo_t* siginfo, void*) {
       if (const auto& amr = allocator_registration::get( siginfo.si_address ))
         longjmp(std::get<2>(amr), 1);
       else
         raise(sig);
    }

    outcome::result<result_void> setup_signal_handler( ) {
       struct sigaction sa;
       sa.sa_sigaction = &default_wasm_segfault_handler;
       sigemptyset(&sa.sa_mask);
       sa.sa_flags   = SA_NODEFER | SA_SIGINFO;
       sigaction(SIGSEGV, &sa, NULL);
       sigaction(SIGBUS, &sa, NULL);
    }

   class bounded_allocator {
      public:
        static outcome::result<bounded_allocator&&> init(size_t sz) {
           bounded_allocator ba(sz);
           if (LIKELY(_valid))
             return std::move(ba);
           return system_errors::constructor_failure;
        }

        template <typename T>
        outcome::result<T*> alloc(size_t size = 1) {
          EOS_VM_ASSERT((sizeof(T) * size) + _index <= _size, memory_errors::bad_alloc);

          T *ret = (T *)(_raw.get() + _index);
          _index += sizeof(T) * size;
          return ret;
         }

         outcome::result<result_void> free() {
            EOS_VM_ASSERT(_index > 0, memory_errors::double_free);
            _index = 0;
         }

         void reset() { _index = 0; }

         uintptr_t get_base_address()const { return _raw.get(); }
         size_t get_max_size()const { return _size; }

       private:
         bounded_allocator(size_t size) : _size(size) {
           try {
             _raw = std::unique_ptr<char[]>(new char[_size]);
           } catch (...) {
             _valid = false; 
           }
         }
         bool _valid = true;
         size_t _size = 0;
         std::unique_ptr<char[]> _raw = nullptr;
         size_t _index = 0;
   };

   class growable_allocator {
      public:
         static constexpr size_t max_memory_size = 1024 * 1024 * 1024; // 1GB
         static constexpr size_t chunk_size      = 128 * 1024;         // 128KB
         static constexpr size_t align_amt       = 16;
         static constexpr size_t align_offset(size_t offset) {
           return (offset + align_amt-1) & ~(align_amt-1);
         }

         static outcome::result<growable_allocator&&> init(size_t sz) {
            growable_allocator ga(sz);
            if (LIKELY(_valid))
              return std::move(ga);
            return system_errors::constructor_failure;
         }

         ~growable_allocator() {
            if (LIKELY(_valid))
               munmap(_base, max_memory_size);
         }

         template <typename T>
         T* alloc(size_t size=0) {
            size_t aligned = align_offset((sizeof(T)*size)+_offset);
            if ( aligned >= _size ) {
               size_t chunks_to_alloc = aligned / chunk_size;
               EOS_VM_ASSERT_INVALIDATE(mprotect((char*)_base+_size, (chunk_size*chunks_to_alloc), PROT_READ|PROT_WRITE) == -1,
                                        memory_errors::bad_alloc);
               _size += (chunk_size*chunks_to_alloc);
            }
            
            T* ptr = (T*)(_base+_offset);
            _offset = aligned;
            return ptr;
         }

         void free() {
            EOS_VM_ASSERT(false, system_errors::unimplemented_failure);
         }

         void reset() { 
            _offset = 0; 
         }

     private:
         // size in bytes
         growable_allocator(size_t size, bool& failed) : _size((size/chunk_size)) {
           _base = (char*)mmap(NULL, max_memory_size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
           failed = _base == MAP_FAILED;
           if (size != 0) {
             size_t chunks_to_alloc = (size / chunk_size) + 1;
             _size += (chunk_size*chunks_to_alloc);
             failed |= mprotect((char*)_base, _size, PROT_READ|PROT_WRITE) == -1;
           }
         }

         bool _valid = true;
         size_t _offset = 0;
         size_t _size  = 0;
         char* _base;
   };


   class wasm_allocator {
      private:
         bool _valid     = true;
         char* _raw       = nullptr;
         char* _previous = _raw;
         int32_t _page    = 0;
         static long_jump_table 

         void set_up_signals() {
            struct sigaction sa;
            sa.sa_sigaction = [](int sig, siginfo_t*, void*) { throw wasm_memory_exception{"wasm memory out-of-bounds"}; };
            sigemptyset(&sa.sa_mask);
            sa.sa_flags   = SA_NODEFER | SA_SIGINFO;
            sigaction(SIGSEGV, &sa, NULL);
            sigaction(SIGBUS, &sa, NULL);
         }
         wasm_allocator() {
           
         }
      public:
         template <typename T>
         T* alloc(size_t size=1 /*in pages*/) {
            EOS_WB_ASSERT(page + size <= max_pages, wasm_bad_alloc, "exceeded max number of pages");
            mprotect(raw + (page_size * page), (page_size * size), PROT_READ|PROT_WRITE);
            T* ptr = (T*)_previous;
            _previous = (raw + (page_size * page));
            page += size;
            return ptr;
         }
         void free() {
            munmap(raw, max_memory);
         }
         wasm_allocator() {
            //set_up_signals();
            raw = (char*)mmap(NULL, max_memory, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
            _previous = raw;
            mprotect(raw, 3*page_size, PROT_READ|PROT_WRITE);
            page = 3;
         }
         void reset() {
            uint64_t size = page_size * page;
            _previous = raw;
            memset(raw, 0, size);
            page = 3;
            mprotect(raw, size, PROT_NONE);
            mprotect(raw, 3*page_size, PROT_READ|PROT_WRITE);
         }
         template <typename T>
         inline T* get_base_ptr()const { return reinterpret_cast<T*>(raw); }
         inline int32_t get_current_page()const { return page; }
   };
 }} // namespace eosio::vm
