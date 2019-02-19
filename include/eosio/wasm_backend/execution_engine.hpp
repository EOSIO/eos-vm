#pragma once

#include <vector>

#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/parser.hpp>

namespace eosio { namespace wasm_backend {

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
            -> std::enable_if_t<op == opcodes::unreachable, void> {
         }
         template <uint16_t op>
         inline auto exec( wasm_code_ptr& code ) 
            -> std::enable_if_t<op == opcodes::nop, void> {}
         template <uint16_t op>
         inline auto exec( wasm_code_ptr& code ) 
            -> std::enable_if_t<op == opcodes::call, void> {
            std::cout << "CALL\n";
         }
         
         // nop
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::nop, void> { return; }

         // end
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::end, bool> { return false; }

         // return
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::return_, uint64_t> { return 0; }

         // block
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::block, void> {
            //cs.push( *code++ );
         }

         // loop
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::loop, void> {
            //cs.push( *code++ );
         }

         // if
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::if_, void> {
            //cs.push( *code++ );
         }

         // else
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::else_, void> {
         }

         // br
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::br, void> {
            binary_parser::parse_varuint32( code ); 
         }

         // br_if
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::br_if, void> {
            binary_parser::parse_varuint32( code ); 
         }

         // br_table
         //TODO br table limit
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::br_table, void> {
            auto len = binary_parser::parse_varuint32( code ); 
            for ( size_t i=0; i < len; i++ ) {
               binary_parser::parse_varuint32( code );
            }
            binary_parser::parse_varuint32( code );
         }

         // call
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::call, void> {
            binary_parser::parse_varuint32( code );
         }

         // call_indirect
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::call_indirect, void> {
            binary_parser::parse_varuint32( code );
            *code++; //MVP always 0
         }

         // drop
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::drop, void> {
         }

         // select
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::select, void> {
         }

         // get_local
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::get_local, void> {
            binary_parser::parse_varuint32( code );
         }

         // set_local
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::set_local, void> {
            binary_parser::parse_varuint32( code );
         }

         // tee_local
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::tee_local, void> {
            binary_parser::parse_varuint32( code );
         }

         // get_global
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::get_global, void> {
            binary_parser::parse_varuint32( code );
         }

         // set_global
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::set_global, void> {
            binary_parser::parse_varuint32( code );
         }
         
         // i32.load
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_load, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.load
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_load, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // f32.load
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f32_load, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // f64.load
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f64_load, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i32.load8_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_load8_s, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i32.load8_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_load8_u, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i32.load16_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_load16_s, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i32.load16_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_load16_u, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.load8_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_load8_s, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.load8_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_load8_u, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.load16_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_load16_s, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.load16_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_load16_u, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.load32_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_load32_s, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.load32_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_load32_u, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i32.store
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_store, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.store
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_store, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // f32.store
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f32_store, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // f64.store
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f64_store, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i32.store8
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_store8, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i32.store16
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_store16, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.store8
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_store8, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.store16
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_store16, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // i64.store32
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_store32, void> {
            binary_parser::parse_varuint32( code );
            binary_parser::parse_varuint32( code );
         }

         // current_memory 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::current_memory, void> {
            code++; // reserved MVP
         }

         // grow_memory 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::grow_memory, void> {
            code++; // reserved MVP
         }

         // i32.const
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_const, void> {
            binary_parser::parse_varint32( code );
         }

         // i64.const
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_const, void> {
            binary_parser::parse_varint64( code );
         }

         // f32.const
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f32_const, void> {
            code += 4;
         }

         // f64.const
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f64_const, void> {
            code += 8;
         }

         // i32.eqz 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_eqz, void> {
         }
         // i32.eq 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_eq, void> {
         }
         // i32.ne 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_ne, void> {
         }
         // i32.lt_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_lt_s, void> {
         }
         // i32.lt_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_lt_u, void> {
         }
         // i32.gt_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_gt_s, void> {
         }
         // i32.gt_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_gt_u, void> {
         }
         // i32.le_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_le_s, void> {
         }
         // i32.le_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_le_u, void> {
         }
         // i32.ge_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_ge_s, void> {
         }
         // i32.ge_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_ge_u, void> {
         }

         // i64.eqz 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_eqz, void> {
         }
         // i64.eq 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_eq, void> {
         }
         // i64.ne 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_ne, void> {
         }
         // i64.lt_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_lt_s, void> {
         }
         // i64.lt_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_lt_u, void> {
         }
         // i64.gt_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_gt_s, void> {
         }
         // i64.gt_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_gt_u, void> {
         }
         // i64.le_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_le_s, void> {
         }
         // i64.le_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_le_u, void> {
         }
         // i64.ge_s 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_ge_s, void> {
         }
         // i64.ge_u 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_ge_u, void> {
         }

         // f32.eq 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f32_eq, void> {
         }
         // f32.ne 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f32_ne, void> {
         }
         // f32.lt 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f32_lt, void> {
         }
         // f32.gt 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f32_gt, void> {
         }
         // f32.le 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f32_le, void> {
         }
         // f32.ge 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f32_ge, void> {
         }
         // f64.eq 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f64_eq, void> {
         }
         // f64.ne 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f64_ne, void> {
         }
         // f64.lt 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f64_lt, void> {
         }
         // f64.gt 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f64_gt, void> {
         }
         // f64.le 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f64_le, void> {
         }
         // f64.ge 
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::f64_ge, void> {
         }

         // i32.clz
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_clz, void> {
         }
         // i32.ctz
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_ctz, void> {
         }
         // i32.popcnt
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_popcnt, void> {
         }
         // i32.add
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_add, void> {
         }
         // i32.sub
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_sub, void> {
         }
         // i32.mul
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_mul, void> {
         }
         // i32.div_s
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_div_s, void> {
         }
         // i32.div_u
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_div_u, void> {
         }
         // i32.rem_s
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_rem_s, void> {
         }
         // i32.rem_u
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_rem_u, void> {
         }
         // i32.and
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_and, void> {
         }
         // i32.or
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_or, void> {
         }
         // i32.xor
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_xor, void> {
         }
         // i32.shl
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_shl, void> {
         }
         // i32.shr_s
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_shr_s, void> {
         }
         // i32.shr_u
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_shr_u, void> {
         }
         // i32.rotl
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_rotl, void> {
         }
         // i32.rotr
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i32_rotr, void> {
         }

         // i64.clz
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_clz, void> {
         }
         // i64.ctz
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_ctz, void> {
         }
         // i64.popcnt
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_popcnt, void> {
         }
         // i64.add
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_add, void> {
         }
         // i64.sub
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_sub, void> {
         }
         // i64.mul
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_mul, void> {
         }
         // i64.div_s
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_div_s, void> {
         }
         // i64.div_u
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_div_u, void> {
         }
         // i64.rem_s
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_rem_s, void> {
         }
         // i64.rem_u
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_rem_u, void> {
         }
         // i64.and
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_and, void> {
         }
         // i64.or
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_or, void> {
         }
         // i64.xor
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_xor, void> {
         }
         // i64.shl
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_shl, void> {
         }
         // i64.shr_s
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_shr_s, void> {
         }
         // i64.shr_u
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_shr_u, void> {
         }
         // i64.rotl
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_rotl, void> {
         }
         // i64.rotr
         template <opcodes Op>
         inline auto eval( wasm_code_ptr& code )
            -> std::enable_if_t<Op == opcodes::i64_rotr, void> {
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
         //control_stack           cs;
         size_t                  apply_index;
         //std::vector<function>   function_table; 
         void populate_dispatch_table();
   };
}} // namespace eosio::wasm_backend
