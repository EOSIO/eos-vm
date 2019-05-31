#pragma once
#include <eosio/vm/compile_time_string.hpp>

namespace eosio { namespace vm {
   namespace section_id {
      inline static constexpr auto custom_section   = "custom"_c;
      inline static constexpr auto type_section     = "type"_c;
      inline static constexpr auto import_section   = "import"_c;
      inline static constexpr auto function_section = "function"_c;
      inline static constexpr auto table_section    = "table"_c;
      inline static constexpr auto memory_section   = "memory"_c;
      inline static constexpr auto global_section   = "global"_c;
      inline static constexpr auto export_section   = "export"_c;
      inline static constexpr auto start_section    = "start"_c;
      inline static constexpr auto element_section  = "element"_c;
      inline static constexpr auto code_section     = "code"_c;
      inline static constexpr auto data_section     = "data"_c;
   } // namespace section_id

   struct empty_section {};

}} // namespace eosio::vm
