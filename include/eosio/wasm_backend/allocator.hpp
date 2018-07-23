#pragma once

#include <memory>
#include <exceptions.hpp>

namespace eosio { namespace wasm_backend {
   // simple allocator
   template <size_t MemSize> 
   class wasm_allocator {
      public:
         template <typename T>
         T* alloc() {
         }
 
     private:
         wasm_allocator() {
            raw = 
         }
        std::unique_ptr<uint8_t> raw;
   };

}} // namespace eosio::wasm_backend
