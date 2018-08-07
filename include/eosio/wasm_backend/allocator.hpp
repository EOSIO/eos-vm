#pragma once

#include <memory>
#include <eosio/wasm_backend/exceptions.hpp>

namespace eosio { namespace wasm_backend {
   // simple allocator
   class wasm_allocator {
      public:
         template <typename T>
         T* alloc(size_t size) {
            EOS_WB_ASSERT( (sizeof(T)*size)+index < mem_size, wasm_bad_alloc, "wasm failed to allocate" );
            T* ret = (T*)(raw.get()+(sizeof(T)*size)+index);
            index += sizeof(T)*size;
            return ret;
         }
         wasm_allocator(size_t size) {
            mem_size = size;
            raw = std::unique_ptr<uint8_t[]>(new uint8_t[mem_size]);
            memset(raw.get(), 0, mem_size);
         }
        size_t mem_size;
        std::unique_ptr<uint8_t[]> raw;
        size_t index = 0;
   };

}} // namespace eosio::wasm_backend
