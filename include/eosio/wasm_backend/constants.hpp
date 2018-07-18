#pragma once

namespace eosio { namespace wasm_backend {
   struct constant {
      static constexpr uint32_t magic_number = 0x6D736100; // \0asm
      static constexpr uint32_t version      = 0x1;
   };
}} // namespace eosio::wasm_backend
