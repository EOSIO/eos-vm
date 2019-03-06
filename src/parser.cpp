#include <stack>
#include <eosio/wasm_backend/parser.hpp>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/constants.hpp>
#include <eosio/wasm_backend/sections.hpp>

namespace eosio { namespace wasm_backend {
   void binary_parser::parse_init_expr( wasm_code_ptr& code, init_expr& ie ) {
      ie.opcode = *code++;
      switch ( ie.opcode ) {
         case opcodes::i32_const:
            ie.value.i32 = parse_varint32( code );
            break;
         case opcodes::i64_const:
            ie.value.i64 = parse_varint64( code );
            break;
         case opcodes::f32_const:
            ie.value.f32 = *((uint32_t*)code.raw());
            code += sizeof(uint32_t);
            break;
         case opcodes::f64_const:
            ie.value.f64 = *((uint64_t*)code.raw());
            code += sizeof(uint64_t);
            break;
         default:
            EOS_WB_ASSERT(false, wasm_parse_exception, "initializer expression can only acception i32.const, i64.const, f32.const and f64.const");
      }
      EOS_WB_ASSERT((*code++) == opcodes::end, wasm_parse_exception, "no end op found");
   }

   void binary_parser::parse_func_type( wasm_code_ptr& code, func_type& ft ) {
      ft.form = *code++;
      ft.param_count = parse_varuint32( code );
      ft.param_types.resize( ft.param_count );
      for ( size_t i=0; i < ft.param_count; i++ ) {
         uint8_t pt = *code++;
         ft.param_types.at(i) = pt;
         EOS_WB_ASSERT(pt == types::i32 || pt == types::i64 ||
                       pt == types::f32 || pt == types::f64, wasm_parse_exception, "invalid function param type");
      }
      ft.return_count = *code++;
      EOS_WB_ASSERT(ft.return_count < 2, wasm_parse_exception, "invalid function return count");
      if (ft.return_count > 0)
         ft.return_type = *code++;
   }

   void binary_parser::parse_export_entry( wasm_code_ptr& code, export_entry& entry ) {
      entry.field_len = parse_varuint32( code );
      entry.field_str.resize(entry.field_len);
      memcpy( (char*)entry.field_str.raw(), code.raw(), entry.field_len );
      code += entry.field_len;
      entry.kind = (external_kind)(*code++);
      entry.index = parse_varuint32( code );
   }
   
   void binary_parser::parse_elem_segment( wasm_code_ptr& code, elem_segment& es ) {
      es.index = parse_varuint32( code );
      EOS_WB_ASSERT(es.index == 0, wasm_parse_exception, "only table index of 0 is supported");
      parse_init_expr( code, es.offset );
      uint32_t size = parse_varuint32( code );
      es.elems.resize( size );
      for (uint32_t i=0; i < size; i++)
         es.elems.at(i) = parse_varuint32( code );
   }

   void binary_parser::parse_global_variable( wasm_code_ptr& code, global_variable& gv ) {
      uint8_t ct = *code++;
      gv.type.content_type = ct; 
      EOS_WB_ASSERT(ct == types::i32 || ct == types::i64 ||
                    ct == types::f32 || ct == types::f64, wasm_parse_exception, "invalid global content type");

      gv.type.mutability = *code++;
      parse_init_expr( code, gv.init );
   } 
   
   void binary_parser::parse_memory_type( wasm_code_ptr& code, memory_type& mt ) {
      mt.limits.flags = *code++;
      mt.limits.initial = parse_varuint32( code );
      if (mt.limits.flags) {
         mt.limits.maximum = parse_varuint32( code );
      }
   }

   void binary_parser::parse_table_type( wasm_code_ptr& code, table_type& tt ) {
      tt.element_type = *code++;
      tt.limits.flags = *code++;
      tt.limits.initial = parse_varuint32( code );
      if (tt.limits.flags) {
         tt.limits.maximum = parse_varuint32( code );
      }
   } 

   void binary_parser::parse_import_entry( wasm_code_ptr& code, import_entry& entry ) {
      auto len = parse_varuint32( code );
      entry.module_len = len;
      entry.module_str.resize(entry.module_len);
      memcpy( (char*)entry.module_str.raw(), code.raw(), entry.module_len );
      code += entry.module_len;
      len = parse_varuint32( code );
      entry.field_len = len;
      entry.field_str.resize(entry.field_len);
      memcpy( (char*)entry.field_str.raw(), code.raw(), entry.field_len );
      code += entry.field_len;
      entry.kind = (external_kind)(*code++);
      auto type = parse_varuint32( code );
      switch ((uint8_t)entry.kind) {
         case external_kind::Function:
            entry.type.func_t = type;
            break;
         default: 
            EOS_WB_ASSERT(false, wasm_unsupported_import_exception, "only function imports are supported");
      }
   }
   
   void binary_parser::parse_function_body_code( wasm_code_ptr& code, size_t bounds, native_vector<opcode>& fb ) {
      size_t op_index = 0;
      auto parse_br_table = []( wasm_code_ptr& code, br_table_t& bt ) {
         size_t table_size = parse_varuint32( code );
         bt.target_table.resize(table_size);
         for ( size_t i=0; i < table_size; i++ )
            bt.target_table[i] = parse_varuint32( code );
         bt.default_target = parse_varuint32( code );
      };

      std::stack<uint32_t> pc_stack;

      while (code.offset() < bounds) {
         switch ( *code++ ) {
            // CONTROL FLOW OPERATORS
            case opcodes::unreachable:
               fb[op_index++] = unreachable_t{}; break;
            case opcodes::nop:
               fb[op_index++] = nop_t{}; break;
            case opcodes::end:
               {
                  fb[op_index++] = end_t{};
                  if (pc_stack.size()) {
                     auto& el = fb[pc_stack.top()];
                     std::visit(overloaded {
                        [=](block_t& bt) {
                           bt.pc = op_index;
                        }, [=](loop_t& lt) {
                           lt.pc = op_index;
                        }, [=](if__t& it) {
                           it.pc = op_index;
                        }, [=](auto&&) {
                           throw wasm_invalid_element{"invalid element when popping pc stack"};
                        }
                     }, el);
                     pc_stack.pop();
                  }
                  break;
               }
            case opcodes::return_:
               fb[op_index++] = return__t{}; break;
            case opcodes::block:
               pc_stack.push(op_index);
               fb[op_index++] = block_t{*code++, 0};
               break;
            case opcodes::loop:
               pc_stack.push(op_index);
               fb[op_index++] = loop_t{*code++, 0};
               break;
            case opcodes::if_:
               pc_stack.push(op_index);
               fb[op_index++] = if__t{*code++, 0};
               break;
            case opcodes::else_:
               {
                  fb[op_index++] = else__t{};
                  auto& el = fb[pc_stack.top()];
                  std::visit(overloaded {
                     [=](block_t& bt) {
                        bt.pc = op_index;
                     }, [=](loop_t& lt) {
                        lt.pc = op_index;
                     }, [=](if__t& it) {
                        it.pc = op_index;
                     }, [=](auto&&) {
                        throw wasm_invalid_element{"invalid element when popping pc stack"};
                     }
                  }, el);
                  pc_stack.pop();
                  break;
               }
            case opcodes::br:
               fb[op_index++] = br_t{parse_varuint32(code)}; break;
            case opcodes::br_if:
               fb[op_index++] = br_if_t{parse_varuint32(code)}; break;
            case opcodes::br_table:
               {
                  br_table_t bt;
                  parse_br_table( code, bt );
                  fb[op_index++] = bt;
               }
               break;
            case opcodes::call:
               fb[op_index++] = call_t{parse_varuint32(code)}; break;
            case opcodes::call_indirect:
               fb[op_index++] = call_indirect_t{parse_varuint32(code)}; code++; break;
            case opcodes::drop:
               fb[op_index++] = drop_t{}; break;
            case opcodes::select:
               fb[op_index++] = select_t{}; break;
            case opcodes::get_local:
               fb[op_index++] = get_local_t{parse_varuint32(code)}; break;
            case opcodes::set_local:
               fb[op_index++] = set_local_t{parse_varuint32(code)}; break;
            case opcodes::tee_local:
               fb[op_index++] = tee_local_t{parse_varuint32(code)}; break;
            case opcodes::get_global:
               fb[op_index++] = get_global_t{parse_varuint32(code)}; break;
            case opcodes::set_global:
               fb[op_index++] = set_global_t{parse_varuint32(code)}; break;
            case opcodes::i32_load:
               fb[op_index++] = i32_load_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_load:
               fb[op_index++] = i64_load_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::f32_load:
               fb[op_index++] = f32_load_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::f64_load:
               fb[op_index++] = f64_load_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i32_load8_s:
               fb[op_index++] = i32_load8_s_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i32_load16_s:
               fb[op_index++] = i32_load16_s_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i32_load8_u:
               fb[op_index++] = i32_load8_u_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i32_load16_u:
               fb[op_index++] = i32_load16_u_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_load8_s:
               fb[op_index++] = i64_load8_s_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_load16_s:
               fb[op_index++] = i64_load16_s_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_load32_s:
               fb[op_index++] = i64_load32_s_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_load8_u:
               fb[op_index++] = i64_load8_u_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_load16_u:
               fb[op_index++] = i64_load16_u_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_load32_u:
               fb[op_index++] = i64_load32_u_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i32_store:
               fb[op_index++] = i32_store_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_store:
               fb[op_index++] = i64_store_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::f32_store:
               fb[op_index++] = f32_store_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::f64_store:
               fb[op_index++] = f64_store_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i32_store8:
               fb[op_index++] = i32_store8_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i32_store16:
               fb[op_index++] = i32_store16_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_store8:
               fb[op_index++] = i64_store8_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_store16:
               fb[op_index++] = i64_store16_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::i64_store32:
               fb[op_index++] = i64_store32_t{parse_varuint32(code), parse_varuint32(code)}; break;
            case opcodes::current_memory:
               fb[op_index++] = current_memory_t{}; code++; break;
            case opcodes::grow_memory:
               fb[op_index++] = grow_memory_t{}; code++; break;
            case opcodes::i32_const: 
               fb[op_index++] = i32_const_t{parse_varint32(code)}; break;
            case opcodes::i64_const:
               fb[op_index++] = i64_const_t{parse_varint64(code)}; break;
            case opcodes::f32_const:
               fb[op_index++] = f32_const_t{(uint32_t)*code}; code += 4; break;
            case opcodes::f64_const:
               fb[op_index++] = f64_const_t{(uint64_t)*code}; code += 8; break;
            case opcodes::i32_eqz:
               fb[op_index++] = i32_eqz_t{}; break;
            case opcodes::i32_eq:
               fb[op_index++] = i32_eq_t{}; break;
            case opcodes::i32_ne:
               fb[op_index++] = i32_ne_t{}; break;
            case opcodes::i32_lt_s:
               fb[op_index++] = i32_lt_s_t{}; break;
            case opcodes::i32_lt_u:
               fb[op_index++] = i32_lt_u_t{}; break;
            case opcodes::i32_gt_s:
               fb[op_index++] = i32_gt_s_t{}; break;
            case opcodes::i32_gt_u:
               fb[op_index++] = i32_gt_u_t{}; break;
            case opcodes::i32_le_s:
               fb[op_index++] = i32_le_s_t{}; break;
            case opcodes::i32_le_u:
               fb[op_index++] = i32_le_u_t{}; break;
            case opcodes::i32_ge_s:
               fb[op_index++] = i32_ge_s_t{}; break;
            case opcodes::i32_ge_u:
               fb[op_index++] = i32_ge_u_t{}; break;
            case opcodes::i64_eqz:
               fb[op_index++] = i64_eqz_t{}; break;
            case opcodes::i64_eq:
               fb[op_index++] = i64_eq_t{}; break;
            case opcodes::i64_ne:
               fb[op_index++] = i64_ne_t{}; break;
            case opcodes::i64_lt_s:
               fb[op_index++] = i64_lt_s_t{}; break;
            case opcodes::i64_lt_u:
               fb[op_index++] = i64_lt_u_t{}; break;
            case opcodes::i64_gt_s:
               fb[op_index++] = i64_gt_s_t{}; break;
            case opcodes::i64_gt_u:
               fb[op_index++] = i64_gt_u_t{}; break;
            case opcodes::i64_le_s:
               fb[op_index++] = i64_le_s_t{}; break;
            case opcodes::i64_le_u:
               fb[op_index++] = i64_le_u_t{}; break;
            case opcodes::i64_ge_s:
               fb[op_index++] = i64_ge_s_t{}; break;
            case opcodes::i64_ge_u:
               fb[op_index++] = i64_ge_u_t{}; break;
            case opcodes::f32_eq:
               fb[op_index++] = f32_eq_t{}; break;
            case opcodes::f32_ne:
               fb[op_index++] = f32_ne_t{}; break;
            case opcodes::f32_lt:
               fb[op_index++] = f32_lt_t{}; break;
            case opcodes::f32_gt:
               fb[op_index++] = f32_gt_t{}; break;
            case opcodes::f32_le:
               fb[op_index++] = f32_le_t{}; break;
            case opcodes::f32_ge:
               fb[op_index++] = f32_ge_t{}; break;
            case opcodes::f64_eq:
               fb[op_index++] = f64_eq_t{}; break;
            case opcodes::f64_ne:
               fb[op_index++] = f64_ne_t{}; break;
            case opcodes::f64_lt:
               fb[op_index++] = f64_lt_t{}; break;
            case opcodes::f64_gt:
               fb[op_index++] = f64_gt_t{}; break;
            case opcodes::f64_le:
               fb[op_index++] = f64_le_t{}; break;
            case opcodes::f64_ge:
               fb[op_index++] = f64_ge_t{}; break;
            case opcodes::i32_clz:
               fb[op_index++] = i32_clz_t{}; break;
            case opcodes::i32_ctz:
               fb[op_index++] = i32_ctz_t{}; break;
            case opcodes::i32_popcnt:
               fb[op_index++] = i32_popcnt_t{}; break;
            case opcodes::i32_add:
               fb[op_index++] = i32_add_t{}; break;
            case opcodes::i32_sub:
               fb[op_index++] = i32_sub_t{}; break;
            case opcodes::i32_mul:
               fb[op_index++] = i32_mul_t{}; break;
            case opcodes::i32_div_s:
               fb[op_index++] = i32_div_s_t{}; break;
            case opcodes::i32_div_u:
               fb[op_index++] = i32_div_u_t{}; break;
            case opcodes::i32_rem_s:
               fb[op_index++] = i32_rem_s_t{}; break;
            case opcodes::i32_rem_u:
               fb[op_index++] = i32_rem_u_t{}; break;
            case opcodes::i32_and:
               fb[op_index++] = i32_and_t{}; break;
            case opcodes::i32_or:
               fb[op_index++] = i32_or_t{}; break;
            case opcodes::i32_xor:
               fb[op_index++] = i32_xor_t{}; break;
            case opcodes::i32_shl:
               fb[op_index++] = i32_shl_t{}; break;
            case opcodes::i32_shr_s:
               fb[op_index++] = i32_shr_s_t{}; break;
            case opcodes::i32_shr_u:
               fb[op_index++] = i32_shr_u_t{}; break;
            case opcodes::i32_rotl:
               fb[op_index++] = i32_rotl_t{}; break;
            case opcodes::i32_rotr:
               fb[op_index++] = i32_rotr_t{}; break;
            case opcodes::i64_clz:
               fb[op_index++] = i64_clz_t{}; break;
            case opcodes::i64_ctz:
               fb[op_index++] = i64_ctz_t{}; break;
            case opcodes::i64_popcnt:
               fb[op_index++] = i64_popcnt_t{}; break;
            case opcodes::i64_add:
               fb[op_index++] = i64_add_t{}; break;
            case opcodes::i64_sub:
               fb[op_index++] = i64_sub_t{}; break;
            case opcodes::i64_mul:
               fb[op_index++] = i64_mul_t{}; break;
            case opcodes::i64_div_s:
               fb[op_index++] = i64_div_s_t{}; break;
            case opcodes::i64_div_u:
               fb[op_index++] = i64_div_u_t{}; break;
            case opcodes::i64_rem_s:
               fb[op_index++] = i64_rem_s_t{}; break;
            case opcodes::i64_rem_u:
               fb[op_index++] = i64_rem_u_t{}; break;
            case opcodes::i64_and:
               fb[op_index++] = i64_and_t{}; break;
            case opcodes::i64_or:
               fb[op_index++] = i64_or_t{}; break;
            case opcodes::i64_xor:
               fb[op_index++] = i64_xor_t{}; break;
            case opcodes::i64_shl:
               fb[op_index++] = i64_shl_t{}; break;
            case opcodes::i64_shr_s:
               fb[op_index++] = i64_shr_s_t{}; break;
            case opcodes::i64_shr_u:
               fb[op_index++] = i64_shr_u_t{}; break;
            case opcodes::i64_rotl:
               fb[op_index++] = i64_rotl_t{}; break;
            case opcodes::i64_rotr:
               fb[op_index++] = i64_rotr_t{}; break;
            case opcodes::f32_abs:
               fb[op_index++] = f32_abs_t{}; break;
            case opcodes::f32_neg:
               fb[op_index++] = f32_neg_t{}; break;
            case opcodes::f32_ceil:
               fb[op_index++] = f32_ceil_t{}; break;
            case opcodes::f32_floor:
               fb[op_index++] = f32_floor_t{}; break;
            case opcodes::f32_trunc:
               fb[op_index++] = f32_trunc_t{}; break;
            case opcodes::f32_nearest:
               fb[op_index++] = f32_nearest_t{}; break;
            case opcodes::f32_sqrt:
               fb[op_index++] = f32_sqrt_t{}; break;
            case opcodes::f32_add:
               fb[op_index++] = f32_add_t{}; break;
            case opcodes::f32_sub:
               fb[op_index++] = f32_sub_t{}; break;
            case opcodes::f32_mul:
               fb[op_index++] = f32_mul_t{}; break;
            case opcodes::f32_div:
               fb[op_index++] = f32_div_t{}; break;
            case opcodes::f32_min:
               fb[op_index++] = f32_min_t{}; break;
            case opcodes::f32_max:
               fb[op_index++] = f32_max_t{}; break;
            case opcodes::f32_copysign:
               fb[op_index++] = f32_copysign_t{}; break;
            case opcodes::f64_abs:
               fb[op_index++] = f64_abs_t{}; break;
            case opcodes::f64_neg:
               fb[op_index++] = f64_neg_t{}; break;
            case opcodes::f64_ceil:
               fb[op_index++] = f64_ceil_t{}; break;
            case opcodes::f64_floor:
               fb[op_index++] = f64_floor_t{}; break;
            case opcodes::f64_trunc:
               fb[op_index++] = f64_trunc_t{}; break;
            case opcodes::f64_nearest:
               fb[op_index++] = f64_nearest_t{}; break;
            case opcodes::f64_sqrt:
               fb[op_index++] = f64_sqrt_t{}; break;
            case opcodes::f64_add:
               fb[op_index++] = f64_add_t{}; break;
            case opcodes::f64_sub:
               fb[op_index++] = f64_sub_t{}; break;
            case opcodes::f64_mul:
               fb[op_index++] = f64_mul_t{}; break;
            case opcodes::f64_div:
               fb[op_index++] = f64_div_t{}; break;
            case opcodes::f64_min:
               fb[op_index++] = f64_min_t{}; break;
            case opcodes::f64_max:
               fb[op_index++] = f64_max_t{}; break;
            case opcodes::f64_copysign:
               fb[op_index++] = f64_copysign_t{}; break;
            case opcodes::i32_wrap_i64:
               fb[op_index++] = i32_wrap_i64_t{}; break;
            case opcodes::i32_trunc_s_f32:
               fb[op_index++] = i32_trunc_s_f32_t{}; break;
            case opcodes::i32_trunc_u_f32:
               fb[op_index++] = i32_trunc_u_f32_t{}; break;
            case opcodes::i32_trunc_s_f64:
               fb[op_index++] = i32_trunc_s_f64_t{}; break;
            case opcodes::i32_trunc_u_f64:
               fb[op_index++] = i32_trunc_u_f64_t{}; break;
            case opcodes::i64_extend_s_i32:
               fb[op_index++] = i64_extend_s_i32_t{}; break;
            case opcodes::i64_extend_u_i32:
               fb[op_index++] = i64_extend_u_i32_t{}; break;
            case opcodes::i64_trunc_s_f32:
               fb[op_index++] = i64_trunc_s_f32_t{}; break;
            case opcodes::i64_trunc_u_f32:
               fb[op_index++] = i64_trunc_u_f32_t{}; break;
            case opcodes::i64_trunc_s_f64:
               fb[op_index++] = i64_trunc_s_f64_t{}; break;
            case opcodes::i64_trunc_u_f64:
               fb[op_index++] = i64_trunc_u_f64_t{}; break;
            case opcodes::f32_convert_s_i32:
               fb[op_index++] = f32_convert_s_i32_t{}; break;
            case opcodes::f32_convert_u_i32:
               fb[op_index++] = f32_convert_u_i32_t{}; break;
            case opcodes::f32_convert_s_i64:
               fb[op_index++] = f32_convert_s_i64_t{}; break;
            case opcodes::f32_convert_u_i64:
               fb[op_index++] = f32_convert_u_i64_t{}; break;
            case opcodes::f32_demote_f64:
               fb[op_index++] = f32_demote_f64_t{}; break;
            case opcodes::f64_convert_s_i32:
               fb[op_index++] = f64_convert_s_i32_t{}; break;
            case opcodes::f64_convert_u_i32:
               fb[op_index++] = f64_convert_u_i32_t{}; break;
            case opcodes::f64_convert_s_i64:
               fb[op_index++] = f64_convert_s_i64_t{}; break;
            case opcodes::f64_convert_u_i64:
               fb[op_index++] = f64_convert_u_i64_t{}; break;
            case opcodes::f64_promote_f32:
               fb[op_index++] = f64_promote_f32_t{}; break;
            case opcodes::i32_reinterpret_f32:
               fb[op_index++] = i32_reinterpret_f32_t{}; break;
            case opcodes::i64_reinterpret_f64:
               fb[op_index++] = i64_reinterpret_f64_t{}; break;
            case opcodes::f32_reinterpret_i32:
               fb[op_index++] = f32_reinterpret_i32_t{}; break;
            case opcodes::f64_reinterpret_i64:
               fb[op_index++] = f64_reinterpret_i64_t{}; break;
            case opcodes::error:
               fb[op_index++] = error_t{}; break;
         } 
      }
      fb.resize(op_index);
   }

   void binary_parser::parse_function_body( wasm_code_ptr& code, function_body& fb ) {
      const auto& body_size = parse_varuint32( code );
      const auto& before = code.offset();
      const auto& local_cnt = parse_varuint32( code );
      fb.local_count = local_cnt;
      fb.locals.resize(local_cnt);
      // parse the local entries
      for ( size_t i=0; i < local_cnt; i++ ) {
         fb.locals.at(i).count = parse_varuint32( code );
         fb.locals.at(i).type  = *code++;
      }

      size_t bytes = body_size - (code.offset() - before); // -1 is 'end' 0xb byte
      fb.code.resize(bytes);
      wasm_code_ptr fb_code(code.raw(), bytes);
      parse_function_body_code( fb_code, bytes, fb.code );
      code += bytes-1;
      EOS_WB_ASSERT( *code++ == 0x0B, wasm_parse_exception, "failed parsing function body, expected 'end'");
      fb.code[fb.code.size()-1] = fend_t{};
   };

   void binary_parser::parse_data_segment( wasm_code_ptr& code, data_segment& ds ) {
      ds.index  = parse_varuint32( code );
      parse_init_expr( code, ds.offset );
      ds.size   = parse_varuint32( code );
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
