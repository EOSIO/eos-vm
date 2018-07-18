#pragma once

#include <eosio/wasm_backend/types.hpp>

namespace eosio { namespace wasm_backend {
   struct type_section {
      varuint<32>            count;
      std::vector<func_type> entries;
   };

   struct import_section {
   };
}} // eosio::wasm_backend
