#pragma once

#include <eosio/wasm_backend/utils.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/wasm_stack.hpp>
#include <eosio/wasm_backend/execution_context.hpp>
#include <iostream>
#include <variant>
#include <sstream>

#define dbg_print print
//#define dbg_print

namespace eosio { namespace wasm_backend {

   struct test {
      void hello() { std::cout << "HELLO\n"; }
   };

struct interpret_visitor {
   interpret_visitor(execution_context<interpret_visitor>& ec) : context(ec) {}
   execution_context<interpret_visitor>& context;
   std::stringstream  dbg_output;

   struct stack_elem_visitor {
      std::stringstream& dbg_output;
      stack_elem_visitor(std::stringstream& ss) : dbg_output(ss) {}
      void operator()(const uint32_t& pc) {
         dbg_output << "PC : " << pc << "\n";
      }
      void operator()(const i32_const_t& val) {
      }
      void operator()(const i64_const_t& val) {
      }
      void operator()(const f32_const_t& val) {
      }
      void operator()(const f64_const_t& val) {
      }
      void operator()(const block_t& ctrl) {
         dbg_output << "block : " << std::to_string(ctrl.data) << "\n";
      }
      void operator()(const loop_t& ctrl) {
         dbg_output << "loop : " << std::to_string(ctrl.data) << "\n";
      }
      void operator()(const if__t& ctrl) {
         dbg_output << "if : " << std::to_string(ctrl.data) << "\n";
      }
   } _elem_visitor{dbg_output};

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
   void operator()(end_t) {
      context.inc_pc();
      stack_elem c = context.pop_label();
      std::visit(_elem_visitor, c);
      dbg_output << "end {" << context.get_pc() << "}\n";
   }
   void operator()(return__t) {
      context.inc_pc();
      dbg_output << "return {" << context.get_pc() << "}\n";
   }
   void operator()(block_t bt) {
      context.push_label(bt);
      context.inc_pc();
      dbg_print("block : "+std::to_string(bt.data)+" "+std::to_string(bt.pc));
      dbg_output << "block {" << context.get_pc() << "} " << bt.data << " " << bt.pc << "\n";
   }
   void operator()(loop_t lt) {
      context.inc_pc();
      context.push_label(lt);
      dbg_output << "loop {" << context.get_pc() << "} " << lt.data << " " << lt.pc << "\n";
   }
   void operator()(if__t it) {
      context.inc_pc();
      context.push_label(it);
      dbg_output << "if {" << context.get_pc() << "} " << it.data << " " << it.pc << "\n";
   }
   void operator()(else__t et) {
      context.set_pc(et.pc);
      dbg_output << "else {" << context.get_pc() << "} " << et.data << " " << et.pc << "\n";
   }
   void operator()(br_t b) {
      context.jump(b.data);
      dbg_output << "br {" << context.get_pc() << "} " << b.data << "\n";
   }
   void operator()(br_if_t b) {
      const auto& val = context.pop_operand();
      if (context.is_true(val))
         context.jump(b.data);
      dbg_output << "br.if {" << context.get_pc() << "} " << b.data << "\n";
   }
   void operator()(br_table_t b) {
      const auto& val = context.pop_operand();
      EOS_WB_ASSERT(std::holds_alternative<i32_const_t>(val), wasm_interpreter_exception, "br_table expected i32");
      const auto& i32_v = std::get<i32_const_t>(val);
      if (i32_v.data < b.target_table.size())
         context.jump(b.target_table[i32_v.data]);
      else
         context.jump(b.target_table[b.default_target]);
      dbg_output << "br.table {" << context.get_pc() << "} " << b.default_target << "\n\t";
      for (int i=0; i < b.target_table.size(); i++) {
         dbg_output << ", " << b.target_table[i];
      }
      dbg_output << "\n";
   }
   void operator()(call_t b) {
      const uint32_t& funcs_size = context.get_module().get_functions_total();
      EOS_WB_ASSERT(b.index < funcs_size, wasm_interpreter_exception, "call index out of bounds");
      const auto& ftype = context.get_module().get_function_type(b.index);
      /*
      auto ret_val = context.invoke(b.index, ftype);
      if (ftype.return_count > 0) {
         EOS_WB_ASSERT(ftype.return_count <= 1, wasm_interpreter_exception, "mvp only supports single value returns");
         context.push_operand(ret_val);
      }
      */
      context.inc_pc();
      //test t;
      //registered_function<test, &test::hello, decltype("hello"_hfn)> rf;
      //std::invoke(rf.member_pointer, t);
      dbg_output << "call {" << context.get_pc() << "} " << b.index << "\n";
   }
   void operator()(call_indirect_t b) {
      context.inc_pc();
      dbg_output << "call_indirect " << b.index << " " << context.get_module().tables[b.index].element_type << "\n";
   }
   void operator()(drop_t b) {
      context.pop_operand();
      context.inc_pc();
      dbg_print("drop");
   }
   void operator()(select_t b) {
      const auto& c = context.pop_operand();
      EOS_WB_ASSERT(std::holds_alternative<i32_const_t>(c), wasm_interpreter_exception, "select expected i32 on stack");
      const auto& v2 = context.pop_operand();
      if (std::get<i32_const_t>(c).data == 0) {
         context.pop_operand();
         context.push_operand(v2);
      }
      context.inc_pc();
      dbg_print("select");
   }
   void operator()(get_local_t b) {
      context.inc_pc();
      context.push_operand(context.get_operand(b.index));
      dbg_print("get_local : "+std::to_string(b.index));
   }
   void operator()(set_local_t b) {
      context.inc_pc();
      context.set_operand(b.index, context.pop_operand());
      dbg_print("set_local : "+std::to_string(b.index));
   }
   void operator()(tee_local_t b) {
      context.inc_pc();
      const auto& op = context.pop_operand();
      context.set_operand(b.index, op);
      context.push_operand(op);
      dbg_print("tee_local : "+std::to_string(b.index));
   }
   void operator()(get_global_t b) {
      context.inc_pc();
      const auto& gl = context.get_global(b.index);
      context.push_operand(gl);
      dbg_output << "get_global " << b.index;
      std::visit(overloaded {
            [&](const i32_const_t& i){
               dbg_output << " " << *(int32_t*)&i.data << "\n"; 
            }, [&](const i64_const_t& i){
               dbg_output << " " << *(int64_t*)&i.data << "\n"; 
            }, [&](const f32_const_t& f){
               dbg_output << " " << *(float*)&f.data << "\n"; 
            }, [&](const f64_const_t& f){
               dbg_output << " " << *(double*)&f.data << "\n"; 
            }, [](auto) {
               throw wasm_interpreter_exception{"invalid global type"};
            }
         }, gl);
   }
   void operator()(set_global_t b) {
      context.inc_pc();
      const auto& op = context.pop_operand();
      context.set_global(b.index, op);
      dbg_output << "set_global " << b.index << "\n";
   }
   void operator()(i32_load_t b) {
      context.inc_pc();
      dbg_output << "i32.load " << " align " << b.flags_align << " offset " << b.offset << "\n";
   }
   void operator()(i32_load8_s_t b) {
      context.inc_pc();
      dbg_print("i32.load8_s : "); //+std::to_string(b.index);
   }
   void operator()(i32_load16_s_t b) {
      context.inc_pc();
      dbg_print("i32.load16_s : "); //+std::to_string(b.index);
   }
   void operator()(i32_load8_u_t b) {
      context.inc_pc();
      dbg_print("i32.load8_u : "); //+std::to_string(b.index);
   }
   void operator()(i32_load16_u_t b) {
      context.inc_pc();
      dbg_print("i32.load16_u : "); //+std::to_string(b.index);
   }
   void operator()(i64_load_t b) {
      context.inc_pc();
      dbg_output << "i64.load " << " align " << b.flags_align << " offset " << b.offset << "\n";
   }
   void operator()(i64_load8_s_t b) {
      context.inc_pc();
      dbg_print("i64.load8_s : "); //+std::to_string(b.index);
   }
   void operator()(i64_load16_s_t b) {
      context.inc_pc();
      dbg_print("i64.load16_s : "); //+std::to_string(b.index);
   }
   void operator()(i64_load32_s_t b) {
      context.inc_pc();
      dbg_print("i64.load32_s : "); //+std::to_string(b.index);
   }
   void operator()(i64_load8_u_t b) {
      context.inc_pc();
      dbg_print("i64.load8_u : "); //+std::to_string(b.index);
   }
   void operator()(i64_load16_u_t b) {
      context.inc_pc();
      dbg_print("i64.load16_u : "); //+std::to_string(b.index);
   }
   void operator()(i64_load32_u_t b) {
      context.inc_pc();
      dbg_print("i64.load32_u : "); //+std::to_string(b.index);
   }
   void operator()(f32_load_t b) {
      context.inc_pc();
      dbg_print("f32.load : "); //+std::to_string(b.index);
   }
   void operator()(f64_load_t b) {
      context.inc_pc();
      dbg_print("f64.load : "); //+std::to_string(b.index);
   }
   void operator()(i32_store_t b) {
      context.inc_pc();
      dbg_print("i32.store : "); //+std::to_string(b.index);
   }
   void operator()(i32_store8_t b) {
      context.inc_pc();
      dbg_print("i32.store8 : "); //+std::to_string(b.index);
   }
   void operator()(i32_store16_t b) {
      context.inc_pc();
      dbg_print("i32.store16 : "); //+std::to_string(b.index);
   }
   void operator()(i64_store_t b) {
      context.inc_pc();
      dbg_print("i64.store : "); //+std::to_string(b.index);
   }
   void operator()(i64_store8_t b) {
      context.inc_pc();
      dbg_print("i64.store8 : "); //+std::to_string(b.index);
   }
   void operator()(i64_store16_t b) {
      context.inc_pc();
      dbg_print("i64.store16 : "); //+std::to_string(b.index);
   }
   void operator()(i64_store32_t b) {
      context.inc_pc();
      dbg_print("i64.store32 : "); //+std::to_string(b.index);
   }
   void operator()(f32_store_t b) {
      context.inc_pc();
      dbg_print("f32.store : "); //+std::to_string(b.index);
   }
   void operator()(f64_store_t b) {
      context.inc_pc();
      dbg_print("f64.store : "); //+std::to_string(b.index);
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
      dbg_print("i32.const : "+std::to_string(*(int32_t*)&b.data));
   }
   void operator()(i64_const_t b) {
      context.inc_pc();
      context.push_operand(b);
      dbg_print("i64.const : "+std::to_string(*(int64_t*)&b.data));
   }
   void operator()(f32_const_t b) {
      context.inc_pc();
      context.push_operand(b);
      dbg_print("f32.const : "+std::to_string(*((float*)&b.data)));
   }
   void operator()(f64_const_t b) {
      context.inc_pc();
      context.push_operand(b);
      dbg_print("f64.const : "+std::to_string(*((double*)&b.data)));
   }
   void operator()(i32_eqz_t b) {
      context.inc_pc();
      dbg_print("i32.eqz");
   }
   void operator()(i32_eq_t b) {
      context.inc_pc();
      dbg_print("i32.eq");
   }
   void operator()(i32_ne_t b) {
      context.inc_pc();
      dbg_print("i32.ne");
   }
   void operator()(i32_lt_s_t b) {
      context.inc_pc();
      dbg_print("i32.lt_s");
   }
   void operator()(i32_lt_u_t b) {
      context.inc_pc();
      dbg_print("i32.lt_u");
   }
   void operator()(i32_le_s_t b) {
      context.inc_pc();
      dbg_print("i32.le_s");
   }
   void operator()(i32_le_u_t b) {
      context.inc_pc();
      dbg_print("i32.le_u");
   }
   void operator()(i32_gt_s_t b) {
      context.inc_pc();
      dbg_print("i32.gt_s");
   }
   void operator()(i32_gt_u_t b) {
      context.inc_pc();
      dbg_print("i32.gt_u");
   }
   void operator()(i32_ge_s_t b) {
      context.inc_pc();
      dbg_print("i32.ge_s");
   }
   void operator()(i32_ge_u_t b) {
      context.inc_pc();
      dbg_print("i32.ge_u");
   }
   void operator()(i64_eqz_t b) {
      context.inc_pc();
      dbg_print("i64.eqz");
   }
   void operator()(i64_eq_t b) {
      context.inc_pc();
      dbg_print("i64.eq");
   }
   void operator()(i64_ne_t b) {
      context.inc_pc();
      dbg_print("i64.ne");
   }
   void operator()(i64_lt_s_t b) {
      context.inc_pc();
      dbg_print("i64.lt_s");
   }
   void operator()(i64_lt_u_t b) {
      context.inc_pc();
      dbg_print("i64.lt_u");
   }
   void operator()(i64_le_s_t b) {
      context.inc_pc();
      dbg_print("i64.le_s");
   }
   void operator()(i64_le_u_t b) {
      context.inc_pc();
      dbg_print("i64.le_u");
   }
   void operator()(i64_gt_s_t b) {
      context.inc_pc();
      dbg_print("i64.gt_s");
   }
   void operator()(i64_gt_u_t b) {
      context.inc_pc();
      dbg_print("i64.gt_u");
   }
   void operator()(i64_ge_s_t b) {
      context.inc_pc();
      dbg_print("i64.ge_s");
   }
   void operator()(i64_ge_u_t b) {
      context.inc_pc();
      dbg_print("i64.ge_u");
   }
   void operator()(f32_eq_t b) {
      context.inc_pc();
      dbg_print("f32.eq");
   }
   void operator()(f32_ne_t b) {
      context.inc_pc();
      dbg_print("f32.ne");
   }
   void operator()(f32_lt_t b) {
      context.inc_pc();
      dbg_print("f32.lt");
   }
   void operator()(f32_gt_t b) {
      context.inc_pc();
      dbg_print("f32.gt");
   }
   void operator()(f32_le_t b) {
      context.inc_pc();
      dbg_print("f32.le");
   }
   void operator()(f32_ge_t b) {
      context.inc_pc();
      dbg_print("f32.ge");
   }
   void operator()(f64_eq_t b) {
      context.inc_pc();
      dbg_print("f64.eq");
   }
   void operator()(f64_ne_t b) {
      context.inc_pc();
      dbg_print("f64.ne");
   }
   void operator()(f64_lt_t b) {
      context.inc_pc();
      dbg_print("f64.lt");
   }
   void operator()(f64_gt_t b) {
      context.inc_pc();
      dbg_print("f64.gt");
   }
   void operator()(f64_le_t b) {
      context.inc_pc();
      dbg_print("f64.le");
   }
   void operator()(f64_ge_t b) {
      context.inc_pc();
      dbg_print("f64.ge");
   }
   void operator()(i32_clz_t) {
      context.inc_pc();
      dbg_print("i32.clz");
   }
   void operator()(i32_ctz_t) {
      context.inc_pc();
      dbg_print("i32.ctz");
   }
   void operator()(i32_popcnt_t) {
      context.inc_pc();
      dbg_print("i32.popcnt");
   }
   void operator()(i32_add_t) {
      context.inc_pc();
      dbg_print("i32.add");
   }
   void operator()(i32_sub_t) {
      context.inc_pc();
      dbg_print("i32.sub");
   }
   void operator()(i32_mul_t) {
      context.inc_pc();
      dbg_print("i32.mul");
   }
   void operator()(i32_div_s_t) {
      context.inc_pc();
      dbg_print("i32.div_s");
   }
   void operator()(i32_div_u_t) {
      context.inc_pc();
      dbg_print("i32.div_u");
   }
   void operator()(i32_rem_s_t) {
      context.inc_pc();
      dbg_print("i32.rem_s");
   }
   void operator()(i32_rem_u_t) {
      context.inc_pc();
      dbg_print("i32.rem_u");
   }
   void operator()(i32_and_t) {
      context.inc_pc();
      dbg_print("i32.and");
   }
   void operator()(i32_or_t) {
      context.inc_pc();
      dbg_print("i32.or");
   }
   void operator()(i32_xor_t) {
      context.inc_pc();
      dbg_print("i32.xor");
   }
   void operator()(i32_shl_t) {
      context.inc_pc();
      dbg_print("i32.shl");
   }
   void operator()(i32_shr_s_t) {
      context.inc_pc();
      dbg_print("i32.shr_s");
   }
   void operator()(i32_shr_u_t) {
      context.inc_pc();
      dbg_print("i32.shr_u");
   }
   void operator()(i32_rotl_t) {
      context.inc_pc();
      dbg_print("i32.rotl");
   }
   void operator()(i32_rotr_t) {
      context.inc_pc();
      dbg_print("i32.rotr");
   }
   void operator()(i64_clz_t) {
      context.inc_pc();
      dbg_print("i64.clz");
   }
   void operator()(i64_ctz_t) {
      context.inc_pc();
      dbg_print("i64.ctz");
   }
   void operator()(i64_popcnt_t) {
      context.inc_pc();
      dbg_print("i64.popcnt");
   }
   void operator()(i64_add_t) {
      context.inc_pc();
      dbg_print("i64.add");
   }
   void operator()(i64_sub_t) {
      context.inc_pc();
      dbg_print("i64.sub");
   }
   void operator()(i64_mul_t) {
      context.inc_pc();
      dbg_print("i64.mul");
   }
   void operator()(i64_div_s_t) {
      context.inc_pc();
      dbg_print("i64.div_s");
   }
   void operator()(i64_div_u_t) {
      context.inc_pc();
      dbg_print("i64.div_u");
   }
   void operator()(i64_rem_s_t) {
      context.inc_pc();
      dbg_print("i64.rem_s");
   }
   void operator()(i64_rem_u_t) {
      context.inc_pc();
      dbg_print("i64.rem_u");
   }
   void operator()(i64_and_t) {
      context.inc_pc();
      dbg_print("i64.and");
   }
   void operator()(i64_or_t) {
      context.inc_pc();
      dbg_print("i64.or");
   }
   void operator()(i64_xor_t) {
      context.inc_pc();
      dbg_print("i64.xor");
   }
   void operator()(i64_shl_t) {
      context.inc_pc();
      dbg_print("i64.shl");
   }
   void operator()(i64_shr_s_t) {
      context.inc_pc();
      dbg_print("i64.shr_s");
   }
   void operator()(i64_shr_u_t) {
      context.inc_pc();
      dbg_print("i64.shr_u");
   }
   void operator()(i64_rotl_t) {
      context.inc_pc();
      dbg_print("i64.rotl");
   }
   void operator()(i64_rotr_t) {
      context.inc_pc();
      dbg_print("i64.rotr");
   }
   void operator()(f32_abs_t) {
      context.inc_pc();
      dbg_print("f32.abs");
   }
   void operator()(f32_neg_t) {
      context.inc_pc();
      dbg_print("f32.neg");
   }
   void operator()(f32_ceil_t) {
      context.inc_pc();
      dbg_print("f32.ceil");
   }
   void operator()(f32_floor_t) {
      context.inc_pc();
      dbg_print("f32.floor");
   }
   void operator()(f32_trunc_t) {
      context.inc_pc();
      dbg_print("f32.trunc");
   }
   void operator()(f32_nearest_t) {
      context.inc_pc();
      dbg_print("f32.nearest");
   }
   void operator()(f32_sqrt_t) {
      context.inc_pc();
      dbg_print("f32.sqrt");
   }
   void operator()(f32_add_t) {
      context.inc_pc();
      dbg_print("f32.add");
   }
   void operator()(f32_sub_t) {
      context.inc_pc();
      dbg_print("f32.sub");
   }
   void operator()(f32_mul_t) {
      context.inc_pc();
      dbg_print("f32.mul");
   }
   void operator()(f32_div_t) {
      context.inc_pc();
      dbg_print("f32.div");
   }
   void operator()(f32_min_t) {
      context.inc_pc();
      dbg_print("f32.min");
   }
   void operator()(f32_max_t) {
      context.inc_pc();
      dbg_print("f32.max");
   }
   void operator()(f32_copysign_t) {
      context.inc_pc();
      dbg_print("f32.copysign");
   }
   void operator()(f64_abs_t) {
      context.inc_pc();
      dbg_print("f64.abs");
   }
   void operator()(f64_neg_t) {
      context.inc_pc();
      dbg_print("f64.neg");
   }
   void operator()(f64_ceil_t) {
      context.inc_pc();
      dbg_print("f64.ceil");
   }
   void operator()(f64_floor_t) {
      context.inc_pc();
      dbg_print("f64.floor");
   }
   void operator()(f64_trunc_t) {
      context.inc_pc();
      dbg_print("f64.trunc");
   }
   void operator()(f64_nearest_t) {
      context.inc_pc();
      dbg_print("f64.nearest");
   }
   void operator()(f64_sqrt_t) {
      context.inc_pc();
      dbg_print("f64.sqrt");
   }
   void operator()(f64_add_t) {
      context.inc_pc();
      dbg_print("f64.add");
   }
   void operator()(f64_sub_t) {
      context.inc_pc();
      dbg_print("f64.sub");
   }
   void operator()(f64_mul_t) {
      context.inc_pc();
      dbg_print("f64.mul");
   }
   void operator()(f64_div_t) {
      context.inc_pc();
      dbg_print("f64.div");
   }
   void operator()(f64_min_t) {
      context.inc_pc();
      dbg_print("f64.min");
   }
   void operator()(f64_max_t) {
      context.inc_pc();
      dbg_print("f64.max");
   }
   void operator()(f64_copysign_t) {
      context.inc_pc();
      dbg_print("f64.copysign");
   }
   void operator()(i32_wrap_i64_t) {
      context.inc_pc();
      dbg_print("i32.wrap_i64");
   }
   void operator()(i32_trunc_s_f32_t) {
      context.inc_pc();
      dbg_print("i32.trunc_s_f32");
   }
   void operator()(i32_trunc_u_f32_t) {
      context.inc_pc();
      dbg_print("i32.trunc_u_f32");
   }
   void operator()(i32_trunc_s_f64_t) {
      context.inc_pc();
      dbg_print("i32.trunc_s_f64");
   }
   void operator()(i32_trunc_u_f64_t) {
      context.inc_pc();
      dbg_print("i32.trunc_u_f64");
   }
   void operator()(i64_extend_s_i32_t) {
      context.inc_pc();
      dbg_print("i64.extend_s_i32");
   }
   void operator()(i64_extend_u_i32_t) {
      context.inc_pc();
      dbg_print("i64.extend_u_i32");
   }
   void operator()(i64_trunc_s_f32_t) {
      context.inc_pc();
      dbg_print("i64.trunc_s_f32");
   }
   void operator()(i64_trunc_u_f32_t) {
      context.inc_pc();
      dbg_print("i64.trunc_u_f32");
   }
   void operator()(i64_trunc_s_f64_t) {
      context.inc_pc();
      dbg_print("i64.trunc_s_f64");
   }
   void operator()(i64_trunc_u_f64_t) {
      context.inc_pc();
      dbg_print("i64.trunc_u_f64");
   }
   void operator()(f32_convert_s_i32_t) {
      context.inc_pc();
      dbg_print("f32.convert_s_i32");
   }
   void operator()(f32_convert_u_i32_t) {
      context.inc_pc();
      dbg_print("f32.convert_u_i32");
   }
   void operator()(f32_convert_s_i64_t) {
      context.inc_pc();
      dbg_print("f32.convert_s_i64");
   }
   void operator()(f32_convert_u_i64_t) {
      context.inc_pc();
      dbg_print("f32.convert_u_i64");
   }
   void operator()(f32_demote_f64_t) {
      context.inc_pc();
      dbg_print("f32.demote_f64");
   }
   void operator()(f64_convert_s_i32_t) {
      context.inc_pc();
      dbg_print("f64.convert_s_i32");
   }
   void operator()(f64_convert_u_i32_t) {
      context.inc_pc();
      dbg_print("f64.convert_u_i32");
   }
   void operator()(f64_convert_s_i64_t) {
      context.inc_pc();
      dbg_print("f64.convert_s_i64");
   }
   void operator()(f64_convert_u_i64_t) {
      context.inc_pc();
      dbg_print("f64.convert_u_i64");
   }
   void operator()(f64_promote_f32_t) {
      context.inc_pc();
      dbg_print("f64.promote_f32");
   }
   void operator()(i32_reinterpret_f32_t) {
      context.inc_pc();
      dbg_print("i32.reinterpret_f32");
   }
   void operator()(i64_reinterpret_f64_t) {
      context.inc_pc();
      dbg_print("i64.reinterpret_f64");
   }
   void operator()(f32_reinterpret_i32_t) {
      context.inc_pc();
      dbg_print("f32.reinterpret_i32");
   }
   void operator()(f64_reinterpret_i64_t) {
      context.inc_pc();
      dbg_print("f64.reinterpret_i64");
   }
   void operator()(error_t) {
      context.inc_pc();
      dbg_print("error");
   }

   uint32_t tab_width;
};

}} // ns eosio::wasm_backend
