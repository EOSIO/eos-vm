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
      EOS_WB_ASSERT((*code++) == opcode::end, wasm_parse_exception, "no end op found");
      return ie;
   }

   void binary_parser::parse_func_type( wasm_code_ptr& code, func_type& ft ) {
      ft.form = *code++;
      ft.param_count = parse_varuint<32>( code );
      ft.param_types.resize( ft.param_count );
      for ( int i=0; i < ft.param_count; i++ ) 
         ft.param_types.at(i) = *code++;
      ft.return_count = *code++;
      if (ft.return_count > 0)
         ft.return_type = *code++;
   }

   void binary_parser::parse_export_entry( wasm_code_ptr& code, export_entry& entry ) {
      entry.field_len = parse_varuint<32>( code );
      entry.field_str.resize(entry.field_len);
      memcpy( (char*)entry.field_str.raw(), code.raw(), entry.field_len );
      code += entry.field_len;
      entry.kind = (external_kind)(*code++);
      entry.index = parse_varuint<32>( code );
   }
   
   void binary_parser::parse_elem_segment( wasm_code_ptr& code, elem_segment& es ) {
      es.index = parse_varuint<32>( code );
      EOS_WB_ASSERT(es.index == 0, wasm_parse_exception, "only table index of 0 is supported");
      es.offset = parse_init_expr( code );
      uint32_t size = parse_varuint<32>( code );
      es.elems.resize( size );
      for (uint32_t i=0; i < size; i++)
         es.elems.at(i) = parse_varuint<32>( code );
   }

   void binary_parser::parse_global_variable( wasm_code_ptr& code, global_variable& gv ) {
      gv.type.content_type = *code++; 
      gv.type.mutability = *code++;
      gv.init = parse_init_expr( code );
   } 
   
   void binary_parser::parse_memory_type( wasm_code_ptr& code, memory_type& mt ) {
      mt.limits.flags = *code++;
      mt.limits.initial = parse_varuint<32>( code );
      if (mt.limits.flags) {
         mt.limits.maximum = parse_varuint<32>( code );
      }
   }

   void binary_parser::parse_table_type( wasm_code_ptr& code, table_type& tt ) {
      tt.element_type = *code++;
      tt.limits.flags = *code++;
      tt.limits.initial = parse_varuint<32>( code );
      if (tt.limits.flags) {
         tt.limits.maximum = parse_varuint<32>( code );
      }
   } 

   void binary_parser::parse_import_entry( wasm_code_ptr& code, import_entry& entry ) {
      auto len = parse_varuint<32>( code );
      entry.module_len = len;
      entry.module_str.resize(entry.module_len);
      memcpy( (char*)entry.module_str.raw(), code.raw(), entry.module_len );
      code += entry.module_len;
      len = parse_varuint<32>( code );
      entry.field_len = len;
      entry.field_str.resize(entry.field_len);
      memcpy( (char*)entry.field_str.raw(), code.raw(), entry.field_len );
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
   }

}} // namespace eosio::wasm_backend
