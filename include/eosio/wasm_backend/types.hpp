#pragma once

/*
 * definitions from https://github.com/WebAssembly/design/blob/master/BinaryEncoding.md
 */

#include <string>
#include <eosio/wasm_backend/leb128.hpp>
#include <eosio/wasm_backend/utils.hpp>
#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/vector.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
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
   
   template <typename T>
   using native_vector = managed_vector<T, memory_manager::types::native>;
   template <typename T>
   using stack64_vector = managed_vector<T, memory_manager::types::stack64>;

   struct resizable_limits {
      bool  flags;
      uint32_t initial;
      uint32_t maximum = 0;
   };
   
   struct func_type {
      value_type                 form;  // value for the func type constructor
      uint32_t                   param_count; 
      native_vector<value_type>  param_types;
      bool                       return_count;
      value_type                 return_type;
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
   
   struct global_variable {
      global_type type;
      init_expr   init; 
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
      native_vector<uint8_t> module_str;      
      uint32_t      field_len;
      native_vector<uint8_t> field_str;
      external_kind kind;
      import_type   type;
   };
   
   struct export_entry {
      uint32_t      field_len;
      native_vector<uint8_t> field_str;
      external_kind kind;
      uint32_t      index;
   };

   struct elem_segment {
      uint32_t index;
      init_expr offset;
      native_vector<uint32_t> elems;
   };
   
   struct local_entry {
      uint32_t count;
      value_type type;
   };

   struct function_body {
      uint32_t body_size;
      uint32_t local_count;
      native_vector<local_entry> locals;
      native_vector<opcode> code;
   };
   
   struct data_segment {
      uint32_t  index;
      init_expr offset;
      uint32_t  size;
      native_vector<uint8_t> data;
   };
   
   struct control {
      uint8_t  ret_type : 8;
      uint8_t opcode    : 8;
   };
   struct control_stack {
      control_stack() { cs.resize( 1000 ); }
      inline void push( uint64_t c ) { cs.at(index++) = c; }
      inline uint64_t peek() { return cs.at(index); }
      inline void pop() { index--; }
      stack64_vector<uint64_t> cs;
      size_t index=0;
   };

   using wasm_code = std::vector<uint8_t>;
   using wasm_code_ptr = guarded_ptr<uint8_t>;
   using wasm_code_iterator = std::vector<uint8_t>::iterator;
   using wasm_bytes = std::vector<uint8_t>;

   struct module {
      module(){}
      native_vector<func_type>       types;
      native_vector<import_entry>    imports;
      native_vector<uint32_t>        functions;
      native_vector<table_type>      tables;
      native_vector<memory_type>     memories;
      native_vector<global_variable> globals;
      native_vector<export_entry>    exports;
      uint32_t                       start;
      native_vector<elem_segment>    elements;
      native_vector<function_body>   code;
      native_vector<data_segment>    data;
      uint32_t                       apply_index;
   };
}} // namespace eosio::wasm_backend
