#pragma once
#include <eosio/wasm_backend/integer_types.hpp>
#include <fc/optional.hpp>

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
   
   struct resizable_limits {
      varuint<1>  flags;
      varuint<32> initial;
      fc::optional<varuint<32>> maximum;
   };

   struct func_type {
      value_type              form;  // value for the func type constructor
      varuint<32>             param_count; 
      std::vector<value_type> param_types;
      varuint<1>                return_count;
      fc::optional<value_type> return_type;
   };

   struct global_type {
      value_type content_type; 
      varuint<1> mutability;
   };
   
   struct table_type {
      elem_type element_type;
      resizable_limits limits;
   };

   struct memory_type {
      resizable_limits limits;
   };
   
   struct external_kind {
      static constexpr uint8_t Function = 0;
      static constexpr uint8_t Table = 1;
      static constexpr uint8_t Memory = 2;
      static constexpr uint8_t Global = 3;
   };

   using wasm_code = std::vector<uint8_t>;
   using wasm_code_iterator = std::vector<uint8_t>::iterator;
   using wasm_bytes = std::vector<uint8_t>;
   
   struct function {
      func_type type;
      wasm_code code;
   };
}} // namespace eosio::wasm_backend
