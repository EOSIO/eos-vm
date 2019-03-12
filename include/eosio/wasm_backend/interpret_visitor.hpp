#pragma once

#include <eosio/wasm_backend/utils.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/wasm_stack.hpp>
#include <eosio/wasm_backend/execution_context.hpp>
#include <iostream>
#include <variant>
#include <sstream>

#include <eosio/wasm_backend/softfloat.hpp>

#define dbg_print print
//#define dbg_print

//struct float32_t {
//   uint32_t data;
//};
//
//namespace std {
//   template <>
//   struct is_floating_point<float32_t> {
//      static constexpr bool value = true;
//   };
//}
#define TO_INT32(X) \
   std::get<i32_const_t>(X).data.i

#define TO_INT64(X) \
   std::get<i64_const_t>(X).data.i

#define TO_UINT32(X) \
   std::get<i32_const_t>(X).data.ui

#define TO_UINT64(X) \
   std::get<i64_const_t>(X).data.ui

#define TO_FUINT32(X)                            \
   std::get<f32_const_t>(X).data.ui

#define TO_FUINT64(X)                            \
   std::get<f64_const_t>(X).data.ui

#define TO_F32(X) \
   std::get<f32_const_t>(X).data.f

#define TO_F64(X) \
   std::get<f64_const_t>(X).data.f

namespace eosio { namespace wasm_backend {

struct interpret_visitor {
   interpret_visitor(execution_context<interpret_visitor>& ec) : context(ec) {}
   execution_context<interpret_visitor>& context;
   std::stringstream  dbg_output;
   /*
   struct stack_elem_visitor {
      std::stringstream& dbg_output;
      stack_elem_visitor(std::stringstream& ss) : dbg_output(ss) {}
      void operator()(const block_t& ctrl) {
         dbg_output << "block : " << std::to_string(ctrl.data) << "\n";
      }
      void operator()(const loop_t& ctrl) {
         dbg_output << "loop : " << std::to_string(ctrl.data) << "\n";
      }
      void operator()(const if__t& ctrl) {
         dbg_output << "if : " << std::to_string(ctrl.data) << "\n";
      }
      template <typename T>
      void operator()(T) {
      }
   } _elem_visitor{dbg_output};
   */
   void print(const std::string& s) {
      //std::string tb(tab_width, '\t');
      dbg_output << s << '\n';
   }
   void operator()(unreachable_t) {
      context.inc_pc();
      throw wasm_interpreter_exception{"unreachable"};
   }
   void operator()(nop_t) {
      context.inc_pc();
      dbg_output << "nop {" << context.get_pc() << "}\n";
   }
   void operator()(fend_t) {
      context.apply_pop_call();
      dbg_output << "fend {" << context.get_pc() << "}\n";
   }
   void operator()(end_t) {
      const auto& label = context.pop_label();
      std::visit(overloaded {
         [&](const block_t& b) {
            context.eat_labels(b.index);
         }, [&](const loop_t& l) {
            context.eat_labels(l.index);
         },[&](const if__t& i) {
            context.eat_labels(i.index);
         }, [&](auto) {
            throw wasm_interpreter_exception{"expected control structure"};
         }
      }, label);
      context.inc_pc();
      dbg_output << "end {" << context.get_pc() << "}\n";
   }
   void operator()(return__t) {
      context.apply_pop_call();
      dbg_output << "return {" << context.get_pc() << "}\n";
   }
   void operator()(block_t bt) {
      context.inc_pc();
      bt.index = context.current_label_index();
      context.push_label(bt);
      dbg_output << "block {" << context.get_pc() << "} " << bt.data << " " << bt.pc << "\n";
   }
   void operator()(loop_t lt) {
      context.inc_pc();
      lt.index = context.current_label_index();
      context.push_label(lt);
      dbg_output << "loop {" << context.get_pc() << "} " << lt.data << " " << lt.pc << "\n";
   }
   void operator()(if__t it) {
      context.inc_pc();
      it.index = context.current_label_index();
      const auto& op = context.pop_operand();
      if (!TO_UINT32(op))
         context.set_pc(it.pc);
      context.push_label(it);
      dbg_output << "if {" << context.get_pc() << "} " << it.data << " " << it.pc << "\n";
   }
   void operator()(else__t et) {
      context.set_relative_pc(et.pc);
      dbg_output << "else {" << context.get_pc() << "} " << et.data << " " << et.pc << "\n";
   }
   void operator()(br_t b) {
      context.jump(b.data);
      dbg_output << "br {" << context.get_pc() << "} " << b.data << "\n";
   }
   void operator()(br_if_t b) {
      dbg_output << "br.if {" << context.get_pc() << "} " << b.data << "\n";
      const auto& val = context.pop_operand();
      if (context.is_true(val))
         context.jump(b.data);
      else
         context.inc_pc();
   }
   void operator()(br_table_t b) {
      const auto& in = TO_UINT32(context.pop_operand());
      if (in < b.target_table.size())
         context.jump(b.target_table[in]);
      else
         context.jump(b.default_target);
      dbg_output << "br.table\n";
   }
   void operator()(call_t b) {
      context.call(b.index);
      // TODO place these in parser
      //EOS_WB_ASSERT(b.index < funcs_size, wasm_interpreter_exception, "call index out of bounds");
      /*
      if (ftype.return_count > 0) {
         EOS_WB_ASSERT(ftype.return_count <= 1, wasm_interpreter_exception, "mvp only supports single value returns");
         context.push_operand(ret_val);
      }
      */
      dbg_output << "call {" << context.get_pc() << "} " << b.index << "\n";
   }
   void operator()(call_indirect_t b) {
      const auto& op = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(op), wasm_interpreter_exception, "call_indirect expected i32 operand");
      context.call(context.table_elem(TO_UINT32(op)));
      dbg_output << "call_indirect " << b.index << " " << context.get_module().tables[b.index].element_type << "\n";
   }
   void operator()(drop_t b) {
      context.pop_operand();
      context.inc_pc();
      dbg_output << "drop ";
   }
   void operator()(select_t b) {
      const auto& c = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(c), wasm_interpreter_exception, "select expected i32 on stack");
      const auto& v2 = context.pop_operand();
      if (TO_UINT32(c) == 0) {
         context.peek_operand() = v2;
      }
      context.inc_pc();
      dbg_output << "select\n";
   }
   void operator()(get_local_t b) {
      context.inc_pc();
      context.push_operand(context.get_operand(b.index));
      dbg_output << "get_local " << b.index << TO_UINT32(context.get_operand(b.index)) << "\n";
   }
   void operator()(set_local_t b) {
      context.inc_pc();
      context.set_operand(b.index, context.pop_operand());
      dbg_output << "set_local\n";
   }
   void operator()(tee_local_t b) {
      context.inc_pc();
      const auto& op = context.pop_operand();
      context.set_operand(b.index, op);
      context.push_operand(op);
      dbg_output << "tee_local\n";
   }
   void operator()(get_global_t b) {
      context.inc_pc();
      const auto& gl = context.get_global(b.index);
      context.push_operand(gl);
      dbg_output << "get_global\n";
   }
   void operator()(set_global_t b) {
      context.inc_pc();
      const auto& op = context.pop_operand();
      context.set_global(b.index, op);
      dbg_output << "set_global\n";
   }
   void operator()(i32_load_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i32.load expected i32 operand");
      uint32_t val = *(uint32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      uint32_t* store_loc = (uint32_t*)context.linear_memory()+b.offset+TO_UINT32(ptr)-sizeof(uint32_t);
      std::cout << "i32.load loc " << store_loc << "\n"; 
      context.push_operand(i32_const_t{val});
      dbg_output << "i32.load " << " align " << b.flags_align << " offset " << b.offset << " value " << val << "\n";
   }
   void operator()(i32_load8_s_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i32.load8_s expected i32 operand");
      int8_t val = *(int8_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i32_const_t{*(uint32_t*)&val});
      dbg_output << "i32.load8_s\n";
   }
   void operator()(i32_load16_s_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i32.load16_s expected i32 operand");
      int16_t val = *(int16_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i32_const_t{*(uint32_t*)&val});
      dbg_output << "i32.load16_s " << " align " << b.flags_align << " offset " << b.offset << " value " << (int)val << "\n";
   }
   void operator()(i32_load8_u_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i32.load8_u expected i32 operand");
      uint8_t val = *(uint8_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i32_const_t{val});
      dbg_output << "i32.load8_u\n";
   }
   void operator()(i32_load16_u_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i32.load16_u expected i32 operand");
      uint16_t val = *(uint16_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i32_const_t{val});
      dbg_output << "i32.load16_u " << " align " << b.flags_align << " offset " << b.offset << " value " << (int)val << "\n";
   }
   void operator()(i64_load_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.load expected i32 operand");
      uint64_t val = *(uint64_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i64_const_t{val});
      dbg_output << "i64.load " << " align " << b.flags_align << " offset " << b.offset << " value " << val << "\n";
   }
   void operator()(i64_load8_s_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.load8_s expected i32 operand");
      int8_t val = *(int8_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i64_const_t{*(uint64_t*)&val});
      dbg_output << "i64.load8_s " << " align " << b.flags_align << " offset " << b.offset << " value " << (long long)val << "\n";
   }
   void operator()(i64_load16_s_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.load16_s expected i32 operand");
      int16_t val = *(int16_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i64_const_t{*(uint64_t*)&val});
      dbg_output << "i64.load16_s " << " align " << b.flags_align << " offset " << b.offset << " value " << (long long)val << "\n";
   }
   void operator()(i64_load32_s_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.load32_s expected i32 operand");
      int32_t val = *(int32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i64_const_t{*(uint64_t*)&val});
      dbg_output << "i64.load32_s " << " align " << b.flags_align << " offset " << b.offset << " value " << (long long)val << "\n";
   }
   void operator()(i64_load8_u_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.load8_u expected i32 operand");
      uint8_t val = *(uint8_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i64_const_t{static_cast<uint64_t>(val)});
      dbg_output << "i64.load8_u " << " align " << b.flags_align << " offset " << b.offset << " value " << (long long)val << "\n";
   }
   void operator()(i64_load16_u_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.load16_u expected i32 operand");
      uint16_t val = *(uint16_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i64_const_t{static_cast<uint64_t>(val)});
      dbg_output << "i64.load16_u " << " align " << b.flags_align << " offset " << b.offset << " value " << (long long)val << "\n";
   }
   void operator()(i64_load32_u_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.load32_u expected i32 operand");
      uint32_t val = *(uint32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(i64_const_t{static_cast<uint64_t>(val)});
      dbg_output << "i64.load32_u " << " align " << b.flags_align << " offset " << b.offset << " value " << (long long)val << "\n";
   }
   void operator()(f32_load_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "f32.load expected i32 operand");
      uint32_t val = *(uint32_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(f32_const_t{val});
      dbg_output << "f32.load " << " align " << b.flags_align << " offset " << b.offset << " value " << *(float*)&val << "\n";
   }
   void operator()(f64_load_t b) {
      context.inc_pc();
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "f64.load expected i32 operand");
      uint64_t val = *(uint64_t*)(context.linear_memory()+b.offset+TO_UINT32(ptr));
      context.push_operand(f64_const_t{val});
      dbg_output << "f64.load " << " align " << b.flags_align << " offset " << b.offset << " value " << *(double*)&val << "\n";
   }
   void operator()(i32_store_t b) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(val), wasm_interpreter_exception, "i32.store expected i32 operand");
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i32.store expected i32 operand");
      uint32_t* store_loc = (uint32_t*)context.linear_memory()+b.offset+TO_UINT32(ptr);
      *store_loc = TO_UINT32(val);
      std::cout << "loc " << store_loc << " " << TO_UINT32(val) << " " << *store_loc << "\n";
      dbg_output << "i32.store\n";
   }
   void operator()(i32_store8_t b) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(val), wasm_interpreter_exception, "i32.store8 expected i32 operand");
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i32.store8 expected i32 operand");

      *((uint8_t*)context.linear_memory()+b.offset+TO_UINT32(ptr)) = static_cast<uint8_t>(TO_UINT32(val));
      dbg_output << "i32.store8\n";
   }
   void operator()(i32_store16_t b) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(val), wasm_interpreter_exception, "i32.store16 expected i32 operand");
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i32.store16 expected i32 operand");

      *((uint16_t*)context.linear_memory()+b.offset+TO_UINT32(ptr)) = static_cast<uint16_t>(TO_UINT32(val));
      dbg_output << "i32.store16\n";
   }
   void operator()(i64_store_t b) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(val), wasm_interpreter_exception, "i64.store expected i64 operand");
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.store expected i32 operand");

      *((uint64_t*)context.linear_memory()+b.offset+TO_UINT32(ptr)) = TO_UINT64(val);
      dbg_output << "i64.store\n";
   }
   void operator()(i64_store8_t b) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(val), wasm_interpreter_exception, "i64.store expected i64 operand");
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.store8 expected i32 operand");

      *((uint8_t*)context.linear_memory()+b.offset+TO_UINT32(ptr)) = static_cast<uint8_t>(TO_UINT64(val));
      dbg_output << "i64.store8\n";
   }
   void operator()(i64_store16_t b) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(val), wasm_interpreter_exception, "i64.store expected i64 operand");
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.store16 expected i32 operand");

      *((uint16_t*)context.linear_memory()+b.offset+TO_UINT32(ptr)) = static_cast<uint16_t>(TO_UINT64(val));
      dbg_output << "i64.store16\n";
   }
   void operator()(i64_store32_t b) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(val), wasm_interpreter_exception, "i64.store expected i64 operand");
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "i64.store32 expected i32 operand");

      *((uint32_t*)context.linear_memory()+b.offset+TO_UINT32(ptr)) = static_cast<uint32_t>(TO_UINT64(val));
      dbg_output << "i64.store32\n";
   }
   void operator()(f32_store_t b) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      EOS_WB_ASSERT(is_a<f32_const_t>(val), wasm_interpreter_exception, "f32.store expected f32 operand");
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "f32.store expected i32 operand");

      *((uint32_t*)context.linear_memory()+b.offset+TO_UINT32(ptr)) = TO_FUINT32(val);
      dbg_output << "f32.store\n";
   }
   void operator()(f64_store_t b) {
      context.inc_pc();
      const auto& val = context.pop_operand();
      EOS_WB_ASSERT(is_a<f32_const_t>(val), wasm_interpreter_exception, "f64.store expected f64 operand");
      const auto& ptr = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(ptr), wasm_interpreter_exception, "f64.store expected i32 operand");

      *((uint64_t*)context.linear_memory()+b.offset+TO_UINT32(ptr)) = TO_FUINT64(val);
      dbg_output << "f64.store\n";
   }
   void operator()(current_memory_t b) {
      context.inc_pc();
      dbg_print("current_memory ");
   }
   void operator()(grow_memory_t b) {
      context.inc_pc();
      dbg_print("grow_memory ");
   }
   void operator()(i32_const_t b) {
      context.inc_pc();
      context.push_operand(b);
      dbg_output << "i32.const " << b.data.i << "\n";
   }
   void operator()(i64_const_t b) {
      context.inc_pc();
      context.push_operand(b);
      dbg_output << "i64.const " << b.data.i << "\n";
   }
   void operator()(f32_const_t b) {
      context.inc_pc();
      context.push_operand(b);
      dbg_output << "f32.const " << b.data.f << "\n";
   }
   void operator()(f64_const_t b) {
      context.inc_pc();
      context.push_operand(b);
      dbg_output << "f64.const " << b.data.f << "\n";
   }
   void operator()(i32_eqz_t i) {
      context.inc_pc();
      auto& op = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(op), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_UINT32(op);
      if (!t)
         t = 1;
      else
         t = 0;
      dbg_output << "i32.eqz\n";
   }
   void operator()(i32_eq_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "expected i32 operand");
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_UINT32(lhs);
      if (t == TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
      dbg_output << "i32.eq\n";
   }
   void operator()(i32_ne_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "expected i32 operand");
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_UINT32(lhs);

      if (t != TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
      dbg_output << "i32.ne\n";
   }
   void operator()(i32_lt_s_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "expected i32 operand");
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_INT32(lhs);

      if (t < TO_INT32(rhs))
         t = 1;
      else
         t = 0;
      dbg_output << "i32.lt_s\n";
   }
   void operator()(i32_lt_u_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "expected i32 operand");
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_UINT32(lhs);

      if (t < TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
      dbg_output << "i32.lt_s\n";
   }
   void operator()(i32_le_s_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "expected i32 operand");
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_INT32(lhs);

      if (t <= TO_INT32(rhs))
         t = 1;
      else
         t = 0;
      dbg_output << "i32.le_s\n";
   }
   void operator()(i32_le_u_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "expected i32 operand");
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_UINT32(lhs);

      if (t <= TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
      dbg_output << "i32.le_u\n";
   }
   void operator()(i32_gt_s_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "expected i32 operand");
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_INT32(lhs);

      if (t > TO_INT32(rhs))
         t = 1;
      else
         t = 0;
      dbg_output << "i32.gt_s\n";
   }
   void operator()(i32_gt_u_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "expected i32 operand");
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_UINT32(lhs);

      if (t > TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
      dbg_output << "i32.gt_u\n";
   }
   void operator()(i32_ge_s_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "expected i32 operand");
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_INT32(lhs);

      if (t >= TO_INT32(rhs))
         t = 1;
      else
         t = 0;
      dbg_output << "i32.ge_s\n";
   }
   void operator()(i32_ge_u_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "expected i32 operand");
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "expected i32 operand");
      auto& t = TO_UINT32(lhs);

      if (t >= TO_UINT32(rhs))
         t = 1;
      else
         t = 0;
      dbg_output << "i32.ge_u\n";
   }
   void operator()(i64_eqz_t b) {
      context.inc_pc();
      const auto& op = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(op), wasm_interpreter_exception, "expected i64 operand");
      if (TO_UINT64(op) == 0)
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
      dbg_output << "i64.eqz\n";
   }
   void operator()(i64_eq_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "expected i64 operand");
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "expected i64 operand");

      if (TO_UINT64(rhs) == TO_UINT64(lhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
      dbg_output << "i64.eq\n"; 
   }
   void operator()(i64_ne_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "expected i64 operand");
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "expected i64 operand");

      if (TO_UINT64(rhs) != TO_UINT64(lhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});
      dbg_output << "i64.ne\n";
   }
   void operator()(i64_lt_s_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "expected i64 operand");
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "expected i64 operand");

      if (TO_INT64(lhs) < TO_INT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});

      dbg_output << "i64.lt_s\n";

   }
   void operator()(i64_lt_u_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "expected i64 operand");
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "expected i64 operand");

      if (TO_UINT64(lhs) < TO_UINT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});

      dbg_output << "i64.lt_u\n";
   }
   void operator()(i64_le_s_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "expected i64 operand");
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "expected i64 operand");

      if (TO_INT64(lhs) <= TO_INT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});

      dbg_output << "i64.le_s\n";
   }
   void operator()(i64_le_u_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "expected i64 operand");
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "expected i64 operand");

      if (TO_UINT64(lhs) <= TO_UINT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});

      dbg_output << "i64.le_u\n";
   }
   void operator()(i64_gt_s_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "expected i64 operand");
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "expected i64 operand");

      if (TO_INT64(lhs) > TO_INT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});

      dbg_output << "i64.gt_s\n";
   }
   void operator()(i64_gt_u_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "expected i64 operand");
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "expected i64 operand");

      if (TO_UINT64(lhs) > TO_UINT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});

      dbg_output << "i64.gt_u\n";
   }
   void operator()(i64_ge_s_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "expected i64 operand");
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "expected i64 operand");

      if (TO_INT64(lhs) >= TO_INT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});

      dbg_output << "i64.ge_s\n";
   }
   void operator()(i64_ge_u_t b) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "expected i64 operand");
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "expected i64 operand");

      if (TO_UINT64(lhs) >= TO_UINT64(rhs))
         context.push_operand(i32_const_t{1});
      else
         context.push_operand(i32_const_t{0});

      dbg_output << "i64.ge_u\n";
   }
   void operator()(f32_eq_t b) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f32_eq(lhs, rhs)});
      dbg_print("f32.eq");
   }
   void operator()(f32_ne_t b) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f32_ne(lhs, rhs)});
      dbg_print("f32.ne");
   }
   void operator()(f32_lt_t b) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f32_lt(lhs, rhs)});
      dbg_print("f32.lt");
   }
   void operator()(f32_gt_t b) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)!_eosio_f32_le(lhs, rhs)});
      dbg_print("f32.gt");
   }
   void operator()(f32_le_t b) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f32_le(lhs, rhs)});
      dbg_print("f32.le");
   }
   void operator()(f32_ge_t b) {
      context.inc_pc();
      const auto& rhs = TO_F32(context.pop_operand());
      const auto& lhs = TO_F32(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)!_eosio_f32_lt(lhs, rhs)});
      dbg_print("f32.ge");
   }
   void operator()(f64_eq_t b) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f64_eq(lhs, rhs)});
      dbg_print("f64.eq");
   }
   void operator()(f64_ne_t b) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f64_ne(lhs, rhs)});
      dbg_print("f64.ne");
   }
   void operator()(f64_lt_t b) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f64_lt(lhs, rhs)});
      dbg_print("f64.lt");
   }
   void operator()(f64_gt_t b) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)!_eosio_f64_le(lhs, rhs)});
      dbg_print("f64.gt");
   }
   void operator()(f64_le_t b) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)_eosio_f64_le(lhs, rhs)});
      dbg_print("f64.le");
   }
   void operator()(f64_ge_t b) {
      context.inc_pc();
      const auto& rhs = TO_F64(context.pop_operand());
      const auto& lhs = TO_F64(context.pop_operand());
      context.push_operand(i32_const_t{(uint32_t)!_eosio_f64_lt(lhs, rhs)});
      dbg_print("f64.ge");
   }
   void operator()(i32_clz_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(op), wasm_interpreter_exception, "i32.clz expected i32 operand");
      auto& o = TO_UINT32(op);
      o = __builtin_clz(o);
      dbg_output << "i32.clz " << o << "\n";
   }
   void operator()(i32_ctz_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(op), wasm_interpreter_exception, "i32.ctz expected i32 operand");
      auto& o = TO_UINT32(op);
      o = __builtin_ctz(o);
      dbg_output << "i32.ctz " << o << "\n";
   }
   void operator()(i32_popcnt_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(op), wasm_interpreter_exception, "i32.popcnt expected i32 operand");
      auto& o = TO_UINT32(op);
      o = __builtin_popcount(o);
      dbg_output << "i32.popcnt " << o << "\n";
   }
   void operator()(i32_add_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.add expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.add expected i32 operand");
      TO_UINT32(lhs) += TO_UINT32(rhs);
      dbg_output << "i32.add\n";
   }
   void operator()(i32_sub_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.sub expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.sub expected i32 operand");
      TO_UINT32(lhs) -= TO_UINT32(rhs);
      dbg_output << "i32.sub\n";
   }
   void operator()(i32_mul_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.mul expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.mul expected i32 operand");
      TO_UINT32(lhs) *= TO_UINT32(rhs);
      dbg_output << "i32.mul\n";
   }
   void operator()(i32_div_s_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.div_s expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.div_s expected i32 operand");
      const int32_t& i2 = TO_INT32(rhs);
      int32_t& i1       = TO_INT32(lhs);
      EOS_WB_ASSERT(i2 == 0, wasm_interpreter_exception, "i32.div_s divide by zero");
      EOS_WB_ASSERT(!(i1 == std::numeric_limits<int32_t>::max() && i2 == -1), wasm_interpreter_exception, "i32.div_s traps when I32_MAX/-1");
      i1 /= i2;
      dbg_output << "i32.div_s\n";
   }
   void operator()(i32_div_u_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.div_u expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.div_u expected i32 operand");
      const uint32_t& i2 = TO_UINT32(rhs);
      EOS_WB_ASSERT(i2 == 0, wasm_interpreter_exception, "i32.div_u divide by zero");
      TO_UINT32(lhs) /= i2;
      dbg_output << "i32.div_u\n";
   }
   void operator()(i32_rem_s_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.rem_s expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.rem_s expected i32 operand");
      const int32_t& i2 = TO_INT32(rhs);
      int32_t& i1       = TO_INT32(lhs);
      EOS_WB_ASSERT(i2 == 0, wasm_interpreter_exception, "i32.rem_s divide by zero");
      if (UNLIKELY(i1 == std::numeric_limits<int32_t>::max() && i2 == -1))
         i1 = 0;
      else
         i1 %= i2;
      dbg_output << "i32.rem_s\n";
   }
   void operator()(i32_rem_u_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.rem_u expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.rem_u expected i32 operand");
      const uint32_t& i2 = TO_UINT32(rhs);
      EOS_WB_ASSERT(i2 == 0, wasm_interpreter_exception, "i32.rem_u divide by zero");
      TO_UINT32(lhs) %= i2;
      dbg_output << "i32.rem_u\n"; 
   }
   void operator()(i32_and_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.and expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.and expected i32 operand");
      TO_UINT32(lhs) &= TO_UINT32(rhs); 
      dbg_output << "i32.and\n";
   }
   void operator()(i32_or_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.or expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.or expected i32 operand");
      TO_UINT32(lhs) -= TO_UINT32(rhs);
      dbg_output << "i32.or\n";
   }
   void operator()(i32_xor_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.xor expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.xor expected i32 operand");
      TO_UINT32(lhs) -= TO_UINT32(rhs);
      dbg_output << "i32.xor\n";
   }
   void operator()(i32_shl_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.shl expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.shl expected i32 operand");
      TO_UINT32(lhs) -= TO_UINT32(rhs);
      dbg_output << "i32.shl\n";
   }
   void operator()(i32_shr_s_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.shr_s expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.shr_s expected i32 operand");
      TO_INT32(lhs) >>= TO_INT32(rhs);
      dbg_output << "i32.shr_s\n";
   }
   void operator()(i32_shr_u_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.shr_u expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.shr_u expected i32 operand");
      TO_UINT32(lhs) >>= TO_UINT32(rhs);
      dbg_output << "i32.shr_u\n";
   }
   void operator()(i32_rotl_t) {
      context.inc_pc();
      static constexpr uint32_t mask = (8*sizeof(uint32_t)-1);
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.rotl expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.rotl expected i32 operand");
      auto& op = TO_UINT32(lhs);
      uint32_t c = TO_UINT32(rhs);
      c &= mask;
      op = (op << c) | (op >> ((-c) & mask));
      dbg_output << "i32.rotl\n";
   }
   void operator()(i32_rotr_t) {
      context.inc_pc();
      static constexpr uint32_t mask = (8*sizeof(uint32_t)-1);
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(rhs), wasm_interpreter_exception, "i32.rotr expected i32 operand");
      EOS_WB_ASSERT(is_a<i32_const_t>(lhs), wasm_interpreter_exception, "i32.rotr expected i32 operand");
      auto& op = TO_UINT32(lhs);
      uint32_t c = TO_UINT32(rhs);
      c &= mask;
      op = (op >> c) | (op << ((-c) & mask));
      dbg_output << "i32.rotr\n";
   }
   void operator()(i64_clz_t) {
      context.inc_pc();
      const auto& op = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(op), wasm_interpreter_exception, "i64.clz expected i64 operand");
      context.push_operand(i64_const_t{static_cast<int64_t>(__builtin_clzll(TO_UINT64(op)))});
      dbg_output << "i64.clz\n";
   }
   void operator()(i64_ctz_t) {
      context.inc_pc();
      const auto& op = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(op), wasm_interpreter_exception, "i64.ctz expected i64 operand");
      context.push_operand(i64_const_t{static_cast<int64_t>(__builtin_ctzll(TO_UINT64(op)))});
      dbg_output << "i64.ctz\n";
   }
   void operator()(i64_popcnt_t) {
      context.inc_pc();
      const auto& op = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(op), wasm_interpreter_exception, "i64.popcnt expected i64 operand");
      context.push_operand(i64_const_t{static_cast<int64_t>(__builtin_popcountll(TO_UINT64(op)))});
      dbg_output << "i64.popcnt\n";
   }
   void operator()(i64_add_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.add expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.add expected i64 operand");
      uint64_t sum = TO_UINT64(lhs) + TO_UINT64(rhs);
      context.push_operand(i64_const_t{sum});
      dbg_output << "i64.add\n";
   }
   void operator()(i64_sub_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.sub expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.sub expected i64 operand");
      uint64_t dif = TO_UINT64(lhs) - TO_UINT64(rhs);
      context.push_operand(i64_const_t{dif});
      dbg_output << "i64.sub\n";
   }
   void operator()(i64_mul_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.mul expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.mul expected i64 operand");
      uint64_t prod = TO_UINT64(lhs) * TO_UINT64(rhs);
      context.push_operand(i64_const_t{prod});
      dbg_output << "i64.mul\n";
   }
   void operator()(i64_div_s_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.div_s expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.div_s expected i64 operand");
      const int64_t& i2 = TO_INT64(rhs);
      const int64_t& i1 = TO_INT64(lhs);
      EOS_WB_ASSERT(i2 == 0, wasm_interpreter_exception, "i64.div_s divide by zero");
      EOS_WB_ASSERT(!(i1 == std::numeric_limits<int64_t>::max() && i2 == -1), wasm_interpreter_exception, "i64.div_s traps when I64_MAX/-1");
      int64_t q = i1 / i2;
      context.push_operand(i64_const_t{q});
      dbg_output << "i64.div_s\n";
   }
   void operator()(i64_div_u_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.div_u expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.div_u expected i64 operand");
      const uint64_t& i2 = TO_UINT64(rhs);
      EOS_WB_ASSERT(i2 == 0, wasm_interpreter_exception, "i64.div_u divide by zero");
      uint64_t q = TO_UINT32(lhs)/i2;
      context.push_operand(i64_const_t{q});
      dbg_output << "i64.div_u\n";
   }
   void operator()(i64_rem_s_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.rem_s expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.rem_s expected i64 operand");
      const int64_t& i2 = TO_INT64(rhs);
      const int64_t& i1 = TO_INT64(lhs);
      EOS_WB_ASSERT(i2 == 0, wasm_interpreter_exception, "i64.rem_s divide by zero");
      int64_t rem;
      if (UNLIKELY(i1 == std::numeric_limits<int64_t>::max() && i2 == -1))
         rem = 0;
      else
         rem = i1 % i2;
      context.push_operand(i64_const_t{rem});
      dbg_output << "i64.rem_s\n";
   }
   void operator()(i64_rem_u_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.rem_u expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.rem_u expected i64 operand");
      uint64_t rem = TO_UINT64(lhs) % TO_UINT64(rhs);
      context.push_operand(i64_const_t{rem});
      dbg_output << "i64.rem_u\n";
   }
   void operator()(i64_and_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.and expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.and expected i64 operand");
      uint64_t o = TO_UINT64(lhs) & TO_UINT64(rhs);
      context.push_operand(i64_const_t{o});
      dbg_output << "i64.and\n";
   }
   void operator()(i64_or_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.or expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.or expected i64 operand");
      uint64_t o = TO_UINT64(lhs) | TO_UINT64(rhs);
      context.push_operand(i64_const_t{o});
      dbg_output << "i64.or\n";
   }
   void operator()(i64_xor_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.xor expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.xor expected i64 operand");
      uint64_t o = TO_UINT64(lhs) ^ TO_UINT64(rhs);
      context.push_operand(i64_const_t{o});
      dbg_output << "i64.xor\n";
   }
   void operator()(i64_shl_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.shl expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.shl expected i64 operand");
      uint64_t o = TO_UINT64(lhs) << TO_UINT64(rhs);
      context.push_operand(i64_const_t{o});
      dbg_output << "i64.shl\n";
   }
   void operator()(i64_shr_s_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.shr_s expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.shr_s expected i64 operand");
      int64_t o = TO_INT64(lhs) >> TO_UINT64(rhs);
      context.push_operand(i64_const_t{o});
      dbg_output << "i64.shr_s\n";
   }
   void operator()(i64_shr_u_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      const auto& lhs = context.pop_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.shr_u expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.shr_u expected i64 operand");
      uint64_t o = TO_UINT64(lhs) >> TO_UINT64(rhs);
      context.push_operand(i64_const_t{o});
      dbg_output << "i64.shr_u\n";
   }
   void operator()(i64_rotl_t) {
      context.inc_pc();
      static constexpr uint64_t mask = (8*sizeof(uint64_t)-1);
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.rotl expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.rotl expected i64 operand");
      uint32_t c = TO_UINT64(rhs);
      c &= mask;
      uint64_t rot = (TO_UINT64(lhs) << c) | (TO_UINT64(lhs) >> ((-c) & mask));
      context.push_operand(i64_const_t{rot});
      dbg_output << "i64.rotl\n";
   }
   void operator()(i64_rotr_t) {
      context.inc_pc();
      static constexpr uint64_t mask = (8*sizeof(uint64_t)-1);
      const auto& rhs = context.pop_operand();
      auto& lhs = context.peek_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(rhs), wasm_interpreter_exception, "i64.rotl expected i64 operand");
      EOS_WB_ASSERT(is_a<i64_const_t>(lhs), wasm_interpreter_exception, "i64.rotl expected i64 operand");
      uint32_t c = TO_UINT64(rhs);
      c &= mask;
      uint64_t rot = (TO_UINT64(lhs) >> c) | (TO_UINT64(lhs) << ((-c) & mask));
      context.push_operand(i64_const_t{rot});
      dbg_output << "i64.rotr\n";
   }
   void operator()(f32_abs_t) {
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_abs(op);
      dbg_print("f32.abs");
   }
   void operator()(f32_neg_t) {
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_neg(op);
      dbg_print("f32.neg");
   }
   void operator()(f32_ceil_t) {
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_ceil(op);
      dbg_print("f32.ceil");
   }
   void operator()(f32_floor_t) {
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_floor(op);
      dbg_print("f32.floor");
   }
   void operator()(f32_trunc_t) {
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_trunc(op);
      dbg_print("f32.trunc");
   }
   void operator()(f32_nearest_t) {
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_nearest(op);
      dbg_print("f32.nearest");
   }
   void operator()(f32_sqrt_t) {
      context.inc_pc();
      auto& op = TO_F32(context.peek_operand());
      op = _eosio_f32_sqrt(op);
      dbg_print("f32.sqrt");
   }
   void operator()(f32_add_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_add(lhs, TO_F32(rhs));
      dbg_print("f32.add");
   }
   void operator()(f32_sub_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_sub(lhs, TO_F32(rhs));
      dbg_print("f32.sub");
   }
   void operator()(f32_mul_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_mul(lhs, TO_F32(rhs));
      dbg_print("f32.mul");
   }
   void operator()(f32_div_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_div(lhs, TO_F32(rhs));
      dbg_print("f32.div");
   }
   void operator()(f32_min_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_min(lhs, TO_F32(rhs));
      dbg_print("f32.min");
   }
   void operator()(f32_max_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_max(lhs, TO_F32(rhs));
      dbg_print("f32.max");
   }
   void operator()(f32_copysign_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F32(context.peek_operand());
      lhs = _eosio_f32_copysign(lhs, TO_F32(rhs));
      dbg_print("f32.copysign");
   }
   void operator()(f64_abs_t) {
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_abs(op);
      dbg_print("f64.abs");
   }
   void operator()(f64_neg_t) {
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_neg(op);
      dbg_print("f64.neg");
   }
   void operator()(f64_ceil_t) {
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_ceil(op);
      dbg_print("f64.ceil");
   }
   void operator()(f64_floor_t) {
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_floor(op);
      dbg_print("f64.floor");
   }
   void operator()(f64_trunc_t) {
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_trunc(op);
      dbg_print("f64.trunc");
   }
   void operator()(f64_nearest_t) {
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_nearest(op);
      dbg_print("f64.nearest");
   }
   void operator()(f64_sqrt_t) {
      context.inc_pc();
      auto& op = TO_F64(context.peek_operand());
      op = _eosio_f64_sqrt(op);
      dbg_print("f64.sqrt");
   }
   void operator()(f64_add_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_add(lhs, TO_F64(rhs));
      dbg_print("f64.add");
   }
   void operator()(f64_sub_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_sub(lhs, TO_F64(rhs));
      dbg_print("f64.sub");
   }
   void operator()(f64_mul_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_mul(lhs, TO_F64(rhs));
      dbg_print("f64.mul");
   }
   void operator()(f64_div_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_div(lhs, TO_F64(rhs));
      dbg_print("f64.div");
   }
   void operator()(f64_min_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_min(lhs, TO_F64(rhs));
      dbg_print("f64.min");
   }
   void operator()(f64_max_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_max(lhs, TO_F64(rhs));
      dbg_print("f64.max");
   }
   void operator()(f64_copysign_t) {
      context.inc_pc();
      const auto& rhs = context.pop_operand();
      auto& lhs = TO_F64(context.peek_operand());
      lhs = _eosio_f64_copysign(lhs, TO_F64(rhs));
      dbg_print("f64.copysign");
   }
   void operator()(i32_wrap_i64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i32_const_t{static_cast<int32_t>(TO_INT64(op))}};
      dbg_print("i32.wrap_i64");
   }
   void operator()(i32_trunc_s_f32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i32_const_t{_eosio_f32_trunc_i32s(TO_F32(op))}};
      dbg_print("i32.trunc_s_f32");
   }
   void operator()(i32_trunc_u_f32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i32_const_t{_eosio_f32_trunc_i32u(TO_F32(op))}};
      dbg_print("i32.trunc_u_f32");
   }
   void operator()(i32_trunc_s_f64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i32_const_t{_eosio_f64_trunc_i32s(TO_F64(op))}};
      dbg_print("i32.trunc_s_f64");
   }
   void operator()(i32_trunc_u_f64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i32_const_t{_eosio_f64_trunc_i32u(TO_F64(op))}};
      dbg_print("i32.trunc_u_f64");
   }
   void operator()(i64_extend_s_i32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{static_cast<int64_t>(TO_INT32(op))}};
      dbg_print("i64.extend_s_i32");
   }
   void operator()(i64_extend_u_i32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{static_cast<uint64_t>(TO_UINT32(op))}};
      dbg_print("i64.extend_u_i32");
   }
   void operator()(i64_trunc_s_f32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{_eosio_f32_trunc_i64s(TO_F64(op))}};
      dbg_print("i64.trunc_s_f32");
   }
   void operator()(i64_trunc_u_f32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{_eosio_f32_trunc_i64u(TO_F64(op))}};
      dbg_print("i64.trunc_u_f32");
   }
   void operator()(i64_trunc_s_f64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{_eosio_f64_trunc_i64s(TO_F64(op))}};
      dbg_print("i64.trunc_s_f64");
   }
   void operator()(i64_trunc_u_f64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {i64_const_t{_eosio_f64_trunc_i64u(TO_F64(op))}};
      dbg_print("i64.trunc_u_f64");
   }
   void operator()(f32_convert_s_i32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f32_const_t{_eosio_i32_to_f32(TO_INT32(op))}};
      dbg_print("f32.convert_s_i32");
   }
   void operator()(f32_convert_u_i32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f32_const_t{_eosio_ui32_to_f32(TO_UINT32(op))}};
      dbg_print("f32.convert_u_i32");
   }
   void operator()(f32_convert_s_i64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f32_const_t{_eosio_i64_to_f32(TO_INT64(op))}};
      dbg_print("f32.convert_s_i64");
   }
   void operator()(f32_convert_u_i64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f32_const_t{_eosio_ui64_to_f32(TO_UINT64(op))}};
      dbg_print("f32.convert_u_i64");
   }
   void operator()(f32_demote_f64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f32_const_t{_eosio_f64_demote(TO_F64(op))}};
      dbg_print("f32.demote_f64");
   }
   void operator()(f64_convert_s_i32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f64_const_t{_eosio_i32_to_f64(TO_INT32(op))}};
      dbg_print("f64.convert_s_i32");
   }
   void operator()(f64_convert_u_i32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f64_const_t{_eosio_ui32_to_f64(TO_UINT32(op))}};
      dbg_print("f64.convert_u_i32");
   }
   void operator()(f64_convert_s_i64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f64_const_t{_eosio_i64_to_f64(TO_INT64(op))}};
      dbg_print("f64.convert_s_i64");
   }
   void operator()(f64_convert_u_i64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f64_const_t{_eosio_ui64_to_f64(TO_UINT64(op))}};
      dbg_print("f64.convert_u_i64");
   }
   void operator()(f64_promote_f32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      op = {f64_const_t{_eosio_f32_promote(TO_F32(op))}};
      dbg_print("f64.promote_f32");
   }
   void operator()(i32_reinterpret_f32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      EOS_WB_ASSERT(is_a<f32_const_t>(op), wasm_interpreter_exception, "i32.reinterpret/f32 expected f32 operand");
      op = i32_const_t{TO_UINT32(op)};
      dbg_output << "i32.reinterpret/f32\n";
   }
   void operator()(i64_reinterpret_f64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      EOS_WB_ASSERT(is_a<f64_const_t>(op), wasm_interpreter_exception, "i64.reinterpret/f64 expected f64 operand");
      op = i64_const_t{TO_UINT64(op)};
      dbg_output << "i64.reinterpret/f64\n";
   }
   void operator()(f32_reinterpret_i32_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      EOS_WB_ASSERT(is_a<i32_const_t>(op), wasm_interpreter_exception, "f32.reinterpret/i32 expected i32 operand");
      op = i32_const_t{TO_FUINT32(op)};
      dbg_output << "f32.reinterpret/i32\n";
   }
   void operator()(f64_reinterpret_i64_t) {
      context.inc_pc();
      auto& op = context.peek_operand();
      EOS_WB_ASSERT(is_a<i64_const_t>(op), wasm_interpreter_exception, "f64.reinterpret/i64 expected i64 operand");
      op = i64_const_t{TO_FUINT64(op)};
      dbg_output << "f64.reinterpret/i64\n";
   }
   void operator()(error_t) {
      context.inc_pc();
      dbg_output << "error\n";
   }

   uint32_t tab_width;
};

}} // ns eosio::wasm_backend
