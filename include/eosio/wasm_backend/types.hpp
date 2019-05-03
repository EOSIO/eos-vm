#pragma once

/*
 * definitions from https://github.com/WebAssembly/design/blob/master/BinaryEncoding.md
 */

#include <string>
#include <cstring>
#include <vector>
#include <eosio/wasm_backend/leb128.hpp>
#include <eosio/wasm_backend/utils.hpp>
#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/vector.hpp>
#include <eosio/wasm_backend/opcodes.hpp>

namespace eosio { namespace wasm_backend {
   enum types {
      i32 = 0x7f,
      i64 = 0x7e,
      f32 = 0x7d,
      f64 = 0x7c,
      anyfunc = 0x70,
      func    = 0x60,
      pseudo  = 0x40,
      ret_void
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

   template <typename T, typename B>
   using guarded_vector = managed_vector<T, B>;

   struct activation_frame {
      uint32_t pc;
      uint32_t offset;
      uint32_t index;
      uint16_t op_index;
      uint8_t  ret_type;
   };

   struct resizable_limits {
      bool  flags;
      uint32_t initial;
      uint32_t maximum = 0;
   };

   template <typename B>
   struct func_type {
      value_type                    form;  // value for the func type constructor
      uint32_t                      param_count;
      guarded_vector<value_type, B> param_types;
      uint8_t                       return_count;
      value_type                    return_type;
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
   template <typename B> 
   struct import_entry {
      guarded_vector<uint8_t, B> module_str;
      guarded_vector<uint8_t, B> field_str;
      external_kind kind;
      import_type   type;
   };

   template <typename B> 
   struct export_entry {
      guarded_vector<uint8_t, B> field_str;
      external_kind kind;
      uint32_t      index;
   };

   template <typename B>
   struct elem_segment {
      uint32_t index;
      init_expr offset;
      guarded_vector<uint32_t, B> elems;
   };
   
   struct local_entry {
      uint32_t count;
      value_type type;
   };

   template <typename B>
   struct function_body {
      uint32_t body_size;
      uint32_t local_count;
      guarded_vector<local_entry, B> locals;
      guarded_vector<opcode, B> code;
   };

   template <typename B>
   struct data_segment {
      uint32_t  index;
      init_expr offset;
      uint32_t  size;
      guarded_vector<uint8_t, B> data;
   };

   using wasm_code          = std::vector<uint8_t>;
   using wasm_code_ptr      = guarded_ptr<uint8_t>;

   template <typename Backend>
   struct module {
      module(Backend& backend) :
         types(backend, 0),
         imports(backend, 0),
         functions(backend, 0),
         tables(backend, 0),
         memories(backend, 0),
         globals(backend, 0),
         exports(backend, 0),
         start(0),
         elements(backend, 0),
         code(backend, 0),
         data(backend, 0),
         import_functions(backend, 0),
         function_sizes(backend, 0) {}

      uint32_t                              start;
      guarded_vector<func_type<B>, B>       types;
      guarded_vector<import_entry<B>, B>    imports;
      guarded_vector<uint32_t, B>           functions;
      guarded_vector<table_type, B>         tables;
      guarded_vector<memory_type, B>        memories;
      guarded_vector<global_variable, B>    globals;
      guarded_vector<export_entry<B>, B>    exports;
      guarded_vector<elem_segment<B>, B>    elements;
      guarded_vector<function_body<B>, B>   code;
      guarded_vector<data_segment<B>, B>    data;
      guarded_vector<uint32_t, B>           import_functions;  // not part of the spec for WASM, used for the mappings to host functions
      guarded_vector<uint32_t, B>           function_sizes;    // not part of the spec for WASM, used for caching total function sizes at function N
      uint32_t get_imported_functions_size()const {
         uint32_t number_of_imports = 0;
         for (int i=0; i < imports.size(); i++) {
            if (imports[i].kind == external_kind::Function)
               number_of_imports++;
         }
         return number_of_imports;
      }
      inline uint32_t get_functions_size()const {
         return code.size();
      }
      inline uint32_t get_functions_total()const {
         return get_imported_functions_size() + get_functions_size();
      }
      func_type<B> get_function_type(uint32_t index)const {
         if (index < get_imported_functions_size())
            return types[imports[index].type.func_t];
         return types[functions[index]];
      } 
      uint32_t get_exported_function(const std::string_view str) {
         uint32_t index = std::numeric_limits<uint32_t>::max();
         for (int i=0; i < exports.size(); i++) {
            if (exports[i].kind == external_kind::Function &&
                exports[i].field_str.size() == str.size() &&
                memcmp((const char*)str.data(), (const char*)exports[i].field_str.raw(), exports[i].field_str.size()) == 0) {
               index = exports[i].index;
               break;
            }
         }
         return index;
      }
   };

}} // namespace eosio::wasm_backend
