#pragma once

#include <cstdint>

namespace eosio { namespace vm {

struct options {
   std::uint32_t max_mutable_global_bytes;
   std::uint32_t max_table_elements;
   std::uint32_t max_section_elements;
   std::uint32_t max_type_section_elements;
   std::uint32_t max_import_section_elements;
   std::uint32_t max_function_section_elements;
   std::uint32_t max_global_section_elements;
   std::uint32_t max_export_section_elements;
   std::uint32_t max_element_section_elements;
   std::uint32_t max_data_section_elements;
   // code is the same as functions
   // memory and tables are both 1.
   std::uint32_t max_element_segment_elements;
   std::uint32_t max_data_segment_bytes;
   std::uint32_t max_linear_memory_init;
   std::uint32_t max_func_local_bytes;
   std::uint32_t max_local_sets;
   std::uint32_t max_nested_structures;
   std::uint32_t max_br_table_elements;
   // import and export
   std::uint32_t max_symbol_bytes;
};

struct default_options {
};

struct eosio_options {
   static constexpr std::uint32_t max_mutable_global_bytes = 1024;
   static constexpr std::uint32_t max_table_elements = 1024;
   // max_section_elements in nodeos is a lie.
   static constexpr std::uint32_t max_section_elements = 8191;
   static constexpr std::uint32_t max_function_section_elements = 1023;
   static constexpr std::uint32_t max_import_section_elements = 1023;
   static constexpr std::uint32_t max_element_segment_elements = 8191;
   static constexpr std::uint32_t max_data_segment_bytes = 8191;
   static constexpr std::uint32_t max_linear_memory_init = 64*1024;
   static constexpr std::uint32_t max_func_local_bytes = 8192;
   static constexpr std::uint32_t max_local_sets = 1023;
   static constexpr std::uint32_t eosio_max_nested_structures = 1023;
   static constexpr std::uint32_t max_br_table_elements = 8191;
   static constexpr std::uint32_t max_symbol_bytes = 8191;
};

}}
