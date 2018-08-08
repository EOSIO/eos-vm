#pragma once

namespace eosio { namespace wasm_backend {
   enum constants {
      magic   = 0x6D736100,
      version = 0x1,
      magic_size   = sizeof(uint32_t),
      version_size = sizeof(uint32_t),
      id_size      = sizeof(uint8_t),
      varuint32_size = 5
   };
}} // namespace eosio::wasm_backend
