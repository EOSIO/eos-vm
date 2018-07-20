#pragma once
#include <string>
#include <eosio/wasm_backend/integer_types.hpp>
#include <eosio/wasm_backend/utils.hpp>
#include <fc/optional.hpp>

namespace eosio { namespace wasm_backend {
   enum types {
      i32 = 0x7f,
      i64 = 0x7e,
      f32 = 0x7d,
      f64 = 0x7c,
      anyfunc = 0x70,
      func    = 0x60,
      pseudo  = 0x40
   };

   enum external_kind {
      Function = 0,
      Table    = 1,
      Memory   = 2,
      Global   = 3
   };

   typedef uint8_t value_type;
   typedef uint8_t block_type;
   typedef uint8_t elem_type;
  
   struct resizable_limits {
      varuint<1>  flags;
      varuint<32> initial;
      fc::optional<varuint<32>> maximum;
   };

   struct func_type {
      value_type              form;  // value for the func type constructor
      uint32_t                param_count; 
      std::vector<value_type> param_types;
      bool                    return_count;
      value_type              return_type;
   };
   
   struct import_entry {
      uint32_t      module_len;
      std::string   module_str;      
      uint32_t      field_len;
      std::string   field_str;
      external_kind kind;
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
   
   using wasm_code = std::vector<uint8_t>;
   using wasm_code_ptr = guarded_ptr<uint8_t>;
   using wasm_code_iterator = std::vector<uint8_t>::iterator;
   using wasm_bytes = std::vector<uint8_t>;
   
   struct function {
      func_type type;
      wasm_code code;
   };
}} // namespace eosio::wasm_backend
