#include <eosio/wasm_backend/parser.hpp>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/constants.hpp>
#include <eosio/wasm_backend/sections.hpp>

namespace eosio { namespace wasm_backend {
   // TODO: figure out if we can omit this. currently just ignore 
   size_t binary_parser::parse_custom_section( const wasm_code& code, size_t index ) {
      const uint8_t* raw = code.data()+index;
      return 1+sizeof(uint32_t)+*((uint32_t*)raw);
   }

   void binary_parser::parse_type_section( wasm_code_ptr& code, binary_parser_vec<func_type>& types ) {
      auto type_cnt = parse_varuint<32>( code );
      types.resize( type_cnt );
      for ( int i=0; i < type_cnt; i++ ) {
         func_type ft;
         ft.form = *code++;
         ft.param_count = parse_varuint<32>( code );
         ft.param_types.resize(ft.param_count);
         for ( int j=0; j < ft.param_count; j++ ) {
            ft.param_types[j] = *code++;
         }
         ft.return_count = *code++;
         if ( ft.return_count > 0 ) {
            ft.return_type = *code++;
         }
         types.push_back( ft );
      } 
   }
   
   init_expr binary_parser::parse_init_expr( wasm_code_ptr& code ) {
      init_expr ie;
      ie.opcode = *code++;
      switch ( ie.opcode ) {
         case opcode::i32_const:
            ie.value.i32 = parse_varuint<32>( code );
            break;
         case opcode::i64_const:
            ie.value.i64 = parse_varuint<64>( code );
            break;
         case opcode::f32_const:
            ie.value.f32 = *((uint32_t*)code.raw());
            code += sizeof(uint32_t);
            break;
         case opcode::f64_const:
            ie.value.f64 = *((uint64_t*)code.raw());
            code += sizeof(uint64_t);
            break;
         default:
            EOS_WB_ASSERT(false, wasm_illegal_opcode_exception, "initializer expression can only acception i32.const, i64.const, f32.const and f64.const");
      }
      return ie;
   }

   global_variable binary_parser::parse_global_variable( wasm_code_ptr& code ) {
      global_variable gv;
      gv.type.content_type = *code++; 
      gv.type.mutability = *code++;
      return gv;
   } 
   
   memory_type binary_parser::parse_memory_type( wasm_code_ptr& code ) {
      memory_type mt; 
      mt.limits.flags = *code++;
      mt.limits.initial = parse_varuint<32>( code );
      if (mt.limits.flags) {
         mt.limits.maximum = parse_varuint<32>( code );
      }
      return mt;
   }

   table_type binary_parser::parse_table_type( wasm_code_ptr& code ) {
      table_type tt;
      tt.element_type = *code++;
      tt.limits.flags = *code++;
      tt.limits.initial = parse_varuint<32>( code );
      if (tt.limits.flags) {
         tt.limits.maximum = parse_varuint<32>( code );
      }
      return tt;
   } 

   import_entry binary_parser::parse_import_entry( wasm_code_ptr& code ) {
      import_entry entry;
      auto len = parse_varuint<32>( code );
      entry.module_len = len;
      entry.module_str.resize(entry.module_len);
      memcpy( (char*)entry.module_str.data(), code.raw(), entry.module_len );
      code += entry.module_len;
      len = parse_varuint<32>( code );
      entry.field_len = len;
      entry.field_str.resize(entry.field_len);
      memcpy( (char*)entry.field_str.data(), code.raw(), entry.field_len );
      code += entry.field_len;
      entry.kind = (external_kind)(*code++);
      auto type = parse_varuint<32>( code );
      switch ((uint8_t)entry.kind) {
         case external_kind::Function:
            entry.type.func_t = type;
            break;
         default: 
            EOS_WB_ASSERT(false, wasm_unsupported_import_exception, "only function imports are supported");
      }
      return entry;
   }

}} // namespace eosio::wasm_backend
