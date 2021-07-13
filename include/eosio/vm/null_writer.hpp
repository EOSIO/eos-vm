#pragma once

namespace eosio { namespace vm {

class null_writer {
 public:
   struct branch_t {};
   struct label_t {};
   explicit null_writer(growable_allocator& alloc, std::size_t source_bytes, module& mod) {}
   void emit_unreachable() {}
   void emit_nop() {}
   label_t emit_end() { return {}; }
   branch_t emit_return(uint32_t /*depth_change*/) { return {}; }
   void emit_block() {}
   label_t emit_loop() { return {}; }
   branch_t emit_if() { return {}; }
   branch_t emit_else(branch_t /*if_loc*/) { return {}; }
   branch_t emit_br(uint32_t /*depth_change*/) { return {}; }
   branch_t emit_br_if(uint32_t /*depth_change*/) { return {}; }
   struct br_table_parser {
      branch_t emit_case(uint32_t /*depth_change*/) { return {}; }
      branch_t emit_default(uint32_t /*depth_change*/) { return {}; }
   };
   br_table_parser emit_br_table(uint32_t /*table_size*/) { return {}; }
   void emit_call(const func_type& /*ft*/, uint32_t /*funcnum*/) {}
   void emit_call_indirect(const func_type& /*ft*/, uint32_t /*functypeidx*/) {}

   void emit_drop() {}
   void emit_select() {}
   void emit_get_local(uint32_t /*localidx*/) {}
   void emit_set_local(uint32_t /*localidx*/) {}
   void emit_tee_local(uint32_t /*localidx*/) {}
   void emit_get_global(uint32_t /*localidx*/) {}
   void emit_set_global(uint32_t /*localidx*/) {}

   void emit_i32_load(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_load(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_f32_load(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_f64_load(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i32_load8_s(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i32_load16_s(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i32_load8_u(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i32_load16_u(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_load8_s(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_load16_s(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_load32_s(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_load8_u(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_load16_u(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_load32_u(uint32_t /*offset*/, uint32_t /*alignment*/) {}

   void emit_i32_store(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_store(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_f32_store(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_f64_store(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i32_store8(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i32_store16(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_store8(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_store16(uint32_t /*offset*/, uint32_t /*alignment*/) {}
   void emit_i64_store32(uint32_t /*offset*/, uint32_t /*alignment*/) {}

   void emit_current_memory() {}
   void emit_grow_memory() {}
   void emit_i32_const(uint32_t /*value*/) {}
   void emit_i64_const(uint64_t /*value*/) {}
   void emit_f32_const(float /*value*/) {}
   void emit_f64_const(double /*value*/) {}

   void emit_i32_eqz() {}
   void emit_i32_eq() {}
   void emit_i32_ne() {}
   void emit_i32_lt_s() {}
   void emit_i32_lt_u() {}
   void emit_i32_gt_s() {}
   void emit_i32_gt_u() {}
   void emit_i32_le_s() {}
   void emit_i32_le_u() {}
   void emit_i32_ge_s() {}
   void emit_i32_ge_u() {}

   void emit_i64_eqz() {}
   void emit_i64_eq() {}
   void emit_i64_ne() {}
   void emit_i64_lt_s() {}
   void emit_i64_lt_u() {}
   void emit_i64_gt_s() {}
   void emit_i64_gt_u() {}
   void emit_i64_le_s() {}
   void emit_i64_le_u() {}
   void emit_i64_ge_s() {}
   void emit_i64_ge_u() {}

   void emit_f32_eq() {}
   void emit_f32_ne() {}
   void emit_f32_lt() {}
   void emit_f32_gt() {}
   void emit_f32_le() {}
   void emit_f32_ge() {}

   void emit_f64_eq() {}
   void emit_f64_ne() {}
   void emit_f64_lt() {}
   void emit_f64_gt() {}
   void emit_f64_le() {}
   void emit_f64_ge() {}

   void emit_i32_clz() {}
   void emit_i32_ctz() {}
   void emit_i32_popcnt() {}
   void emit_i32_add() {}
   void emit_i32_sub() {}
   void emit_i32_mul() {}
   void emit_i32_div_s() {}
   void emit_i32_div_u() {}
   void emit_i32_rem_s() {}
   void emit_i32_rem_u() {}
   void emit_i32_and() {}
   void emit_i32_or() {}
   void emit_i32_xor() {}
   void emit_i32_shl() {}
   void emit_i32_shr_s() {}
   void emit_i32_shr_u() {}
   void emit_i32_rotl() {}
   void emit_i32_rotr() {}

   void emit_i64_clz() {}
   void emit_i64_ctz() {}
   void emit_i64_popcnt() {}
   void emit_i64_add() {}
   void emit_i64_sub() {}
   void emit_i64_mul() {}
   void emit_i64_div_s() {}
   void emit_i64_div_u() {}
   void emit_i64_rem_s() {}
   void emit_i64_rem_u() {}
   void emit_i64_and() {}
   void emit_i64_or() {}
   void emit_i64_xor() {}
   void emit_i64_shl() {}
   void emit_i64_shr_s() {}
   void emit_i64_shr_u() {}
   void emit_i64_rotl() {}
   void emit_i64_rotr() {}

   void emit_f32_abs() {}
   void emit_f32_neg() {}
   void emit_f32_ceil() {}
   void emit_f32_floor() {}
   void emit_f32_trunc() {}
   void emit_f32_nearest() {}
   void emit_f32_sqrt() {}
   void emit_f32_add() {}
   void emit_f32_sub() {}
   void emit_f32_mul() {}
   void emit_f32_div() {}
   void emit_f32_min() {}
   void emit_f32_max() {}
   void emit_f32_copysign() {}

   void emit_f64_abs() {}
   void emit_f64_neg() {}
   void emit_f64_ceil() {}
   void emit_f64_floor() {}
   void emit_f64_trunc() {}
   void emit_f64_nearest() {}
   void emit_f64_sqrt() {}
   void emit_f64_add() {}
   void emit_f64_sub() {}
   void emit_f64_mul() {}
   void emit_f64_div() {}
   void emit_f64_min() {}
   void emit_f64_max() {}
   void emit_f64_copysign() {}

   void emit_i32_wrap_i64() {}
   void emit_i32_trunc_s_f32() {}
   void emit_i32_trunc_u_f32() {}
   void emit_i32_trunc_s_f64() {}
   void emit_i32_trunc_u_f64() {}
   void emit_i64_extend_s_i32() {}
   void emit_i64_extend_u_i32() {}
   void emit_i64_trunc_s_f32() {}
   void emit_i64_trunc_u_f32() {}
   void emit_i64_trunc_s_f64() {}
   void emit_i64_trunc_u_f64() {}
   void emit_f32_convert_s_i32() {}
   void emit_f32_convert_u_i32() {}
   void emit_f32_convert_s_i64() {}
   void emit_f32_convert_u_i64() {}
   void emit_f32_demote_f64() {}
   void emit_f64_convert_s_i32() {}
   void emit_f64_convert_u_i32() {}
   void emit_f64_convert_s_i64() {}
   void emit_f64_convert_u_i64() {}
   void emit_f64_promote_f32() {}
   void emit_i32_reinterpret_f32() {}
   void emit_i64_reinterpret_f64() {}
   void emit_f32_reinterpret_i32() {}
   void emit_f64_reinterpret_i64() {}

   void fix_branch(branch_t, label_t) {}
   void emit_prologue(const func_type& /*ft*/, const guarded_vector<local_entry>& /*locals*/, uint32_t /*idx*/) {}
   void emit_epilogue(const func_type& /*ft*/, const guarded_vector<local_entry>& /*locals*/, uint32_t /*idx*/) {}
   void finalize(function_body& /*body*/) {}

   const void* get_addr() const { return nullptr; }
   const void* get_base_addr() const { return nullptr; }
};

}}
