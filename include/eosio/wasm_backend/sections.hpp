#pragma once

#include <eosio/wasm_backend/types.hpp>

namespace eosio { namespace wasm_backend {
   enum section_id {
      custom_section = 0,
      type_section   = 1,
      import_section = 2,
      function_section = 3,
      table_section    = 4,
      memory_section   = 5,
      global_section   = 6,
      export_section   = 7,
      start_section    = 8,
      element_section  = 9,
      code_section     = 10,
      data_section     = 11
   };

   struct type_section {
      varuint<32>            count;
      std::vector<func_type> entries;
   };

   struct import_section {
   };
}} // eosio::wasm_backend
