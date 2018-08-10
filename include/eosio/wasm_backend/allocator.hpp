#pragma once

#include <memory>
#include <eosio/wasm_backend/exceptions.hpp>

namespace eosio { namespace wasm_backend {
   // simple allocator
   class native_allocator {
      public:
         template <typename T>
         T* alloc(size_t size=1) {
            EOS_WB_ASSERT( (sizeof(T)*size)+index < mem_size, wasm_bad_alloc, "wasm failed to allocate" );
            T* ret = (T*)(raw.get()+index);
            index += sizeof(T)*size;
            return ret;
         }
         template <typename T>
         void free() {
            EOS_WB_ASSERT( index <= 0, wasm_double_free, "double free" );
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
            EOS_WB_ASSERT( (sizeof(T)*size)+index < mem_size, wasm_bad_alloc, "wasm failed to allocate" );
            T* ret = (T*)(raw+(sizeof(T)*size)+index);
            index += sizeof(T)*size;
            return ret;
         }
         template <typename T>
         void free() {
            EOS_WB_ASSERT( index <= 0, wasm_double_free, "double free" );
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
  
 }} // namespace eosio::wasm_backend
