#pragma once

#include <vector>

#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/parser.hpp>

namespace eosio { namespace wasm_backend {
   /*
   template <uint16_t opcode>
   struct execute_op {
      inline static ret_type exec( wasm_code_ptr& code ) { 
         EOS_WB_ASSERT( false, wasm_illegal_opcode_exception, "unsupported opcode" ); 
      }
   };
   template <>
   struct execute_op<unreachable> {
      inline static ret_type exec( wasm_code_ptr& code ) { 
         EOS_WB_ASSERT( false, wasm_unreachable_exception, "unreachable opcode" ); 
      }
   };
   template <>
   struct execute_op<nop> {
      inline static ret_type exec( wasm_code_ptr& code ) { 
      }
   };
   template <>
   struct execute_op<block> {
      inline static ret_type exec( wasm_code_ptr& code ) { 
      }
   };
   */

   class execution_engine {
      public:
         execution_engine(module& m) : mod(&m) {
            apply_index = get_apply_index();
         }
         void execute();
         void call( size_t index );
         void eval( wasm_code_ptr& code );
         void call( wasm_code_ptr& code, size_t size );

         template <uint16_t op>
         inline auto exec( wasm_code_ptr& code ) 
            -> std::enable_if_t<op == opcode::unreachable, void> {
            EOS_WB_ASSERT( false, wasm_unreachable_exception, "unreachable opcode" ); 
         }
         template <uint16_t op>
         inline auto exec( wasm_code_ptr& code ) 
            -> std::enable_if_t<op == opcode::nop, void> {}
         template <uint16_t op>
         inline auto exec( wasm_code_ptr& code ) 
            -> std::enable_if_t<op == opcode::call, void> {
            std::cout << "CALL\n";
         }
         
         // nop
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::nop, void> { /* do nothing */ }

         // end
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::end, bool> { /* do nothing */ }

         // return
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::return_, uint64_t> {
         }

         // block
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::block, void> {
            cs.push( *code++ );
         }

         // loop
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::loop, void> {
            cs.push( *code++ );
         }

         // if
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::if_, void> {
            cs.push( *code++ );
         }

         // else
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::else_, void> {
         }

         // br
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::br, void> {
            binary_parser::parse_varuint<32>( code ); 
         }

         // br_if
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::br_if, void> {
            binary_parser::parse_varuint<32>( code ); 
         }

         // br_table
         //TODO br table limit
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::br_table, void> {
            auto len = binary_parser::parse_varuint<32>( code ); 
            for ( size_t i=0; i < len; i++ ) {
               binary_parser::parse_varuint<32>( code );
            }
            binary_parser::parse_varuint<32>( code );
         }

         // call
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::call, void> {
            binary_parser::parse_varuint<32>( code );
         }

         // call_indirect
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::call_indirect, void> {
            binary_parser::parse_varuint<32>( code );
            *code++; //MVP always 0
         }

         // drop
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::drop, void> {
         }

         // select
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::select, void> {
         }

         // get_local
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::get_local, void> {
            binary_parser::parse_varuint<32>( code );
         }

         // set_local
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::set_local, void> {
            binary_parser::parse_varuint<32>( code );
         }

         // tee_local
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::tee_local, void> {
            binary_parser::parse_varuint<32>( code );
         }

         // get_global
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::get_global, void> {
            binary_parser::parse_varuint<32>( code );
         }

         // set_global
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::set_global, void> {
            binary_parser::parse_varuint<32>( code );
         }
         
         // i32.load
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_load, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.load
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_load, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // f32.load
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f32_load, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // f64.load
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f64_load, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i32.load8_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_load8_s, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i32.load8_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_load8_u, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i32.load16_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_load16_s, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i32.load16_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_load16_u, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.load8_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_load8_s, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.load8_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_load8_u, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.load16_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_load16_s, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.load16_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_load16_u, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.load32_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_load32_s, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.load32_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_load32_u, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i32.store
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_store, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.store
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_store, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // f32.store
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f32_store, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // f64.store
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f64_store, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i32.store8
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_store8, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i32.store16
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_store16, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.store8
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_store8, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.store16
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_store16, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // i64.store32
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_store32, void> {
            binary_parser::parse_varuint<32>( code );
            binary_parser::parse_varuint<32>( code );
         }

         // current_memory 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::current_memory, void> {
            code++; // reserved MVP
         }

         // grow_memory 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::grow_memory, void> {
            code++; // reserved MVP
         }

         // i32.const
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_const, void> {
            binary_parser::parse_varint<32>( code );
         }

         // i64.const
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_const, void> {
            binary_parser::parse_varint<64>( code );
         }

         // f32.const
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f32_const, void> {
            code += 4;
         }

         // f64.const
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f64_const, void> {
            code += 8;
         }

         // i32.eqz 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_eqz, void> {
         }
         // i32.eq 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_eq, void> {
         }
         // i32.ne 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_ne, void> {
         }
         // i32.lt_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_lt_s, void> {
         }
         // i32.lt_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_lt_u, void> {
         }
         // i32.gt_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_gt_s, void> {
         }
         // i32.gt_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_gt_u, void> {
         }
         // i32.le_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_le_s, void> {
         }
         // i32.le_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_le_u, void> {
         }
         // i32.ge_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_ge_s, void> {
         }
         // i32.ge_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_ge_u, void> {
         }

         // i64.eqz 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_eqz, void> {
         }
         // i64.eq 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_eq, void> {
         }
         // i64.ne 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_ne, void> {
         }
         // i64.lt_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_lt_s, void> {
         }
         // i64.lt_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_lt_u, void> {
         }
         // i64.gt_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_gt_s, void> {
         }
         // i64.gt_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_gt_u, void> {
         }
         // i64.le_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_le_s, void> {
         }
         // i64.le_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_le_u, void> {
         }
         // i64.ge_s 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_ge_s, void> {
         }
         // i64.ge_u 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_ge_u, void> {
         }

         // f32.eq 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f32_eq, void> {
         }
         // f32.ne 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f32_ne, void> {
         }
         // f32.lt 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f32_lt, void> {
         }
         // f32.gt 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f32_gt, void> {
         }
         // f32.le 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f32_le, void> {
         }
         // f32.ge 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f32_ge, void> {
         }
         // f64.eq 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f64_eq, void> {
         }
         // f64.ne 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f64_ne, void> {
         }
         // f64.lt 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f64_lt, void> {
         }
         // f64.gt 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f64_gt, void> {
         }
         // f64.le 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f64_le, void> {
         }
         // f64.ge 
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::f64_ge, void> {
         }

         // i32.clz
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_clz, void> {
         }
         // i32.ctz
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_ctz, void> {
         }
         // i32.popcnt
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_popcnt, void> {
         }
         // i32.add
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_add, void> {
         }
         // i32.sub
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_sub, void> {
         }
         // i32.mul
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_mul, void> {
         }
         // i32.div_s
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_div_s, void> {
         }
         // i32.div_u
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_div_u, void> {
         }
         // i32.rem_s
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_rem_s, void> {
         }
         // i32.rem_u
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_rem_u, void> {
         }
         // i32.and
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_and, void> {
         }
         // i32.or
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_or, void> {
         }
         // i32.xor
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_xor, void> {
         }
         // i32.shl
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_shl, void> {
         }
         // i32.shr_s
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_shr_s, void> {
         }
         // i32.shr_u
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_shr_u, void> {
         }
         // i32.rotl
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_rotl, void> {
         }
         // i32.rotr
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i32_rotr, void> {
         }

         // i64.clz
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_clz, void> {
         }
         // i64.ctz
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_ctz, void> {
         }
         // i64.popcnt
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_popcnt, void> {
         }
         // i64.add
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_add, void> {
         }
         // i64.sub
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_sub, void> {
         }
         // i64.mul
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_mul, void> {
         }
         // i64.div_s
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_div_s, void> {
         }
         // i64.div_u
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_div_u, void> {
         }
         // i64.rem_s
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_rem_s, void> {
         }
         // i64.rem_u
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_rem_u, void> {
         }
         // i64.and
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_and, void> {
         }
         // i64.or
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_or, void> {
         }
         // i64.xor
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_xor, void> {
         }
         // i64.shl
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_shl, void> {
         }
         // i64.shr_s
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_shr_s, void> {
         }
         // i64.shr_u
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_shr_u, void> {
         }
         // i64.rotl
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_rotl, void> {
         }
         // i64.rotr
         template <opcode Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcode::i64_rotr, void> {
         }

         inline size_t get_apply_index() {
            for ( size_t i=0; i < mod->exports.size(); i++ )
               if ( mod->exports.at(i).kind == external_kind::Function 
                     && memcmp(mod->exports.at(i).field_str.raw(), "apply", 5) == 0 )
                  return mod->exports.at(i).index;
            return -1;
         }

      private:
         struct params {
            wasm_code* code;
            const wasm_code_iterator* at;
         };
         
         typedef void (*op_cb)(params&& p);
         std::vector<op_cb>      dispatch_table;
         module*                 mod;
         control_stack           cs;
         size_t                  apply_index;
         //std::vector<function>   function_table; 
         void populate_dispatch_table();
   };
}} // namespace eosio::wasm_backend
