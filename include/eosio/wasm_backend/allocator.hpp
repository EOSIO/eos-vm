#pragma once

#include <sys/mman.h>
#include <signal.h>
#include <cstring>
#include <memory>
#include <iostream>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/constants.hpp>

namespace eosio { namespace wasm_backend {
   class native_allocator {
      public:
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
         native_allocator(size_t size) {
            mem_size = size;
            raw = std::unique_ptr<uint8_t[]>(new uint8_t[mem_size]);
            memset(raw.get(), 0, mem_size);
         }
         size_t mem_size;
         std::unique_ptr<uint8_t[]> raw;
         size_t index = 0;
   };

   class simple_allocator {
      public:
         template <typename T>
         T* alloc(size_t size=1) {
            EOS_WB_ASSERT( (sizeof(T)*size)+index <= mem_size, wasm_bad_alloc, "wasm failed to allocate simple" );
            T* ret = (T*)(raw+index);
            index += sizeof(T)*size;
            return ret;
         }
         void free() {
            EOS_WB_ASSERT( index > 0, wasm_double_free, "double free" );
            index = 0;
         }
         simple_allocator(uint8_t* ptr, size_t size) {
            mem_size = size;
            raw = ptr;
         }
         size_t mem_size;
         uint8_t* raw;
         size_t index = 0;
   };

   class wasm_allocator {
      private:
         uint8_t* raw       = nullptr;
         uint8_t* _previous = raw;
         size_t page        = 0;

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
            mprotect(raw + (page_size * (page+1)), (page_size * size), PROT_READ|PROT_WRITE);
            page += size;
            T* ptr = (T*)_previous;
            _previous = (raw + (page_size * page));
            return ptr;
         }
         void free() {
            // nop
         }
         wasm_allocator() {
            raw = (uint8_t*)mmap(NULL, max_memory, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
            _previous = raw;
            mprotect(raw, page_size, PROT_READ|PROT_WRITE);
            set_up_signals();
         }
         void reset() {
            uint64_t size = page_size * page;
            _previous = raw;
            memset(raw, 0, size);
            page = 0;
            mprotect(raw, size, PROT_NONE);
            mprotect(raw, page_size, PROT_READ|PROT_WRITE);
            set_up_signals();
         }
         inline void* get_base_ptr()const { return (void*)raw; }
         inline uint64_t get_current_page()const { return page; }
   };

   class stack64_allocator {
      public:
         template <typename T>
         T* alloc(size_t size=1) {
            static_assert( sizeof(T) == sizeof(uint64_t), "" );
            EOS_WB_ASSERT( (sizeof(T)*size)+index <= mem_size, wasm_bad_alloc, "wasm failed to allocate s64" );
            T* ret = (T*)(raw+index);
            index += sizeof(T)*size;
            return ret;
         }
         void free() {
            EOS_WB_ASSERT( index > 0, wasm_double_free, "double free" );
            index -= sizeof(uint64_t);
         }
         stack64_allocator(uint8_t* ptr, size_t size) {
            mem_size = size;
            raw = ptr;
         }
         size_t mem_size;
         uint8_t* raw;
         size_t index = 0;
   };
 
 }} // namespace eosio::wasm_backend
