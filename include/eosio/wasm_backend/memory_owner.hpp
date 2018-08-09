#pragma once

#include <eosio/wasm_backend/allocator.hpp>

namespace eosio { namespace wasm_backend {
   class memory_owner { 
      public:
         memory_owner( wasm_allocator& allocator ) : _allocator(allocator) {}
         wasm_allocator& get_allocator() { return _allocator; }
      private:
         wasm_allocator& _allocator;
   };
}} // namespace eosio::wasm_backend
