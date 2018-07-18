#pragma once

#include <vector>

#include <eosio/wasm_backend/constants.hpp>

namespace eosio { namespace wasm_backend {
   struct stack {
      std::vector<uint8_t> memory;
      size_t max = constant::max_stack;
   };

   struct heap {
      std::vector<uint8_t> memory;
      size_t max = constant::max_heap;
   };

   struct global {
      std::vector<uint8_t> memory;
      size_t max = constant::max_global;
   };
}} // namespace eosio::wasm_backend
