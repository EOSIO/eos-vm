#pragma once
#include <eosio/wasm_backend/integer_types.hpp>

namespace eosio { namespace wasm_backend {
   struct types {
      static constexpr uint8_t i32 = 0x7F;
      static constexpr uint8_t i64 = 0x7E;
      static constexpr uint8_t f32 = 0x7D;
      static constexpr uint8_t f64 = 0x7C;
      static constexpr uint8_t anyfunc = 0x70;
      static constexpr uint8_t func = 0x60;
      static constexpr uint8_t pseudo = 0x40;
   };
   
   using value_type = varint<7>;
   using block_type = varint<7>;
   using elem_type  = varint<7>;
}} // namespace eosio::wasm_backend
