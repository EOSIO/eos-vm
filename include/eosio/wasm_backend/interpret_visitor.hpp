#pragma once

#include <eosio/wasm_backend/utils.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/execution_context.hpp>
#include <iostream>
#include <variant>
#include <sstream>

// TODO add a config header
static constexpr bool use_softfloat = false;

#include <eosio/wasm_backend/softfloat.hpp>

namespace eosio { namespace wasm_backend {

template <typename Backend>
struct interpret_visitor {
   interpret_visitor(Backend& backend, execution_context<Backend>& ec) : context(ec), allocator(backend.get_wasm_allocator()) {}
   execution_context<Backend>& context;
   wasm_allocator& allocator;

   execution_context<Backend>& get_context() { return context; }

   void operator()( const unreachable_t& op) {
      context.inc_pc();
      throw wasm_interpreter_exception{"unreachable"};
   }
   void operator()( const nop_t& op) {

      context.inc_pc();
   }

   void operator()( const fend_t& op) {
      context.apply_pop_call();
   }
   void operator()( const end_t& op) {
      const auto& label = context.pop_label();
      uint16_t op_index = 0;
      uint8_t  ret_type = 0;
      std::visit(overloaded {
         [&](const block_t& b) {
            op_index = b.op_index;
            ret_type = static_cast<uint8_t>(b.data);
         }, [&](const loop_t& l) {
            op_index = l.op_index;
            ret_type = static_cast<uint8_t>(l.data);
         },[&](const if__t& i) {
            op_index = i.op_index;
            ret_type = static_cast<uint8_t>(i.data);
         }, [&](auto) {
            throw wasm_interpreter_exception{"expected control structure"};
         }
      }, label);

      if ( ret_type != types::pseudo ) {
         const auto& op = context.pop_operand();
         context.eat_operands(op_index);
         context.push_operand(op);
      } else {
         context.eat_operands(op_index);
      }

      context.inc_pc();
   }
   void operator()( const return__t& op) {
      context.apply_pop_call();
   }
   void operator()( block_t& op) {
      context.inc_pc();
      op.index = context.current_label_index();
      op.op_index = context.current_operands_index();
      context.push_label(op);
   }
   void operator()( loop_t& op) {
      context.inc_pc();
      op.index = context.current_label_index();
      op.op_index = context.current_operands_index();
      context.push_label(op);
   }
   void operator()( if__t& op) {
      context.inc_pc();
      op.index = context.current_label_index();
      op.op_index = context.current_operands_index();
      const auto& oper = context.pop_operand();
      if (!TO_UINT32(oper))
         context.set_relative_pc(op.pc+1);
      context.push_label(op);
   }
   void operator()( const else__t& op) {
      context.set_relative_pc(op.pc);
   }
   void operator()( const br_t& op) {
      context.jump(op.data);
   }
   void operator()( const br_if_t& op) {
      const auto& val = context.pop_operand();
      if (context.is_true(val)) {
         context.jump(op.data);
      }
      else {
         context.inc_pc();
      }
   }
   void operator()( const br_table_t& op) {
      const auto& in = TO_UINT32(context.pop_operand());
      if (in < op.size)
         context.jump(op.table[in]);
      else
         context.jump(op.default_target);
   }
   void operator()( const call_t& op) {
      context.call(op.index);
      // TODO place these in parser
      //EOS_WB_ASSERT(b.index < funcs_size, wasm_interpreter_exception, "call index out of bounds");
      /*
      if (ftype.return_count > 0) {
         EOS_WB_ASSERT(ftype.return_count <= 1, wasm_interpreter_exception, "mvp only supports single value returns");
         context.push_operand(ret_val);
      }
      */
   }
   void operator()( const call_indirect_t& op) {
      const auto& index = TO_UINT32(context.pop_operand());
      context.call(context.table_elem(index));
   }
   void operator()( const drop_t& op) {
      context.pop_operand();
      context.inc_pc();
   }
   void operator()( const select_t& op) {
      const auto& c = context.pop_operand();
      const auto& v2 = context.pop_operand();
      if (TO_UINT32(c) == 0) {
         context.peek_operand() = v2;
      }
      context.inc_pc();
   }
   void operator()( const get_local_t& op) {
      context.inc_pc();
      context.push_operand(context.get_operand(op.index));
   }
   void operator()( const set_local_t& op) {
      context.inc_pc();
      context.set_operand(op.index, context.pop_operand());
   }
   void operator()( const tee_local_t& op) {
      context.inc_pc();
      const auto& oper = context.pop_operand();
      context.set_operand(op.index, oper);
      context.push_operand(oper);
   }
   void operator()( const get_global_t& op) {
      context.inc_pc();
      const auto& gl = context.get_global(op.index);
      context.push_operand(gl);
   }
   void operator()( const set_global_t& op) {
      context.inc_pc();
      const auto& oper = context.pop_operand();
      context.set_global(op.index, oper);
   }
   void operator()( const i32_load_t& op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint32_t* _ptr = (uint32_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      context.push_operand(i32_const_t{*_ptr});
   }
   void operator()( const i32_load8_s_t& op) {

      context.inc_pc();
      const auto& ptr = context.pop_operand();
      int8_t* _ptr = (int8_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      int8_t val = *_ptr;
      context.push_operand(i32_const_t{*(uint32_t*)&val});
   }
   void operator()( const i32_load16_s_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      int16_t* _ptr = (int16_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      int16_t val = *_ptr;
      context.push_operand(i32_const_t{*(uint32_t*)&val});
   }
   void operator()( const i32_load8_u_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint8_t* _ptr = (uint8_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      uint8_t val = *_ptr;
      context.push_operand(i32_const_t{val});
   }
   void operator()( const i32_load16_u_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint16_t* _ptr = (uint16_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      uint16_t val = *_ptr;
      context.push_operand(i32_const_t{val});
   }
   void operator()( const i64_load_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint64_t* _ptr = (uint64_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      uint64_t val = *_ptr;
      context.push_operand(i64_const_t{val});
   }
   void operator()( const i64_load8_s_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      int8_t* _ptr = (int8_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      int8_t val = *_ptr;
      context.push_operand(i64_const_t{*(uint64_t*)&val});
   }
   void operator()( const i64_load16_s_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      int16_t* _ptr = (int16_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      int16_t val = *_ptr;
      context.push_operand(i64_const_t{*(uint64_t*)&val});
   }
   void operator()( const i64_load32_s_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      int32_t* _ptr = (int32_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      int32_t val = *_ptr;
      context.push_operand(i64_const_t{*(uint64_t*)&val});
   }
   void operator()( const i64_load8_u_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint8_t* _ptr = (uint8_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      uint8_t val = *_ptr;
      context.push_operand(i64_const_t{static_cast<uint64_t>(val)});
   }
   void operator()( const i64_load16_u_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint16_t* _ptr = (uint16_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      uint16_t val = *_ptr;
      context.push_operand(i64_const_t{static_cast<uint64_t>(val)});
   }
   void operator()( const i64_load32_u_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint32_t* _ptr = (uint32_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      uint32_t val = *_ptr;
      context.push_operand(i64_const_t{static_cast<uint64_t>(val)});
   }
   void operator()( const f32_load_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint32_t* _ptr = (uint32_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      uint32_t val = *_ptr;
      context.push_operand(f32_const_t{val});
   }
   void operator()( const f64_load_t & op) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint64_t* _ptr = (uint64_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      uint64_t val = *_ptr;
      context.push_operand(f64_const_t{val});
   }
   void operator()( const i32_store_t & op) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint32_t* store_loc = (uint32_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      *store_loc = TO_UINT32(val);
   }
   void operator()( const i32_store8_t & op) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint8_t* store_loc = (uint8_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint8_t>(TO_UINT32(val));
   }
   void operator()( const i32_store16_t & op) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint16_t* store_loc = (uint16_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint16_t>(TO_UINT32(val));
   }
   void operator()( const i64_store_t & op) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint64_t* store_loc = (uint64_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint64_t>(TO_UINT64(val));
   }
   void operator()( const i64_store8_t & op) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint8_t* store_loc = (uint8_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint8_t>(TO_UINT64(val));
   }
   void operator()( const i64_store16_t & op) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint16_t* store_loc = (uint16_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint16_t>(TO_UINT64(val));
   }
   void operator()( const i64_store32_t & op) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint32_t* store_loc = (uint32_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint32_t>(TO_UINT64(val));
   }
   void operator()( const f32_store_t& op) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint32_t* store_loc = (uint32_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint32_t>(TO_FUINT32(val));
   }
   void operator()( const f64_store_t& op) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint64_t* store_loc = (uint64_t*)(context.linear_memory()+op.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint64_t>(TO_FUINT64(val));
   }
   void operator()( const current_memory_t& op) {
      context.inc_pc();
      context.push_operand(i32_const_t{ context.current_linear_memory() });
   }
   void operator()( const grow_memory_t& op) {
      context.inc_pc();
      auto& oper = TO_UINT32(context.peek_operand());
      oper = context.grow_linear_memory( oper );
   }
   void operator()( const i32_const_t& op) {
      context.inc_pc();
      context.push_operand(op);
   }
   void operator()( const i64_const_t& op) {
      context.inc_pc();
      context.push_operand(op);
   }
   void operator()( const f32_const_t& op) {
      context.inc_pc();
      context.push_operand(op);
   }
   void operator()( const f64_const_t& op) {
      context.inc_pc();
      context.push_operand(op);
   }
   void operator()( const i32_eqz_t& op) {
      context.inc_pc();
      auto& t = TO_UINT32(context.peek_operand());
      t = t == 0;
   }
   void operator()( const i32_eq_t & op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs = lhs == rhs;
   }
   void operator()( const i32_ne_t & op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs = lhs != rhs;
   }
   void operator()( const i32_lt_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT32(context.pop_operand());
      auto& lhs = TO_INT32(context.peek_operand());
      lhs = lhs < rhs;
   }
   void operator()( const i32_lt_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs = lhs < rhs; 
   }
   void operator()( const i32_le_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT32(context.pop_operand());
      auto& lhs = TO_INT32(context.peek_operand());
      lhs = lhs <= rhs;
   }
   void operator()( const i32_le_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs = lhs <= rhs;
   }
   void operator()( const i32_gt_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT32(context.pop_operand());
      auto& lhs = TO_INT32(context.peek_operand());
      lhs = lhs > rhs;
   }
   void operator()( const i32_gt_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs = lhs > rhs;
   }
   void operator()( const i32_ge_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT32(context.pop_operand());
      auto& lhs = TO_INT32(context.peek_operand());
      lhs = lhs >= rhs;
   }
   void operator()( const i32_ge_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs = lhs >= rhs;
   }
   void operator()( const i64_eqz_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i32_const_t{ TO_UINT64(oper) == 0 };
   }
   void operator()( const i64_eq_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = context.peek_operand();
      lhs = i32_const_t{TO_UINT64(lhs) == rhs};
   }
   void operator()( const i64_ne_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = context.peek_operand();
      lhs = i32_const_t{ TO_UINT64(lhs) != rhs };
   }
   void operator()( const i64_lt_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT64(context.pop_operand());
      auto& lhs = context.peek_operand();
      lhs = i32_const_t{ TO_INT64(lhs) < rhs };
   }
   void operator()( const i64_lt_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = context.peek_operand();
      lhs = i32_const_t{ TO_UINT64(lhs) < rhs };
   }
   void operator()( const i64_le_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT64(context.pop_operand());
      auto& lhs = context.peek_operand();
      lhs = i32_const_t{ TO_INT64(lhs) <= rhs };
   }
   void operator()( const i64_le_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = context.peek_operand();
      lhs = i32_const_t{ TO_UINT64(lhs) <= rhs };
   }
   void operator()( const i64_gt_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT64(context.pop_operand());
      auto& lhs = context.peek_operand();
      lhs = i32_const_t{ TO_INT64(lhs) > rhs };
   }
   void operator()( const i64_gt_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = context.peek_operand();
      lhs = i32_const_t{ TO_UINT64(lhs) > rhs };
   }
   void operator()( const i64_ge_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT64(context.pop_operand());
      auto& lhs = context.peek_operand();
      lhs = i32_const_t{ TO_INT64(lhs) >= rhs };
   }
   void operator()( const i64_ge_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = context.peek_operand();
      lhs = i32_const_t{ TO_UINT64(lhs) >= rhs };
   }
   void operator()( const f32_eq_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f32_eq(TO_F32(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F32(lhs) == rhs)};
   }
   void operator()( const f32_ne_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f32_ne(TO_F32(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F32(lhs) != rhs)};
   }
   void operator()( const f32_lt_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f32_lt(TO_F32(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F32(lhs) < rhs)};
   }
   void operator()( const f32_gt_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f32_gt(TO_F32(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F32(lhs) > rhs)};
   }
   void operator()( const f32_le_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f32_le(TO_F32(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F32(lhs) <= rhs)};
   }
   void operator()( const f32_ge_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f32_ge(TO_F32(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F32(lhs) >= rhs)};
   }
   void operator()( const f64_eq_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f64_eq(TO_F64(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F64(lhs) == rhs)};
   }
   void operator()( const f64_ne_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f64_ne(TO_F64(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F64(lhs) != rhs)};
   }
   void operator()( const f64_lt_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f64_lt(TO_F64(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F64(lhs) < rhs)};
   }
   void operator()( const f64_gt_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f64_gt(TO_F64(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F64(lhs) > rhs)};
   }
   void operator()( const f64_le_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f64_le(TO_F64(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F64(lhs) <= rhs)};
   }
   void operator()( const f64_ge_t& op) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      auto& lhs = context.peek_operand();
      if constexpr (use_softfloat)
         lhs = i32_const_t{(uint32_t)_eosio_f64_ge(TO_F64(lhs), rhs)};
      else
         lhs = i32_const_t{(uint32_t)(TO_F64(lhs) >= rhs)};
   }
   void operator()( const i32_clz_t& op) {
      context.inc_pc();
      auto& oper = TO_UINT32(context.peek_operand());
      oper = __builtin_clz(oper);
   }
   void operator()( const i32_ctz_t& op) {
      context.inc_pc();
      auto& oper = TO_UINT32(context.peek_operand());
      oper = __builtin_ctz(oper);
   }
   void operator()( const i32_popcnt_t& op) {
      context.inc_pc();
      auto& oper = TO_UINT32(context.peek_operand());
      oper = __builtin_popcount(oper);
   }
   void operator()( const i32_add_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs += rhs;
   }
   void operator()( const i32_sub_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs -= rhs;
   }
   void operator()( const i32_mul_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs *= rhs;
   }
   void operator()( const i32_div_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT32(context.pop_operand());
      auto& lhs = TO_INT32(context.peek_operand());
      EOS_WB_ASSERT(rhs != 0, wasm_interpreter_exception, "i32.div_s divide by zero");
      EOS_WB_ASSERT(!(lhs == std::numeric_limits<int32_t>::max() && rhs == -1), wasm_interpreter_exception, "i32.div_s traps when I32_MAX/-1");
      lhs /= rhs;
   }
   void operator()( const i32_div_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      EOS_WB_ASSERT(rhs != 0, wasm_interpreter_exception, "i32.div_u divide by zero");
      lhs /= rhs;
   }
   void operator()( const i32_rem_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT32(context.pop_operand());
      auto& lhs = TO_INT32(context.peek_operand());
      EOS_WB_ASSERT(rhs != 0, wasm_interpreter_exception, "i32.rem_s divide by zero");
      if (UNLIKELY(lhs == std::numeric_limits<int32_t>::max() && rhs == -1))
         lhs = 0;
      else
         lhs %= rhs;
   }
   void operator()( const i32_rem_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      EOS_WB_ASSERT(rhs != 0, wasm_interpreter_exception, "i32.rem_u divide by zero");
      lhs %= rhs;
   }
   void operator()( const i32_and_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs &= rhs;
   }
   void operator()( const i32_or_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs |= rhs;
   }
   void operator()( const i32_xor_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs ^= rhs;
   }
   void operator()( const i32_shl_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs <<= rhs;
   }
   void operator()( const i32_shr_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_INT32(context.peek_operand());
      lhs >>= rhs;
   }
   void operator()( const i32_shr_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs >>= rhs;
   }
   void operator()( const i32_rotl_t& op) {

      context.inc_pc();
      static constexpr uint32_t mask = (8*sizeof(uint32_t)-1);
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      uint32_t c = rhs;
      c &= mask;
      lhs = (lhs << c) | (lhs >> ((-c) & mask));
   }
   void operator()( const i32_rotr_t& op) {
      context.inc_pc();
      static constexpr uint32_t mask = (8*sizeof(uint32_t)-1);
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      uint32_t c = rhs;
      c &= mask;
      lhs = (lhs >> c) | (lhs << ((-c) & mask));
   }
   void operator()( const i64_clz_t& op) {
      context.inc_pc();
      auto& oper = TO_UINT64(context.peek_operand());
      oper = __builtin_clzll(oper);
   }
   void operator()( const i64_ctz_t& op) {
      context.inc_pc();
      auto& oper = TO_UINT64(context.peek_operand());
      oper = __builtin_ctzll(oper);
   }
   void operator()( const i64_popcnt_t& op) {
      context.inc_pc();
      auto& oper = TO_UINT64(context.peek_operand());
      oper = __builtin_popcountll(oper);
   }
   void operator()( const i64_add_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs += rhs;
   }
   void operator()( const i64_sub_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs -= rhs;
   }
   void operator()( const i64_mul_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs *= rhs;
   }
   void operator()( const i64_div_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT64(context.pop_operand());
      auto& lhs = TO_INT64(context.peek_operand());
      EOS_WB_ASSERT(rhs != 0, wasm_interpreter_exception, "i64.div_s divide by zero");
      EOS_WB_ASSERT(!(lhs == std::numeric_limits<int64_t>::max() && rhs == -1), wasm_interpreter_exception, "i64.div_s traps when I64_MAX/-1");
      lhs /= rhs;
   }
   void operator()( const i64_div_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      EOS_WB_ASSERT(rhs != 0, wasm_interpreter_exception, "i64.div_u divide by zero");
      lhs /= rhs;
   }
   void operator()( const i64_rem_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_INT64(context.pop_operand());
      auto& lhs = TO_INT64(context.peek_operand());
      EOS_WB_ASSERT(rhs != 0, wasm_interpreter_exception, "i64.rem_s divide by zero");
      if (UNLIKELY(lhs == std::numeric_limits<int64_t>::max() && rhs == -1))
         lhs = 0;
      else
         lhs %= rhs;
   }
   void operator()( const i64_rem_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      EOS_WB_ASSERT(rhs != 0, wasm_interpreter_exception, "i64.rem_s divide by zero");
      lhs %= rhs;
   }
   void operator()( const i64_and_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs &= rhs;
   }
   void operator()( const i64_or_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs |= rhs;
   }
   void operator()( const i64_xor_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs ^= rhs;
   }
   void operator()( const i64_shl_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs <<= rhs;
   }
   void operator()( const i64_shr_s_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_INT64(context.peek_operand());
      lhs >>= rhs;
   }
   void operator()( const i64_shr_u_t& op) {
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs >>= rhs;
   }
   void operator()( const i64_rotl_t& op) {
      context.inc_pc();
      static constexpr uint64_t mask = (8*sizeof(uint64_t)-1);
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      uint32_t c = rhs;
      c &= mask;
      lhs = (lhs << c) | (lhs >> (-c & mask));
   }
   void operator()( const i64_rotr_t& op) {
      context.inc_pc();
      static constexpr uint64_t mask = (8*sizeof(uint64_t)-1);
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      uint32_t c = rhs;
      c &= mask;
      lhs = (lhs >> c) | (lhs << (-c & mask));
   }
   void operator()( const f32_abs_t& op) {
      context.inc_pc();
      auto& oper = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f32_abs(oper);
      else
         oper = __builtin_fabsf(oper);
   }
   void operator()( const f32_neg_t& op) {
      context.inc_pc();
      auto& oper = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f32_neg(oper);
      else
         oper = (-1.0f)*oper;
   }
   void operator()( const f32_ceil_t& op) {
      context.inc_pc();
      auto& oper = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f32_ceil(oper);
      else
         oper = __builtin_ceilf(oper);
   }
   void operator()( const f32_floor_t& op) {
      context.inc_pc();
      auto& oper = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f32_floor(oper);
      else
         oper = __builtin_floorf(oper);
   }
   void operator()( const f32_trunc_t& op) {
      context.inc_pc();
      auto& oper = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f32_trunc(oper);
      else
         oper = __builtin_trunc(oper);
   }
   void operator()( const f32_nearest_t& op) {
      context.inc_pc();
      auto& oper = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f32_nearest(oper);
      else
         oper = __builtin_nearbyintf(oper);
   }
   void operator()( const f32_sqrt_t& op) {
      context.inc_pc();
      auto& oper = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f32_sqrt(oper);
      else
         oper = __builtin_sqrtf(oper);
   }
   void operator()( const f32_add_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f32_add(lhs, TO_F32(rhs));
      else
         lhs += TO_F32(rhs);
   }
   void operator()( const f32_sub_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f32_sub(lhs, TO_F32(rhs));
      else
         lhs -= TO_F32(rhs);
   }
   void operator()( const f32_mul_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f32_mul(lhs, TO_F32(rhs));
      else
         lhs *= TO_F32(rhs);
   }
   void operator()( const f32_div_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f32_div(lhs, TO_F32(rhs));
      else
         lhs /= TO_F32(rhs);
   }
   void operator()( const f32_min_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f32_min(lhs, TO_F32(rhs));
      else
         lhs = __builtin_fminf(lhs, TO_F32(rhs));
   }
   void operator()( const f32_max_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f32_max(lhs, TO_F32(rhs));
      else
         lhs = __builtin_fmaxf(lhs, TO_F32(rhs));
   }
   void operator()( const f32_copysign_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f32_copysign(lhs, TO_F32(rhs));
      else
         lhs = __builtin_copysignf(lhs, TO_F32(rhs));
   }
   void operator()( const f64_abs_t& op) {
      context.inc_pc();
      auto& oper = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f64_abs(oper);
      else
         oper = __builtin_fabs(oper);
   }
   void operator()( const f64_neg_t& op) {
      context.inc_pc();
      auto& oper = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f64_neg(oper);
      else
         oper = (-1.0)*oper;
   }
   void operator()( const f64_ceil_t& op) {

      context.inc_pc();
      auto& oper = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f64_ceil(oper);
      else
         oper = __builtin_ceil(oper);
   }
   void operator()( const f64_floor_t& op) {
      context.inc_pc();
      auto& oper = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f64_floor(oper);
      else
         oper = __builtin_floor(oper);
   }
   void operator()( const f64_trunc_t& op) {
      context.inc_pc();
      auto& oper = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f64_trunc(oper);
      else
         oper = __builtin_trunc(oper);
   }
   void operator()( const f64_nearest_t& op) {
      context.inc_pc();
      auto& oper = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f64_nearest(oper);
      else
         oper = __builtin_nearbyint(oper);
   }
   void operator()( const f64_sqrt_t& op) {
      context.inc_pc();
      auto& oper = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         oper = _eosio_f64_sqrt(oper);
      else
         oper = __builtin_sqrt(oper);
   }
   void operator()( const f64_add_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f64_add(lhs, TO_F64(rhs));
      else
         lhs += TO_F64(rhs);
   }
   void operator()( const f64_sub_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f64_sub(lhs, TO_F64(rhs));
      else
         lhs -= TO_F64(rhs);
   }
   void operator()( const f64_mul_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f64_mul(lhs, TO_F64(rhs));
      else
         lhs *= TO_F64(rhs);
   }
   void operator()( const f64_div_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f64_div(lhs, TO_F64(rhs));
      else
         lhs /= TO_F64(rhs);
   }
   void operator()( const f64_min_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f64_min(lhs, TO_F64(rhs));
      else
         lhs = __builtin_fmin(lhs, TO_F64(rhs));
   }
   void operator()( const f64_max_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f64_max(lhs, TO_F64(rhs));
      else
         lhs = __builtin_fmax(lhs, TO_F64(rhs));
   }
   void operator()( const f64_copysign_t& op) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      if constexpr (use_softfloat)
         lhs = _eosio_f64_copysign(lhs, TO_F64(rhs));
      else
         lhs = __builtin_copysign(lhs, TO_F64(rhs));
   }
   void operator()( const i32_wrap_i64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i32_const_t{static_cast<int32_t>(TO_INT64(oper))};
   }
   void operator()( const i32_trunc_s_f32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i32_const_t{_eosio_f32_trunc_i32s(TO_F32(oper))};
   }
   void operator()( const i32_trunc_u_f32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i32_const_t{_eosio_f32_trunc_i32u(TO_F32(oper))};
   }
   void operator()( const i32_trunc_s_f64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i32_const_t{_eosio_f64_trunc_i32s(TO_F64(oper))};
   }
   void operator()( const i32_trunc_u_f64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i32_const_t{_eosio_f64_trunc_i32u(TO_F64(oper))};
   }
   void operator()( const i64_extend_s_i32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i64_const_t{static_cast<int64_t>(TO_INT32(oper))};
   }
   void operator()( const i64_extend_u_i32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i64_const_t{static_cast<uint64_t>(TO_UINT32(oper))};
   }
   void operator()( const i64_trunc_s_f32_t& op) {
      context.inc_pc();
      context.print_stack();
      auto& oper = context.peek_operand();
      oper = i64_const_t{_eosio_f32_trunc_i64s(TO_F32(oper))};
   }
   void operator()( const i64_trunc_u_f32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i64_const_t{_eosio_f32_trunc_i64u(TO_F32(oper))};
   }
   void operator()( const i64_trunc_s_f64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i64_const_t{_eosio_f64_trunc_i64s(TO_F64(oper))};
   }
   void operator()( const i64_trunc_u_f64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i64_const_t{_eosio_f64_trunc_i64u(TO_F64(oper))};
   }
   void operator()( const f32_convert_s_i32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f32_const_t{_eosio_i32_to_f32(TO_INT32(oper))};
   }
   void operator()( const f32_convert_u_i32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f32_const_t{_eosio_ui32_to_f32(TO_UINT32(oper))};
   }
   void operator()( const f32_convert_s_i64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f32_const_t{_eosio_i64_to_f32(TO_INT64(oper))};
   }
   void operator()( const f32_convert_u_i64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f32_const_t{_eosio_ui64_to_f32(TO_UINT64(oper))};
   }
   void operator()( const f32_demote_f64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f32_const_t{_eosio_f64_demote(TO_F64(oper))};
   }
   void operator()( const f64_convert_s_i32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f64_const_t{_eosio_i32_to_f64(TO_INT32(oper))};
   }
   void operator()( const f64_convert_u_i32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f64_const_t{_eosio_ui32_to_f64(TO_UINT32(oper))};
   }
   void operator()( const f64_convert_s_i64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f64_const_t{_eosio_i64_to_f64(TO_INT64(oper))};
   }
   void operator()( const f64_convert_u_i64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f64_const_t{_eosio_ui64_to_f64(TO_UINT64(oper))};

   }
   void operator()( const f64_promote_f32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f64_const_t{_eosio_f32_promote(TO_F32(oper))};
   }
   void operator()( const i32_reinterpret_f32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i32_const_t{TO_FUINT32(oper)};
   }
   void operator()( const i64_reinterpret_f64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = i64_const_t{TO_FUINT64(oper)};
   }
   void operator()( const f32_reinterpret_i32_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f32_const_t{TO_UINT32(oper)};
   }
   void operator()( const f64_reinterpret_i64_t& op) {
      context.inc_pc();
      auto& oper = context.peek_operand();
      oper = f64_const_t{TO_UINT64(oper)};
   }
   void operator()( const error_t& op) {
      context.inc_pc();
   }

   uint32_t tab_width;
};

}} // ns eosio::wasm_backend
