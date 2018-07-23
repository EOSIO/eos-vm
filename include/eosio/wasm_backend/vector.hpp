#pragma once

#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/allocator.hpp>

namespace eosio { namespace wasm_backend {
   
   template <typename T> 
   class wasm_vector {
      public:
         wasm_vector() {}
         wasm_vector( size_t size ) {
   };
}} // namespace eosio::wasm_backend
