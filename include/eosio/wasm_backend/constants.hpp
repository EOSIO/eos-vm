#pragma once

namespace eosio { namespace wasm_backend {
   enum constants {
      magic   = 0x6D736100,
      version = 0x1,
      magic_size   = sizeof(uint32_t),
      version_size = sizeof(uint32_t),
      id_size      = sizeof(uint8_t),
      varuint32_size = 5,
      max_nested_structures = 1024,
      max_call_depth        = 250,
      max_stack_size        = 8*1024,
      max_memory            = 4ull << 31,
      max_useable_memory    = (33 * 1024 * 1024), //33mb
      page_size             = 64ull * 1024, //64kb
      max_pages             = (max_useable_memory/page_size) - 1 // -1 for the zero page
   };
}} // namespace eosio::wasm_backend
