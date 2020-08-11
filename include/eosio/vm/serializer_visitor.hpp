// This file is only included in the middle of serializer.hpp
// "#pragma once", includes, and type declarations are not needed.

struct serializer_visitor : base_visitor {
   serializer_visitor(growable_allocator& alloc) : _allocator(alloc) {}
   growable_allocator& _allocator;

   template <typename T>
   constexpr inline auto get_opcode() {
      return opcode::which_alternative<T>();
   }

   template <typename T>
   code_t encode_opcode_only(growable_allocator& allocator) {
      code_t result = {{ allocator, 1 }};
      result.data[result.idx++] = get_opcode<T>();
      return result;
   }

   // control ops
   code_t operator()(const unreachable_t& op) {
      return encode_opcode_only<unreachable_t>(_allocator);
   }

   code_t operator()(const nop_t& op) {
      return encode_opcode_only<nop_t>(_allocator);
   }

   code_t operator()(const block_t& op) {
      return encode_opcode_only<block_t>(_allocator);
   }

   code_t operator()(const loop_t& op) {
      return encode_opcode_only<loop_t>(_allocator);
   }

   code_t operator()(const if_t& op) {
      return encode_opcode_only<if_t>(_allocator);
   }

   code_t operator()(const else_t& op) {
      return encode_opcode_only<else_t>(_allocator);
   }

   code_t operator()(const end_t& op) {
      return encode_opcode_only<end_t>(_allocator);
   }

   code_t operator()(const return_t& op) {
      return encode_opcode_only<return_t>(_allocator);
   }

   template <typename T>
   code_t decode_br(growable_allocator& allocator, const T& op) {
      code_t result = {{ _allocator, 1 + bytes_needed<32>() }};
      result.data[result.idx++] = get_opcode<T>();
      append( result, encode_varuint<uint32_t, 32>(_allocator, op.data) );
      return result;
   }

   code_t operator()(const br_t& op) {
      return decode_br<br_t>(_allocator, op);
   }

   code_t operator()(const br_if_t& op) {
      return decode_br<br_if_t>(_allocator, op);
   }

   // parametric instructions
   code_t operator()(const drop_t& op) {
      return encode_opcode_only<drop_t>(_allocator);
   }

   code_t operator()(const select_t& op) {
      return encode_opcode_only<select_t>(_allocator);
   }

   // variable instructions
   template <typename T>
   code_t decode_variable_access_ops(growable_allocator& allocator, const T& op) {
      code_t result = {{ _allocator, 1 + bytes_needed<32>() }};
      result.data[result.idx++] = get_opcode<T>();
      append( result, encode_varuint<uint32_t, 32>(_allocator, op.index) );
      return result;
   }

   code_t operator()(const get_local_t& op) {
      return decode_variable_access_ops<get_local_t>(_allocator, op);
   }

   code_t operator()(const set_local_t& op) {
      return decode_variable_access_ops<set_local_t>(_allocator, op);
   }

   code_t operator()(const tee_local_t& op) {
      return decode_variable_access_ops<tee_local_t>(_allocator, op);
   }

   code_t operator()(const get_global_t& op) {
      return decode_variable_access_ops<get_global_t>(_allocator, op);
   }

   code_t operator()(const set_global_t& op) {
      return decode_variable_access_ops<set_global_t>(_allocator, op);
   }

   // Memory ops
   template <typename T>
   code_t decode_memory_ops(growable_allocator& allocator, const T& op) {
      code_t result = {{ _allocator, 1 + 2 * bytes_needed<32>() }};
      result.data[result.idx++] = get_opcode<T>();
      append( result, encode_varuint<uint32_t, 32>(_allocator, op.flags_align) );
      append( result, encode_varuint<uint32_t, 32>(_allocator, op.offset) );
      return result;
   }

   code_t operator()(const i32_load_t& op) {
      return decode_memory_ops<i32_load_t>(_allocator, op);
   }

   code_t operator()(const i64_load_t& op) {
      return decode_memory_ops<i64_load_t>(_allocator, op);
   }

   code_t operator()(const f32_load_t& op) {
      return decode_memory_ops<f32_load_t>(_allocator, op);
   }

   code_t operator()(const f64_load_t& op) {
      return decode_memory_ops<f64_load_t>(_allocator, op);
   }

   code_t operator()(const i32_load8_s_t& op) {
      return decode_memory_ops<i32_load8_s_t>(_allocator, op);
   }

   code_t operator()(const i32_load8_u_t& op) {
      return decode_memory_ops<i32_load8_u_t>(_allocator, op);
   }

   code_t operator()(const i32_load16_s_t& op) {
      return decode_memory_ops<i32_load16_s_t>(_allocator, op);
   }

   code_t operator()(const i32_load16_u_t& op) {
      return decode_memory_ops<i32_load16_u_t>(_allocator, op);
   }

   code_t operator()(const i64_load8_s_t& op) {
      return decode_memory_ops<i64_load8_s_t>(_allocator, op);
   }

   code_t operator()(const i64_load8_u_t& op) {
      return decode_memory_ops<i64_load8_u_t>(_allocator, op);
   }

   code_t operator()(const i64_load16_s_t& op) {
      return decode_memory_ops<i64_load16_s_t>(_allocator, op);
   }

   code_t operator()(const i64_load16_u_t& op) {
      return decode_memory_ops<i64_load16_u_t>(_allocator, op);
   }

   code_t operator()(const i64_load32_s_t& op) {
      return decode_memory_ops<i64_load32_s_t>(_allocator, op);
   }

   code_t operator()(const i64_load32_u_t& op) {
      return decode_memory_ops<i64_load32_u_t>(_allocator, op);
   }


   code_t operator()(const i32_store_t& op) {
      return decode_memory_ops<i32_store_t>(_allocator, op);
   }

   code_t operator()(const i64_store_t& op) {
      return decode_memory_ops<i64_store_t>(_allocator, op);
   }

   code_t operator()(const f32_store_t& op) {
      return decode_memory_ops<f32_store_t>(_allocator, op);
   }

   code_t operator()(const f64_store_t& op) {
      return decode_memory_ops<f64_store_t>(_allocator, op);
   }

   code_t operator()(const i32_store8_t& op) {
      return decode_memory_ops<i32_store8_t>(_allocator, op);
   }

   code_t operator()(const i32_store16_t& op) {
      return decode_memory_ops<i32_store16_t>(_allocator, op);
   }

   code_t operator()(const i64_store8_t& op) {
      return decode_memory_ops<i64_store8_t>(_allocator, op);
   }

   code_t operator()(const i64_store16_t& op) {
      return decode_memory_ops<i64_store16_t>(_allocator, op);
   }

   code_t operator()(const i64_store32_t& op) {
      return decode_memory_ops<i64_store32_t>(_allocator, op);
   }

   code_t operator()(const current_memory_t& op) {
      code_t result = {{ _allocator, 1 + bytes_needed<32>() }};
      result.data[result.idx++] = get_opcode<current_memory_t>();
      append( result, encode_varuint<uint32_t, 32>(_allocator, 0x00) );
      return result;
   }

   code_t operator()(const grow_memory_t& op) {
      code_t result = {{ _allocator, 1 + bytes_needed<32>() }};
      result.data[result.idx++] = get_opcode<grow_memory_t>();
      append( result, encode_varuint<uint32_t, 32>(_allocator, 0x00) );
      return result;
   }

   template<typename T, typename I, size_t N>
   code_t encode_i_const(const T& op) {
      code_t result = {{ _allocator, 1 + bytes_needed<N>() }};
      result.data[result.idx++] = get_opcode<T>();
      append( result, encode_varint<I, N>(_allocator, op.data.i) );
      return result;
   }

   code_t operator()(const i32_const_t& op) {
      return encode_i_const<i32_const_t, int32_t, 32>(op);
   }

   code_t operator()(const i64_const_t& op) {
      return encode_i_const<i64_const_t, int64_t, 64>(op);
   }

   // i32 comparison
   code_t operator()(const i32_eqz_t& op) {
      return encode_opcode_only<i32_eqz_t>(_allocator);
   }

   code_t operator()(const i32_eq_t& op) {
      return encode_opcode_only<i32_eq_t>(_allocator);
   }

   code_t operator()(const i32_ne_t& op) {
      return encode_opcode_only<i32_ne_t>(_allocator);
   }

   code_t operator()(const i32_lt_s_t& op) {
      return encode_opcode_only<i32_lt_s_t>(_allocator);
   }

   code_t operator()(const i32_lt_u_t& op) {
      return encode_opcode_only<i32_lt_u_t>(_allocator);
   }

   code_t operator()(const i32_gt_s_t& op) {
      return encode_opcode_only<i32_gt_s_t>(_allocator);
   }

   code_t operator()(const i32_gt_u_t& op) {
      return encode_opcode_only<i32_gt_u_t>(_allocator);
   }

   code_t operator()(const i32_le_s_t& op) {
      return encode_opcode_only<i32_le_s_t>(_allocator);
   }

   code_t operator()(const i32_le_u_t& op) {
      return encode_opcode_only<i32_le_u_t>(_allocator);
   }

   code_t operator()(const i32_ge_s_t& op) {
      return encode_opcode_only<i32_ge_s_t>(_allocator);
   }

   code_t operator()(const i32_ge_u_t& op) {
      return encode_opcode_only<i32_ge_u_t>(_allocator);
   }

   // i64 comparison
   code_t operator()(const i64_eqz_t& op) {
      return encode_opcode_only<i64_eqz_t>(_allocator);
   }

   code_t operator()(const i64_eq_t& op) {
      return encode_opcode_only<i64_eq_t>(_allocator);
   }

   code_t operator()(const i64_ne_t& op) {
      return encode_opcode_only<i64_ne_t>(_allocator);
   }

   code_t operator()(const i64_lt_s_t& op) {
      return encode_opcode_only<i64_lt_s_t>(_allocator);
   }

   code_t operator()(const i64_lt_u_t& op) {
      return encode_opcode_only<i64_lt_u_t>(_allocator);
   }

   code_t operator()(const i64_gt_s_t& op) {
      return encode_opcode_only<i64_gt_s_t>(_allocator);
   }

   code_t operator()(const i64_gt_u_t& op) {
      return encode_opcode_only<i64_gt_u_t>(_allocator);
   }

   code_t operator()(const i64_le_s_t& op) {
      return encode_opcode_only<i64_le_s_t>(_allocator);
   }

   code_t operator()(const i64_le_u_t& op) {
      return encode_opcode_only<i64_le_u_t>(_allocator);
   }

   code_t operator()(const i64_ge_s_t& op) {
      return encode_opcode_only<i64_ge_s_t>(_allocator);
   }

   code_t operator()(const i64_ge_u_t& op) {
      return encode_opcode_only<i64_ge_u_t>(_allocator);
   }

   // f32 comparison
   code_t operator()(const f32_eq_t& op) {
      return encode_opcode_only<f32_eq_t>(_allocator);
   }

   code_t operator()(const f32_ne_t& op) {
      return encode_opcode_only<f32_ne_t>(_allocator);
   }

   code_t operator()(const f32_lt_t& op) {
      return encode_opcode_only<f32_lt_t>(_allocator);
   }

   code_t operator()(const f32_gt_t& op) {
      return encode_opcode_only<f32_gt_t>(_allocator);
   }

   code_t operator()(const f32_le_t& op) {
      return encode_opcode_only<f32_le_t>(_allocator);
   }

   code_t operator()(const f32_ge_t& op) {
      return encode_opcode_only<f32_ge_t>(_allocator);
   }

   // f64 comparison
   code_t operator()(const f64_eq_t& op) {
      return encode_opcode_only<f64_eq_t>(_allocator);
   }

   code_t operator()(const f64_ne_t& op) {
      return encode_opcode_only<f64_ne_t>(_allocator);
   }

   code_t operator()(const f64_lt_t& op) {
      return encode_opcode_only<f64_lt_t>(_allocator);
   }

   code_t operator()(const f64_gt_t& op) {
      return encode_opcode_only<f64_gt_t>(_allocator);
   }

   code_t operator()(const f64_le_t& op) {
      return encode_opcode_only<f64_le_t>(_allocator);
   }

   code_t operator()(const f64_ge_t& op) {
      return encode_opcode_only<f64_ge_t>(_allocator);
   }

   // i32 arithmetics
   code_t operator()(const i32_clz_t& op) {
      return encode_opcode_only<i32_clz_t>(_allocator);
   }

   code_t operator()(const i32_ctz_t& op) {
      return encode_opcode_only<i32_ctz_t>(_allocator);
   }

   code_t operator()(const i32_popcnt_t& op) {
      return encode_opcode_only<i32_popcnt_t>(_allocator);
   }

   code_t operator()(const i32_add_t& op) {
      return encode_opcode_only<i32_add_t>(_allocator);
   }

   code_t operator()(const i32_sub_t& op) {
      return encode_opcode_only<i32_sub_t>(_allocator);
   }

   code_t operator()(const i32_mul_t& op) {
      return encode_opcode_only<i32_mul_t>(_allocator);
   }

   code_t operator()(const i32_div_s_t& op) {
      return encode_opcode_only<i32_div_s_t>(_allocator);
   }

   code_t operator()(const i32_div_u_t& op) {
      return encode_opcode_only<i32_div_u_t>(_allocator);
   }

   code_t operator()(const i32_rem_s_t& op) {
      return encode_opcode_only<i32_rem_s_t>(_allocator);
   }

   code_t operator()(const i32_rem_u_t& op) {
      return encode_opcode_only<i32_rem_u_t>(_allocator);
   }

   code_t operator()(const i32_and_t& op) {
      return encode_opcode_only<i32_and_t>(_allocator);
   }

   code_t operator()(const i32_or_t& op) {
      return encode_opcode_only<i32_or_t>(_allocator);
   }

   code_t operator()(const i32_xor_t& op) {
      return encode_opcode_only<i32_xor_t>(_allocator);
   }

   code_t operator()(const i32_shl_t& op) {
      return encode_opcode_only<i32_shl_t>(_allocator);
   }

   code_t operator()(const i32_shr_s_t& op) {
      return encode_opcode_only<i32_shr_s_t>(_allocator);
   }

   code_t operator()(const i32_shr_u_t& op) {
      return encode_opcode_only<i32_shr_u_t>(_allocator);
   }

   code_t operator()(const i32_rotl_t& op) {
      return encode_opcode_only<i32_rotl_t>(_allocator);
   }

   code_t operator()(const i32_rotr_t& op) {
      return encode_opcode_only<i32_rotr_t>(_allocator);
   }

   // i64 arithmetics
   code_t operator()(const i64_clz_t& op) {
      return encode_opcode_only<i64_clz_t>(_allocator);
   }

   code_t operator()(const i64_ctz_t& op) {
      return encode_opcode_only<i64_ctz_t>(_allocator);
   }

   code_t operator()(const i64_popcnt_t& op) {
      return encode_opcode_only<i64_popcnt_t>(_allocator);
   }

   code_t operator()(const i64_add_t& op) {
      return encode_opcode_only<i64_add_t>(_allocator);
   }

   code_t operator()(const i64_sub_t& op) {
      return encode_opcode_only<i64_sub_t>(_allocator);
   }

   code_t operator()(const i64_mul_t& op) {
      return encode_opcode_only<i64_mul_t>(_allocator);
   }

   code_t operator()(const i64_div_s_t& op) {
      return encode_opcode_only<i64_div_s_t>(_allocator);
   }

   code_t operator()(const i64_div_u_t& op) {
      return encode_opcode_only<i64_div_u_t>(_allocator);
   }

   code_t operator()(const i64_rem_s_t& op) {
      return encode_opcode_only<i64_rem_s_t>(_allocator);
   }

   code_t operator()(const i64_rem_u_t& op) {
      return encode_opcode_only<i64_rem_u_t>(_allocator);
   }

   code_t operator()(const i64_and_t& op) {
      return encode_opcode_only<i64_and_t>(_allocator);
   }

   code_t operator()(const i64_or_t& op) {
      return encode_opcode_only<i64_or_t>(_allocator);
   }

   code_t operator()(const i64_xor_t& op) {
      return encode_opcode_only<i64_xor_t>(_allocator);
   }

   code_t operator()(const i64_shl_t& op) {
      return encode_opcode_only<i64_shl_t>(_allocator);
   }

   code_t operator()(const i64_shr_s_t& op) {
      return encode_opcode_only<i64_shr_s_t>(_allocator);
   }

   code_t operator()(const i64_shr_u_t& op) {
      return encode_opcode_only<i64_shr_u_t>(_allocator);
   }

   code_t operator()(const i64_rotl_t& op) {
      return encode_opcode_only<i64_rotl_t>(_allocator);
   }

   code_t operator()(const i64_rotr_t& op) {
      return encode_opcode_only<i64_rotr_t>(_allocator);
   }

   
   // f32 arithmetics
   code_t operator()(const f32_abs_t& op) {
      return encode_opcode_only<f32_abs_t>(_allocator);
   }

   code_t operator()(const f32_neg_t& op) {
      return encode_opcode_only<f32_neg_t>(_allocator);
   }

   code_t operator()(const f32_ceil_t& op) {
      return encode_opcode_only<f32_ceil_t>(_allocator);
   }

   code_t operator()(const f32_floor_t& op) {
      return encode_opcode_only<f32_floor_t>(_allocator);
   }

   code_t operator()(const f32_trunc_t& op) {
      return encode_opcode_only<f32_trunc_t>(_allocator);
   }

   code_t operator()(const f32_nearest_t& op) {
      return encode_opcode_only<f32_nearest_t>(_allocator);
   }

   code_t operator()(const f32_sqrt_t& op) {
      return encode_opcode_only<f32_sqrt_t>(_allocator);
   }

   code_t operator()(const f32_add_t& op) {
      return encode_opcode_only<f32_add_t>(_allocator);
   }

   code_t operator()(const f32_sub_t& op) {
      return encode_opcode_only<f32_sub_t>(_allocator);
   }

   code_t operator()(const f32_mul_t& op) {
      return encode_opcode_only<f32_mul_t>(_allocator);
   }

   code_t operator()(const f32_div_t& op) {
      return encode_opcode_only<f32_div_t>(_allocator);
   }

   code_t operator()(const f32_min_t& op) {
      return encode_opcode_only<f32_min_t>(_allocator);
   }

   code_t operator()(const f32_max_t& op) {
      return encode_opcode_only<f32_max_t>(_allocator);
   }

   code_t operator()(const f32_copysign_t& op) {
      return encode_opcode_only<f32_copysign_t>(_allocator);
   }

   // f64 arithmetics
   code_t operator()(const f64_abs_t& op) {
      return encode_opcode_only<f64_abs_t>(_allocator);
   }

   code_t operator()(const f64_neg_t& op) {
      return encode_opcode_only<f64_neg_t>(_allocator);
   }

   code_t operator()(const f64_ceil_t& op) {
      return encode_opcode_only<f64_ceil_t>(_allocator);
   }

   code_t operator()(const f64_floor_t& op) {
      return encode_opcode_only<f64_floor_t>(_allocator);
   }

   code_t operator()(const f64_trunc_t& op) {
      return encode_opcode_only<f64_trunc_t>(_allocator);
   }

   code_t operator()(const f64_nearest_t& op) {
      return encode_opcode_only<f64_nearest_t>(_allocator);
   }

   code_t operator()(const f64_sqrt_t& op) {
      return encode_opcode_only<f64_sqrt_t>(_allocator);
   }

   code_t operator()(const f64_add_t& op) {
      return encode_opcode_only<f64_add_t>(_allocator);
   }

   code_t operator()(const f64_sub_t& op) {
      return encode_opcode_only<f64_sub_t>(_allocator);
   }

   code_t operator()(const f64_mul_t& op) {
      return encode_opcode_only<f64_mul_t>(_allocator);
   }

   code_t operator()(const f64_div_t& op) {
      return encode_opcode_only<f64_div_t>(_allocator);
   }

   code_t operator()(const f64_min_t& op) {
      return encode_opcode_only<f64_min_t>(_allocator);
   }

   code_t operator()(const f64_max_t& op) {
      return encode_opcode_only<f64_max_t>(_allocator);
   }

   code_t operator()(const f64_copysign_t& op) {
      return encode_opcode_only<f64_copysign_t>(_allocator);
   }

   // i32 conversion
   code_t operator()(const i32_wrap_i64_t& op) {
      return encode_opcode_only<i32_wrap_i64_t>(_allocator);
   }

   code_t operator()(const i32_trunc_s_f32_t& op) {
      return encode_opcode_only<i32_trunc_s_f32_t>(_allocator);
   }

   code_t operator()(const i32_trunc_u_f32_t& op) {
      return encode_opcode_only<i32_trunc_u_f32_t>(_allocator);
   }

   code_t operator()(const i32_trunc_s_f64_t& op) {
      return encode_opcode_only<i32_trunc_s_f64_t>(_allocator);
   }

   code_t operator()(const i32_trunc_u_f64_t& op) {
      return encode_opcode_only<i32_trunc_u_f64_t>(_allocator);
   }

   // i64 conversion
   code_t operator()(const i64_extend_s_i32_t& op) {
      return encode_opcode_only<i64_extend_s_i32_t>(_allocator);
   }

   code_t operator()(const i64_extend_u_i32_t& op) {
      return encode_opcode_only<i64_extend_u_i32_t>(_allocator);
   }

   code_t operator()(const i64_trunc_s_f32_t& op) {
      return encode_opcode_only<i64_trunc_s_f32_t>(_allocator);
   }

   code_t operator()(const i64_trunc_u_f32_t& op) {
      return encode_opcode_only<i64_trunc_u_f32_t>(_allocator);
   }

   code_t operator()(const i64_trunc_s_f64_t& op) {
      return encode_opcode_only<i64_trunc_s_f64_t>(_allocator);
   }

   code_t operator()(const i64_trunc_u_f64_t& op) {
      return encode_opcode_only<i64_trunc_u_f64_t>(_allocator);
   }

   // f32 conversion
   code_t operator()(const f32_convert_s_i32_t& op) {
      return encode_opcode_only<f32_convert_s_i32_t>(_allocator);
   }

   code_t operator()(const f32_convert_u_i32_t& op) {
      return encode_opcode_only<f32_convert_u_i32_t>(_allocator);
   }

   code_t operator()(const f32_convert_s_i64_t& op) {
      return encode_opcode_only<f32_convert_s_i64_t>(_allocator);
   }

   code_t operator()(const f32_convert_u_i64_t& op) {
      return encode_opcode_only<f32_convert_u_i64_t>(_allocator);
   }

   code_t operator()(const f32_demote_f64_t& op) {
      return encode_opcode_only<f32_demote_f64_t>(_allocator);
   }
   
   // f64 conversion
   code_t operator()(const f64_convert_s_i32_t& op) {
      return encode_opcode_only<f64_convert_s_i32_t>(_allocator);
   }

   code_t operator()(const f64_convert_u_i32_t& op) {
      return encode_opcode_only<f64_convert_u_i32_t>(_allocator);
   }

   code_t operator()(const f64_convert_s_i64_t& op) {
      return encode_opcode_only<f64_convert_s_i64_t>(_allocator);
   }

   code_t operator()(const f64_convert_u_i64_t& op) {
      return encode_opcode_only<f64_convert_u_i64_t>(_allocator);
   }

   code_t operator()(const f64_promote_f32_t& op) {
      return encode_opcode_only<f64_promote_f32_t>(_allocator);
   }
   

   // reinterpret conversion
   code_t operator()(const i32_reinterpret_f32_t& op) {
      return encode_opcode_only<i32_reinterpret_f32_t>(_allocator);
   }

   code_t operator()(const i64_reinterpret_f64_t& op) {
      return encode_opcode_only<i64_reinterpret_f64_t>(_allocator);
   }

   code_t operator()(const f32_reinterpret_i32_t& op) {
      return encode_opcode_only<f32_reinterpret_i32_t>(_allocator);
   }

   code_t operator()(const f64_reinterpret_i64_t& op) {
      return encode_opcode_only<f64_reinterpret_i64_t>(_allocator);
   }

   // TBD f32_const

   // Catch for the rest
   template <typename T>
   inline code_t operator()(T) {
      code_t result = {{ _allocator, 1 }};
      std::cout << "unimplemented yet opcode!\n";
      return result;
   }
};
