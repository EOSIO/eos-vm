#pragma once
#include <string>
#include <eosio/wasm_backend/integer_types.hpp>
#include <eosio/wasm_backend/utils.hpp>
#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/vector.hpp>
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
      bool  flags;
      uint32_t initial;
      uint32_t maximum = 0;
   };

   struct func_type {
      value_type              form;  // value for the func type constructor
      uint32_t                param_count; 
      std::vector<value_type> param_types;
      bool                    return_count;
      value_type              return_type;
   };
   
   union expr_value {
      int32_t i32;
      int64_t i64;
      uint32_t f32;
      uint64_t f64;
   };

   struct init_expr {
      int8_t opcode;
      expr_value value;
   };

   struct global_type {
      value_type content_type; 
      bool mutability;
   };
   
   struct global_variable_type {
      global_type type;
      
   }; 

   struct table_type {
      elem_type element_type;
      resizable_limits limits;
   };

   struct memory_type {
      resizable_limits limits;
   };

   union import_type {
      import_type() {}
      uint32_t    func_t;
      table_type  table_t;
      memory_type mem_t;
      global_type global_t;
   };

   struct import_entry {
      uint32_t      module_len;
      std::string   module_str;      
      uint32_t      field_len;
      std::string   field_str;
      external_kind kind;
      import_type   type;
   };
  
   using wasm_code = std::vector<uint8_t>;
   using wasm_code_ptr = guarded_ptr<uint8_t>;
   using wasm_code_iterator = std::vector<uint8_t>::iterator;
   using wasm_bytes = std::vector<uint8_t>;
   
   struct function {
      func_type type;
      wasm_code code;
   };
   
   template <typename T>
   struct module {
      module(T& ref) : types(ref), memories(ref) {}
      vector<func_type, T>      types;
      std::vector<import_entry> imports;
      std::vector<uint32_t>     functions;
      std::vector<table_type>   tables;
      vector<memory_type, T>  memories;
      std::vector<global_type>  globals;
   };

   struct wasm_env {
      std::vector<func_type>    types;
      std::vector<import_entry> imports;
      std::vector<uint32_t>     functions;
      std::vector<table_type>   tables;
      std::vector<memory_type>  memories;
   };
}} // namespace eosio::wasm_backend
