#include <eosio/wasm_backend/parser.hpp>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/constants.hpp>
#include <eosio/wasm_backend/sections.hpp>

namespace eosio { namespace wasm_backend {
   void binary_parser::parse_init_expr( wasm_code_ptr& code, init_expr& ie ) {
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
      parse_init_expr( code, es.offset );
      uint32_t size = parse_varuint<32>( code );
      es.elems.resize( size );
      for (uint32_t i=0; i < size; i++)
         es.elems.at(i) = parse_varuint<32>( code );
   }

   void binary_parser::parse_global_variable( wasm_code_ptr& code, global_variable& gv ) {
      gv.type.content_type = *code++; 
      gv.type.mutability = *code++;
      parse_init_expr( code, gv.init );
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
   
   void binary_parser::parse_function_body( wasm_code_ptr& code, function_body& fb ) {
      auto body_size = parse_varuint<32>( code );
      auto before = code.offset();
      auto local_cnt = parse_varuint<32>( code );
      fb.locals.resize(local_cnt);
      // parse the local entries
      for ( size_t i=0; i < local_cnt; i++ ) {
         fb.locals.at(i).count = parse_varuint<32>( code );
         fb.locals.at(i).type  = *code++;
      }
      size_t bytes = body_size - (code.offset() - before) - 1; // -1 is 'end' 0xb byte
      fb.code.set( code.raw(), bytes ); 
      memcpy( (char*)fb.code.raw(), (const char*)code.raw(), bytes );
      code += bytes;
      EOS_WB_ASSERT( *code++ == 0x0B, wasm_parse_exception, "failed parsing function body, expected 'end'");
   }

   void binary_parser::parse_data_segment( wasm_code_ptr& code, data_segment& ds ) {
      ds.index  = parse_varuint<32>( code );
      parse_init_expr( code, ds.offset );
      ds.size   = parse_varuint<32>( code );
      ds.data.set( code.raw(), ds.size );
      code += ds.size;
   }
   
   void binary_parser::parse_module( wasm_code& code, module& mod ) {
      wasm_code_ptr code_ptr( code.data(), 0 );
      EOS_WB_ASSERT(parse_magic( code_ptr ) == constants::magic, wasm_parse_exception, "magic number did not match");
      EOS_WB_ASSERT(parse_version( code_ptr ) == constants::version, wasm_parse_exception, "version number did not match");
      for ( int i=0; i < section_id::num_of_elems; i++ ) {
         code_ptr.add_bounds( constants::id_size );
         auto id = parse_section_id( code_ptr );
         code_ptr.add_bounds( constants::varuint32_size );
         auto len = parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds(len);

         switch ( id ) {
            case section_id::custom_section:
               code_ptr += len;
               break;
            case section_id::type_section:
               parse_section<section_id::type_section>( code_ptr, mod.types );
               break;
            case section_id::import_section:
               parse_section<section_id::import_section>( code_ptr, mod.imports );
               break;
            case section_id::function_section:
               parse_section<section_id::function_section>( code_ptr, mod.functions );
               break;
            case section_id::table_section:
               parse_section<section_id::table_section>( code_ptr, mod.tables );
               break;
            case section_id::memory_section:
               parse_section<section_id::memory_section>( code_ptr, mod.memories );
               break;
            case section_id::global_section:
               parse_section<section_id::global_section>( code_ptr, mod.globals );
               break;
            case section_id::export_section:
               parse_section<section_id::export_section>( code_ptr, mod.exports );
               break;
            case section_id::start_section:
               parse_section<section_id::start_section>( code_ptr, mod.start );
               break;
            case section_id::element_section:
               parse_section<section_id::element_section>( code_ptr, mod.elements );
               break;
            case section_id::code_section:
               parse_section<section_id::code_section>( code_ptr, mod.code );
               break;
            case section_id::data_section:
               parse_section<section_id::data_section>( code_ptr, mod.data );
               break;
            default:
               EOS_WB_ASSERT(false, wasm_parse_exception, "error invalid section id");
         }
      }
   }

}} // namespace eosio::wasm_backend
