#pragma once

#include <eosio/wasm_backend/interpret_visitor.hpp>

namespace eosio { namespace wasm_backend {

template <typename Backend>
struct debug_visitor : public interpret_visitor {
   using interpret_visitor::interpret_visitor;
   void operator()(const unreachable_t& op) {
      dbg_print("unreachable");
      interpret_visitor::operator()(op);
   }
   void operator()(const nop_t& op) {
      dbg_print("nop");
      interpret_visitor::operator()(op);
   }
   void operator()(const fend_t& op) {
      dbg_print("fend");
      interpret_visitor::operator()(op);
   }
   void operator()(const end_t& op) {
      dbg_print("end");
      interpret_visitor::operator()(op);
   }
   void operator()(const return__t& op) {
      dbg_print("return");
      interpret_visitor::operator()(op);
   }
   void operator()(const block_t& op) {
      dbg_print("block");
      interpret_visitor::operator()(op);
   }
   void operator()(const loop_t& op) {
      dbg_print("loop");
      interpret_visitor::operator()(op);
   }
   void operator()(const if__t& op) {
      dbg_print("if");
      interpret_visitor::operator()(op);
   }
   void operator()(const else__t& op) {
      dbg_print("else");
      interpret_visitor::operator()(op);
   }
   void operator()(const br_t& op) {
      dbg_print("br");
      interpret_visitor::operator()(op);
   }
   void operator()(const br_if_t& op) {
      dbg_print("br.if");
      interpret_visitor::operator()(op);
   }
   void operator()(br_table_t b) {
      dbg_print("br.table");
      const auto& in = TO_UINT32(context.pop_operand());
      if (in < b.size)
         context.jump(b.table[in]);
      else
         context.jump(b.default_target);
   }
   void operator()(call_t b) {
      dbg_print("call");
      context.call(b.index);
      // TODO place these in parser
      //EOS_WB_ASSERT(b.index < funcs_size, wasm_interpreter_exception, "call index out of bounds");
      /*
      if (ftype.return_count > 0) {
         EOS_WB_ASSERT(ftype.return_count <= 1, wasm_interpreter_exception, "mvp only supports single value returns");
         context.push_operand(ret_val);
      }
      */
   }
   void operator()(call_indirect_t b) {
      dbg_print("call_indirect");
      const auto& op = context.pop_operand();
      context.call(context.table_elem(TO_UINT32(op)));
   }
   void operator()(drop_t b) {
      dbg_print("drop");
      context.pop_operand();
      context.inc_pc();
   }
   void operator()(select_t b) {
      dbg_print("select");
      const auto& c = context.pop_operand();
      const auto& v2 = context.pop_operand();
      if (TO_UINT32(c) == 0) {
         context.peek_operand() = v2;
      }
      context.inc_pc();
   }
   void operator()(get_local_t b) {
      dbg_print("get_local");
      context.inc_pc();
      context.push_operand(context.get_operand(b.index));
   }
   void operator()(set_local_t b) {
      dbg_print("set_local");
      context.inc_pc();
      context.set_operand(b.index, context.pop_operand());
   }
   void operator()(tee_local_t b) {
      dbg_print("tee_local");
      context.inc_pc();
      const auto& op = context.pop_operand();
      context.set_operand(b.index, op);
      context.push_operand(op);
   }
   void operator()(get_global_t b) {
      dbg_print("get_global");
      context.inc_pc();
      const auto& gl = context.get_global(b.index);
      context.push_operand(gl);
   }
   void operator()(set_global_t b) {
      dbg_print("set_global");
      context.inc_pc();
      const auto& op = context.pop_operand();
      context.set_global(b.index, op);
   }
   void operator()(i32_load_t b) {
      dbg_print("i32.load");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint32_t* _ptr = (uint32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i32_const_t{*_ptr});
   }
   void operator()(i32_load8_s_t b) {
      dbg_print("i32.load8_s");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      int8_t* _ptr = (int8_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      int8_t val = *_ptr;
      context.push_operand(i32_const_t{*(uint32_t*)&val});
   }
   void operator()(i32_load16_s_t b) {
      dbg_print("i32.load16_s");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      int16_t* _ptr = (int16_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      int16_t val = *_ptr;
      context.push_operand(i32_const_t{*(uint32_t*)&val});
   }
   void operator()(i32_load8_u_t b) {
      dbg_print("i32.load8_u");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint8_t* _ptr = (uint8_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      uint8_t val = *_ptr;
      context.push_operand(i32_const_t{val});
   }
   void operator()(i32_load16_u_t b) {
      dbg_print("i32.load16_u");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint16_t* _ptr = (uint16_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      uint16_t val = *_ptr;
      context.push_operand(i32_const_t{val});
   }
   void operator()(i64_load_t b) {
      dbg_print("i64.load");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint64_t* _ptr = (uint64_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      uint64_t val = *_ptr;
      context.push_operand(i64_const_t{val});
   }
   void operator()(i64_load8_s_t b) {
      dbg_print("i64.load8_s");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      int8_t* _ptr = (int8_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      int8_t val = *_ptr;
      context.push_operand(i64_const_t{*(uint64_t*)&val});
   }
   void operator()(i64_load16_s_t b) {
      dbg_print("i64.load16_s");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      int16_t* _ptr = (int16_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      int16_t val = *_ptr;
      context.push_operand(i64_const_t{*(uint64_t*)&val});
   }
   void operator()(i64_load32_s_t b) {
      dbg_print("i64.load32_s");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      int32_t* _ptr = (int32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      int32_t val = *_ptr;
      context.push_operand(i64_const_t{*(uint64_t*)&val});
   }
   void operator()(i64_load8_u_t b) {
      dbg_print("i64.load8_u");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint8_t* _ptr = (uint8_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      uint8_t val = *_ptr;
      context.push_operand(i64_const_t{static_cast<uint64_t>(val)});
   }
   void operator()(i64_load16_u_t b) {
      dbg_print("i64.load16_u");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint16_t* _ptr = (uint16_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      uint16_t val = *_ptr;
      context.push_operand(i64_const_t{static_cast<uint64_t>(val)});
   }
   void operator()(i64_load32_u_t b) {
      dbg_print("i64.load32_u");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint32_t* _ptr = (uint32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      uint32_t val = *_ptr;
      context.push_operand(i64_const_t{static_cast<uint64_t>(val)});
   }
   void operator()(f32_load_t b) {
      dbg_print("f32.load");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint32_t* _ptr = (uint32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      uint32_t val = *_ptr;
      context.push_operand(f32_const_t{val});
   }
   void operator()(f64_load_t b) {
      dbg_print("f64.load");
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      uint64_t* _ptr = (uint64_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      uint64_t val = *_ptr;
      context.push_operand(f64_const_t{val});
   }
   void operator()(i32_store_t b) {
      dbg_print("i32.store");
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint32_t* store_loc = (uint32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      *store_loc = TO_UINT32(val);
   }
   void operator()(i32_store8_t b) {
      dbg_print("i32.store8");
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint8_t* store_loc = (uint8_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint8_t>(TO_UINT32(val));
   }
   void operator()(i32_store16_t b) {
      dbg_print("i32.store16");
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint16_t* store_loc = (uint16_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint16_t>(TO_UINT32(val));
   }
   void operator()(i64_store_t b) {
      dbg_print("i64.store");
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint64_t* store_loc = (uint64_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint64_t>(TO_UINT64(val));
   }
   void operator()(i64_store8_t b) {
      dbg_print("i64.store8");
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint8_t* store_loc = (uint8_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint8_t>(TO_UINT64(val));
   }
   void operator()(i64_store16_t b) {
      dbg_print("i64.store16");
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint16_t* store_loc = (uint16_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint16_t>(TO_UINT64(val));
   }
   void operator()(i64_store32_t b) {
      dbg_print("i64.store32");
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint32_t* store_loc = (uint32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint32_t>(TO_UINT64(val));
   }
   void operator()(f32_store_t b) {
      dbg_print("f32.store");
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint32_t* store_loc = (uint32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint32_t>(TO_FUINT32(val));
   }
   void operator()(f64_store_t b) {
      dbg_print("f64.store");
      context.inc_pc();
      const auto& val = context.pop_operand();
      const auto& ptr = context.pop_operand();
      uint64_t* store_loc = (uint64_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      *store_loc = static_cast<uint64_t>(TO_FUINT64(val));
   }
   void operator()(current_memory_t b) {
      dbg_print("current_memory");
      context.inc_pc();
   }
   void operator()(grow_memory_t b) {
      dbg_print("grow_memory");
      context.inc_pc();
   }
   void operator()(i32_const_t b) {
      dbg_print("i32.const");
      context.inc_pc();
      context.push_operand(b);
   }
   void operator()(i64_const_t b) {
      dbg_print("i64.const");
      context.inc_pc();
      context.push_operand(b);
   }
   void operator()(f32_const_t b) {
      dbg_print("f32.const");
      context.inc_pc();
      context.push_operand(b);
   }
   void operator()(f64_const_t b) {
      dbg_print("f64.const");
      context.inc_pc();
      context.push_operand(b);
   }
   void operator()(i32_eqz_t i) {
      dbg_print("i32.eqz");
      context.inc_pc();
      auto& op = context.peek_operand();
      auto& t = TO_UINT32(op);
      if (!t)
         t = 1;
      else
         t = 0;
   }
   void operator()(i32_eq_t b) {
      dbg_print("i32.eq");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      auto& t = TO_UINT32(lhs);
      if (t == TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
   }
   void operator()(i32_ne_t b) {
      dbg_print("i32.ne");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      auto& t = TO_UINT32(lhs);

      if (t != TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
   }
   void operator()(i32_lt_s_t b) {
      dbg_print("i32.lt_s");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      auto& t = TO_INT32(lhs);

      if (t < TO_INT32(rhs))
         t = 1;
      else
         t = 0;
   }
   void operator()(i32_lt_u_t b) {
      dbg_print("i32.lt_u");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      auto& t = TO_UINT32(lhs);

      if (t < TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
   }
   void operator()(i32_le_s_t b) {
      dbg_print("i32.le_s");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      auto& t = TO_INT32(lhs);

      if (t <= TO_INT32(rhs))
         t = 1;
      else
         t = 0;
   }
   void operator()(i32_le_u_t b) {
      dbg_print("i32.le_u");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      auto& t = TO_UINT32(lhs);

      if (t <= TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
   }
   void operator()(i32_gt_s_t b) {
      dbg_print("i32.gt_s");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      auto& t = TO_INT32(lhs);

      if (t > TO_INT32(rhs))
         t = 1;
      else
         t = 0;
   }
   void operator()(i32_gt_u_t b) {
      dbg_print("i32.gt_u");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      auto& t = TO_UINT32(lhs);

      if (t > TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
   }
   void operator()(i32_ge_s_t b) {
      dbg_print("i32.ge_s");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      auto& t = TO_INT32(lhs);

      if (t >= TO_INT32(rhs))
         t = 1;
      else
         t = 0;
   }
   void operator()(i32_ge_u_t b) {
      dbg_print("i32.ge_u");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      auto& t = TO_UINT32(lhs);

      if (t >= TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
   }
   void operator()(i64_eqz_t b) {
      dbg_print("i64.eqz");
      context.inc_pc();
      const auto& op = context.pop_operand();
      if (TO_UINT64(op) == 0)
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(i64_eq_t b) {
      dbg_print("i64.eq");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();

      if (TO_UINT64(rhs) == TO_UINT64(lhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(i64_ne_t b) {
      dbg_print("i64.ne");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();

      if (TO_UINT64(rhs) != TO_UINT64(lhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(i64_lt_s_t b) {
      dbg_print("i64.lt_s");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();

      if (TO_INT64(lhs) < TO_INT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(i64_lt_u_t b) {
      dbg_print("i64.lt_u");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();

      if (TO_UINT64(lhs) < TO_UINT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(i64_le_s_t b) {
      dbg_print("i64.le_s");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();

      if (TO_INT64(lhs) <= TO_INT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(i64_le_u_t b) {
      dbg_print("i64.le_u");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();

      if (TO_UINT64(lhs) <= TO_UINT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(i64_gt_s_t b) {
      dbg_print("i64.gt_s");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();

      if (TO_INT64(lhs) > TO_INT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(i64_gt_u_t b) {
      dbg_print("i64.gt_u");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();

      if (TO_UINT64(lhs) > TO_UINT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(i64_ge_s_t b) {
      dbg_print("i64.ge_s");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();

      if (TO_INT64(lhs) >= TO_INT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(i64_ge_u_t b) {
      dbg_print("i64.ge_u");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();

      if (TO_UINT64(lhs) >= TO_UINT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
   }
   void operator()(f32_eq_t b) {
      dbg_print("f32.eq");
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f32_eq(lhs, rhs)});
   }
   void operator()(f32_ne_t b) {
      dbg_print("f32.ne");
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f32_ne(lhs, rhs)});
   }
   void operator()(f32_lt_t b) {
      dbg_print("f32.lt");
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f32_lt(lhs, rhs)});
   }
   void operator()(f32_gt_t b) {
      dbg_print("f32.gt");
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)!_eosio_f32_le(lhs, rhs)});
   }
   void operator()(f32_le_t b) {
      dbg_print("f32.le");
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f32_le(lhs, rhs)});
   }
   void operator()(f32_ge_t b) {
      dbg_print("f32.ge");
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)!_eosio_f32_lt(lhs, rhs)});
   }
   void operator()(f64_eq_t b) {
      dbg_print("f64.eq");
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f64_eq(lhs, rhs)});
   }
   void operator()(f64_ne_t b) {
      dbg_print("f64.ne");
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f64_ne(lhs, rhs)});
   }
   void operator()(f64_lt_t b) {
      dbg_print("f64.lt");
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f64_lt(lhs, rhs)});
   }
   void operator()(f64_gt_t b) {
      dbg_print("f64.gt");
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)!_eosio_f64_le(lhs, rhs)});
   }
   void operator()(f64_le_t b) {
      dbg_print("f64.le");
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f64_le(lhs, rhs)});
   }
   void operator()(f64_ge_t b) {
      dbg_print("f64.ge");
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)!_eosio_f64_lt(lhs, rhs)});
   }
   void operator()(i32_clz_t) {
      dbg_print("i32.clz");
      context.inc_pc();
      auto& op = TO_UINT32(context.peek_operand());
      op = __builtin_clz(op);
   }
   void operator()(i32_ctz_t) {
      dbg_print("i32.ctz");
      context.inc_pc();
      auto& op = TO_UINT32(context.peek_operand());
      op = __builtin_ctz(op);
   }
   void operator()(i32_popcnt_t) {
      dbg_print("i32.popcnt");
      context.inc_pc();
      auto& op = TO_UINT32(context.peek_operand());
      op = __builtin_popcount(op);
   }
   void operator()(i32_add_t) {
      dbg_print("i32.add");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs += rhs;
   }
   void operator()(i32_sub_t) {
      dbg_print("i32.sub");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs -= rhs;
   }
   void operator()(i32_mul_t) {
      dbg_print("i32.mul");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs *= rhs;
   }
   void operator()(i32_div_s_t) {
      dbg_print("i32.div_s");
      context.inc_pc();
      const auto& rhs = TO_INT32(context.pop_operand());
      auto& lhs = TO_INT32(context.peek_operand());
      EOS_WB_ASSERT(rhs == 0, wasm_interpreter_exception, "i32.div_s divide by zero");
      EOS_WB_ASSERT(!(lhs == std::numeric_limits<int32_t>::max() && rhs == -1), wasm_interpreter_exception, "i32.div_s traps when I32_MAX/-1");
      lhs /= rhs;
   }
   void operator()(i32_div_u_t) {
      dbg_print("i32.div_u");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      EOS_WB_ASSERT(rhs == 0, wasm_interpreter_exception, "i32.div_u divide by zero");
      lhs /= rhs;
   }
   void operator()(i32_rem_s_t) {
      dbg_print("i32.rem_s");
      context.inc_pc();
      const auto& rhs = TO_INT32(context.pop_operand());
      auto& lhs = TO_INT32(context.peek_operand());
      EOS_WB_ASSERT(rhs == 0, wasm_interpreter_exception, "i32.rem_s divide by zero");
      if (UNLIKELY(lhs == std::numeric_limits<int32_t>::max() && rhs == -1))
         lhs = 0;
      else
         lhs %= rhs;
   }
   void operator()(i32_rem_u_t) {
      dbg_print("i32.rem_u");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      EOS_WB_ASSERT(rhs == 0, wasm_interpreter_exception, "i32.rem_u divide by zero");
      lhs %= rhs;
   }
   void operator()(i32_and_t) {
      dbg_print("i32.and");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs &= rhs;
   }
   void operator()(i32_or_t) {
      dbg_print("i32.or");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs |= rhs;
   }
   void operator()(i32_xor_t) {
      dbg_print("i32.xor");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs ^= rhs;
   }
   void operator()(i32_shl_t) {
      dbg_print("i32.shl");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs <<= rhs;
   }
   void operator()(i32_shr_s_t) {
      dbg_print("i32.shr_s");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_INT32(context.peek_operand());
      lhs >>= rhs;
   }
   void operator()(i32_shr_u_t) {
      dbg_print("i32.shr_u");
      context.inc_pc();
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      lhs >>= rhs;
   }
   void operator()(i32_rotl_t) {
      dbg_print("i32.rotl");
      context.inc_pc();
      static constexpr uint32_t mask = (8*sizeof(uint32_t)-1);
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      uint32_t c = rhs;
      c &= mask;
      lhs = (lhs << c) | (lhs >> ((-c) & mask));
   }
   void operator()(i32_rotr_t) {
      dbg_print("i32.rotr");
      context.inc_pc();
      static constexpr uint32_t mask = (8*sizeof(uint32_t)-1);
      const auto& rhs = TO_UINT32(context.pop_operand());
      auto& lhs = TO_UINT32(context.peek_operand());
      uint32_t c = rhs;
      c &= mask;
      lhs = (lhs >> c) | (lhs << ((-c) & mask));
   }
   void operator()(i64_clz_t) {
      dbg_print("i64.clz");
      context.inc_pc();
      auto& op = TO_UINT64(context.peek_operand());
      op = __builtin_clzll(op);
   }
   void operator()(i64_ctz_t) {
      dbg_print("i64.ctz");
      context.inc_pc();
      auto& op = TO_UINT64(context.peek_operand());
      op = __builtin_ctzll(op);
   }
   void operator()(i64_popcnt_t) {
      dbg_print("i64.ctz");
      context.inc_pc();
      auto& op = TO_UINT64(context.peek_operand());
      op = __builtin_popcountll(op);
   }
   void operator()(i64_add_t) {
      dbg_print("i64.add");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs += rhs;
   }
   void operator()(i64_sub_t) {
      dbg_print("i64.sub");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs -= rhs;
   }
   void operator()(i64_mul_t) {
      dbg_print("i64.mul");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs *= rhs;
   }
   void operator()(i64_div_s_t) {
      dbg_print("i64.div_s");
      context.inc_pc();
      const auto& rhs = TO_INT64(context.pop_operand());
      auto& lhs = TO_INT64(context.peek_operand());
      EOS_WB_ASSERT(rhs == 0, wasm_interpreter_exception, "i64.div_s divide by zero");
      EOS_WB_ASSERT(!(lhs == std::numeric_limits<int64_t>::max() && rhs == -1), wasm_interpreter_exception, "i64.div_s traps when I64_MAX/-1");
      lhs /= rhs;
   }
   void operator()(i64_div_u_t) {
      dbg_print("i64.div_u");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      EOS_WB_ASSERT(rhs == 0, wasm_interpreter_exception, "i64.div_u divide by zero");
      lhs /= rhs;
   }
   void operator()(i64_rem_s_t) {
      dbg_print("i64.rem_s");
      context.inc_pc();
      const auto& rhs = TO_INT64(context.pop_operand());
      auto& lhs = TO_INT64(context.peek_operand());
      EOS_WB_ASSERT(rhs == 0, wasm_interpreter_exception, "i64.rem_s divide by zero");
      if (UNLIKELY(lhs == std::numeric_limits<int64_t>::max() && rhs == -1))
         lhs = 0;
      else
         lhs %= rhs;
   }
   void operator()(i64_rem_u_t) {
      dbg_print("i64.rem_u");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      EOS_WB_ASSERT(rhs == 0, wasm_interpreter_exception, "i64.rem_s divide by zero");
      lhs %= rhs;
   }
   void operator()(i64_and_t) {
      dbg_print("i64.and");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs &= rhs;
   }
   void operator()(i64_or_t) {
      dbg_print("i64.or");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs |= rhs;
   }
   void operator()(i64_xor_t) {
      dbg_print("i64.xor");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs ^= rhs;
   }
   void operator()(i64_shl_t) {
      dbg_print("i64.shl");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs <<= rhs;
   }
   void operator()(i64_shr_s_t) {
      dbg_print("i64.shr_s");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_INT64(context.peek_operand());
      lhs >>= rhs;
   }
   void operator()(i64_shr_u_t) {
      dbg_print("i64.shr_u");
      context.inc_pc();
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      lhs >>= rhs;
   }
   void operator()(i64_rotl_t) {
      dbg_print("i64.rotl");
      context.inc_pc();
      static constexpr uint64_t mask = (8*sizeof(uint64_t)-1);
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      uint32_t c = rhs;
      c &= mask;
      lhs = (lhs << c) | (lhs >> (-c & mask));
   }
   void operator()(i64_rotr_t) {
      dbg_print("i64.rotr");
      context.inc_pc();
      static constexpr uint64_t mask = (8*sizeof(uint64_t)-1);
      const auto& rhs = TO_UINT64(context.pop_operand());
      auto& lhs = TO_UINT64(context.peek_operand());
      uint32_t c = rhs;
      c &= mask;
      lhs = (lhs >> c) | (lhs << (-c & mask));
   }
   void operator()(f32_abs_t) {
      dbg_print("f32.abs");
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_abs(op);
   }
   void operator()(f32_neg_t) {
      dbg_print("f32.neg");
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_neg(op);
   }
   void operator()(f32_ceil_t) {
      dbg_print("f32.ceil");
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_ceil(op);
   }
   void operator()(f32_floor_t) {
      dbg_print("f32.floor");
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_floor(op);
   }
   void operator()(f32_trunc_t) {
      dbg_print("f32.trunc");
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_trunc(op);
   }
   void operator()(f32_nearest_t) {
      dbg_print("f32.nearest");
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_nearest(op);
   }
   void operator()(f32_sqrt_t) {
      dbg_print("f32.sqrt");
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_sqrt(op);
   }
   void operator()(f32_add_t) {
      dbg_print("f32.add");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_add(lhs, TO_F32(rhs));
   }
   void operator()(f32_sub_t) {
      dbg_print("f32.sub");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_sub(lhs, TO_F32(rhs));
   }
   void operator()(f32_mul_t) {
      dbg_print("f32.mul");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_mul(lhs, TO_F32(rhs));
   }
   void operator()(f32_div_t) {
      dbg_print("f32.div");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_div(lhs, TO_F32(rhs));
   }
   void operator()(f32_min_t) {
      dbg_print("f32.min");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_min(lhs, TO_F32(rhs));
   }
   void operator()(f32_max_t) {
      dbg_print("f32.max");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_max(lhs, TO_F32(rhs));
   }
   void operator()(f32_copysign_t) {
      dbg_print("f32.copysign");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_copysign(lhs, TO_F32(rhs));
   }
   void operator()(f64_abs_t) {
      dbg_print("f64.abs");
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_abs(op);
   }
   void operator()(f64_neg_t) {
      dbg_print("f64.neg");
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_neg(op);
   }
   void operator()(f64_ceil_t) {
      dbg_print("f64.ceil");
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_ceil(op);
   }
   void operator()(f64_floor_t) {
      dbg_print("f64.floor");
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_floor(op);
   }
   void operator()(f64_trunc_t) {
      dbg_print("f64.trunc");
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_trunc(op);
   }
   void operator()(f64_nearest_t) {
      dbg_print("f64.nearest");
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_nearest(op);
   }
   void operator()(f64_sqrt_t) {
      dbg_print("f64.sqrt");
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_sqrt(op);
   }
   void operator()(f64_add_t) {
      dbg_print("f64.add");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_add(lhs, TO_F64(rhs));
   }
   void operator()(f64_sub_t) {
      dbg_print("f64.sub");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_sub(lhs, TO_F64(rhs));
   }
   void operator()(f64_mul_t) {
      dbg_print("f64.mul");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_mul(lhs, TO_F64(rhs));
   }
   void operator()(f64_div_t) {
      dbg_print("f64.div");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_div(lhs, TO_F64(rhs));
   }
   void operator()(f64_min_t) {
      dbg_print("f64.min");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_min(lhs, TO_F64(rhs));
   }
   void operator()(f64_max_t) {
      dbg_print("f64.max");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_max(lhs, TO_F64(rhs));
   }
   void operator()(f64_copysign_t) {
      dbg_print("f64.copysign");
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_copysign(lhs, TO_F64(rhs));
   }
   void operator()(i32_wrap_i64_t) {
      dbg_print("i32.wrap_i64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i32_const_t{static_cast<int32_t>(TO_INT64(op))}};
   }
   void operator()(i32_trunc_s_f32_t) {
      dbg_print("i32.trunc_s_f32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i32_const_t{_eosio_f32_trunc_i32s(TO_F32(op))}};
   }
   void operator()(i32_trunc_u_f32_t) {
      dbg_print("i32.trunc_u_f32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i32_const_t{_eosio_f32_trunc_i32u(TO_F32(op))}};
   }
   void operator()(i32_trunc_s_f64_t) {
      dbg_print("i32.trunc_s_f64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i32_const_t{_eosio_f64_trunc_i32s(TO_F64(op))}};
   }
   void operator()(i32_trunc_u_f64_t) {
      dbg_print("i32.trunc_u_f64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i32_const_t{_eosio_f64_trunc_i32u(TO_F64(op))}};
   }
   void operator()(i64_extend_s_i32_t) {
      dbg_print("i64.extend_s_i32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{static_cast<int64_t>(TO_INT32(op))}};
   }
   void operator()(i64_extend_u_i32_t) {
      dbg_print("i64.extend_u_i32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{static_cast<uint64_t>(TO_UINT32(op))}};
   }
   void operator()(i64_trunc_s_f32_t) {
      dbg_print("i64.trunc_s_f32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{_eosio_f32_trunc_i64s(TO_F64(op))}};
   }
   void operator()(i64_trunc_u_f32_t) {
      dbg_print("i64.trunc_u_f32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{_eosio_f32_trunc_i64u(TO_F64(op))}};
   }
   void operator()(i64_trunc_s_f64_t) {
      dbg_print("i64.trunc_s_f64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{_eosio_f64_trunc_i64s(TO_F64(op))}};
   }
   void operator()(i64_trunc_u_f64_t) {
      dbg_print("i64.trunc_u_f64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{_eosio_f64_trunc_i64u(TO_F64(op))}};
   }
   void operator()(f32_convert_s_i32_t) {
      dbg_print("f32.convert_s_i32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f32_const_t{_eosio_i32_to_f32(TO_INT32(op))}};
   }
   void operator()(f32_convert_u_i32_t) {
      dbg_print("f32.convert_u_i32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f32_const_t{_eosio_ui32_to_f32(TO_UINT32(op))}};
   }
   void operator()(f32_convert_s_i64_t) {
      dbg_print("f32.convert_s_i64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f32_const_t{_eosio_i64_to_f32(TO_INT64(op))}};
   }
   void operator()(f32_convert_u_i64_t) {
      dbg_print("f32.convert_u_i64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f32_const_t{_eosio_ui64_to_f32(TO_UINT64(op))}};
   }
   void operator()(f32_demote_f64_t) {
      dbg_print("f32.demote_f64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f32_const_t{_eosio_f64_demote(TO_F64(op))}};
   }
   void operator()(f64_convert_s_i32_t) {
      dbg_print("f64.convert_s_i32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f64_const_t{_eosio_i32_to_f64(TO_INT32(op))}};
   }
   void operator()(f64_convert_u_i32_t) {
      dbg_print("f64.convert_u_i32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f64_const_t{_eosio_ui32_to_f64(TO_UINT32(op))}};
   }
   void operator()(f64_convert_s_i64_t) {
      dbg_print("f64.convert_s_i64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f64_const_t{_eosio_i64_to_f64(TO_INT64(op))}};
   }
   void operator()(f64_convert_u_i64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f64_const_t{_eosio_ui64_to_f64(TO_UINT64(op))}};
      dbg_print("f64.convert_u_i64");
   }
   void operator()(f64_promote_f32_t) {
      dbg_print("f64.promote_f32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f64_const_t{_eosio_f32_promote(TO_F32(op))}};
   }
   void operator()(i32_reinterpret_f32_t) {
      dbg_print("i32.reinterpret/f32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = i32_const_t{TO_UINT32(op)};
   }
   void operator()(i64_reinterpret_f64_t) {
      dbg_print("i64.reinterpret/f64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = i64_const_t{TO_UINT64(op)};
   }
   void operator()(f32_reinterpret_i32_t) {
      dbg_print("f32.reinterpret/i32");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = i32_const_t{TO_FUINT32(op)};
   }
   void operator()(f64_reinterpret_i64_t) {
      dbg_print("f64.reinterpret/i64");
      context.inc_pc();
      auto& op = context.peek_operand();
      op = i64_const_t{TO_FUINT64(op)};
   }
   void operator()(error_t) {
      dbg_print("error");
      context.inc_pc();
   }

   uint32_t tab_width;
};

}} // ns eosio::wasm_backend
