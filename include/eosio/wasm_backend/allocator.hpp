#pragma once

#include <sys/mman.h>
#include <signal.h>
#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/constants.hpp>

namespace eosio { namespace wasm_backend {
   class bounded_allocator {
      public:
         bounded_allocator(size_t size) {
            mem_size = size;
            raw = std::unique_ptr<uint8_t[]>(new uint8_t[mem_size]);
            memset(raw.get(), 0, mem_size);
         }
         template <typename T>
         T* alloc(size_t size=1) {
            EOS_WB_ASSERT( (sizeof(T)*size)+index <= mem_size, wasm_bad_alloc, "wasm failed to allocate native" );
            T* ret = (T*)(raw.get()+index);
            index += sizeof(T)*size;
            return ret;
         }
         void free() {
            EOS_WB_ASSERT( index > 0, wasm_double_free, "double free" );
            index = 0;
         }
         void reset() { index = 0; }
         size_t mem_size;
         std::unique_ptr<uint8_t[]> raw;
         size_t index = 0;
   };

   class growable_allocator {
      public:
         growable_allocator(size_t size) : buff(size, (char)0 ) {}
         template <typename T>
         T* alloc(size_t size=1) {
            if ( (sizeof(T)*size) + index >= buff.size() ) {
               buff.resize(buff.size()+buff.size()/2);
            }
            T* ret = (T*)(buff.data()+index);
            index += sizeof(T)*size;
            return ret;
         }
         void free() {
            EOS_WB_ASSERT( index > 0, wasm_double_free, "double free" );
            index = 0;
         }
         void reset() { index = 0; }
         std::basic_string<uint8_t> buff;
         size_t index = 0;
   };

   template <typename T>
   class fixed_stack_allocator {
      private:
         T* raw = nullptr;
         size_t max_size = 0;
         void set_up_signals() {
            struct sigaction sa;
            sa.sa_sigaction = [](int sig, siginfo_t*, void*) { throw stack_memory_exception{"stack memory out-of-bounds"}; };
            sigemptyset(&sa.sa_mask);
            sa.sa_flags   = SA_NODEFER | SA_SIGINFO;
            sigaction(SIGSEGV, &sa, NULL);
            sigaction(SIGBUS, &sa, NULL);
         }

      public:
         template <typename U>
         void free() {
            munmap(raw, max_memory);
         }
         fixed_stack_allocator(size_t max_size) : max_size(max_size) {
            set_up_signals();
            raw = (T*)mmap(NULL, max_memory, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
            mprotect(raw, max_size*sizeof(T), PROT_READ|PROT_WRITE);
         }
         inline T* get_base_ptr()const { return raw; }
   };

   class wasm_allocator {
      private:
         uint8_t* raw       = nullptr;
         uint8_t* _previous = raw;
         int32_t page       = 0;

         void set_up_signals() {
            struct sigaction sa;
            sa.sa_sigaction = [](int sig, siginfo_t*, void*) { throw wasm_memory_exception{"wasm memory out-of-bounds"}; };
            sigemptyset(&sa.sa_mask);
            sa.sa_flags   = SA_NODEFER | SA_SIGINFO;
            sigaction(SIGSEGV, &sa, NULL);
            sigaction(SIGBUS, &sa, NULL);
         }

      public:
         template <typename T>
         T* alloc(size_t size=1 /*in pages*/) {
            EOS_WB_ASSERT(page + size <= max_pages, wasm_bad_alloc, "exceeded max number of pages");
            mprotect(raw + (page_size * page), (page_size * size), PROT_READ|PROT_WRITE);
            page += size;
            T* ptr = (T*)_previous;
            _previous = (raw + (page_size * page));
            return ptr;
         }
         void free() {
            munmap(raw, max_memory);
         }
         wasm_allocator() {
            set_up_signals();
            raw = (uint8_t*)mmap(NULL, max_memory, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
            _previous = raw;
            mprotect(raw, page_size, PROT_READ|PROT_WRITE);
            page = 1;
         }
         void reset() {
            uint64_t size = page_size * page;
            _previous = raw;
            memset(raw, 0, size);
            page = 1;
            mprotect(raw, size, PROT_NONE);
            mprotect(raw, page_size, PROT_READ|PROT_WRITE);
         }
         template <typename T>
         inline T* get_base_ptr()const { return raw; }
         inline int32_t get_current_page()const { return page; }
   };
 }} // namespace eosio::wasm_backend
