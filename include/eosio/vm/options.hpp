#pragma once

#include <cstdint>

namespace eosio { namespace vm {

struct options {
   std::uint32_t max_mutable_global_bytes;
   std::uint32_t max_table_elements;
   std::uint32_t max_section_elements;
   std::uint32_t max_linear_memory_init;
   std::uint32_t max_func_local_bytes;
   std::uint32_t max_local_sets;
   std::uint32_t max_nested_structures;
   std::uint32_t max_br_table_elements;
};

struct default_options {
   static constexpr std::uint32_t max_mutable_global_bytes = 1024;
   static constexpr std::uint32_t max_table_elements = 1024;
};

struct eosio_options {
   static constexpr std::uint32_t max_mutable_global_bytes = 1024;
   static constexpr std::uint32_t max_table_elements = 1024;
   static constexpr std::uint32_t max_section_elements = 1024;
   static constexpr std::uint32_t max_linear_memory_init = 64*1024;
   static constexpr std::uint32_t max_func_local_bytes = 8192;
   static constexpr std::uint32_t max_local_sets = 1024;
   // static constexpr std::uint32_t max_nested_structures;
   static constexpr std::uint32_t max_br_table_elements = 8191;
};

}}
