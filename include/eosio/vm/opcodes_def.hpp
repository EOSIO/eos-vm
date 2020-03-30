#pragma once

#include <cstdint>
#include <string>

/* clang-format off */
#define EOS_VM_CONTROL_FLOW_OPS(opcode_macro) \
   opcode_macro(unreachable, 0x00, "!")       \
   opcode_macro(nop, 0x01, "!")               \
   opcode_macro(block, 0x02, "!")             \
   opcode_macro(loop, 0x03, "!")              \
   opcode_macro(if_, 0x04, "if")              \
   opcode_macro(else_, 0x05, "else")          \
   opcode_macro(padding_cf_0, 0x06, "")       \
   opcode_macro(padding_cf_1, 0x07, "")       \
   opcode_macro(padding_cf_2, 0x08, "")       \
   opcode_macro(padding_cf_3, 0x09, "")       \
   opcode_macro(padding_cf_4, 0x0A, "")       \
   opcode_macro(end, 0x0B, "!")               \
   opcode_macro(br, 0x0C, "! %")              \
   opcode_macro(br_if, 0x0D, "! %")
#define EOS_VM_BR_TABLE_OP(opcode_macro) \
   opcode_macro(br_table, 0x0E, "") /* TODO: fix br_table formatting */
#define EOS_VM_RETURN_OP(opcode_macro) \
   opcode_macro(return_, 0x0F, "return")
#define EOS_VM_CALL_OPS(opcode_macro)              \
   opcode_macro(call, 0x10, "! %")                 \
   opcode_macro(call_indirect, 0x11, "! (type %)") \
   opcode_macro(br_table_data, 0x12, "") /* TODO: fix br_table formatting */
#define EOS_VM_CALL_IMM_OPS(opcode_macro)                          \
   opcode_macro(call_imm, 0x13, "call %")                          \
   opcode_macro(call_indirect_imm, 0x14, "call_indirect (type %)") \
   opcode_macro(padding_call_1, 0x15, "")                          \
   opcode_macro(padding_call_2, 0x16, "")                          \
   opcode_macro(padding_call_3, 0x17, "")                          \
   opcode_macro(padding_call_4, 0x18, "")                          \
   opcode_macro(padding_call_5, 0x19, "")
#define EOS_VM_PARAMETRIC_OPS(opcode_macro) \
   opcode_macro(drop, 0x1A, "!")            \
   opcode_macro(select, 0x1B, "!")          \
   opcode_macro(padding_param_0, 0x1C, "")  \
   opcode_macro(padding_param_1, 0x1D, "")  \
   opcode_macro(padding_param_2, 0x1E, "")  \
   opcode_macro(padding_param_3, 0x1F, "")
#define EOS_VM_VARIABLE_ACCESS_OPS(opcode_macro) \
   opcode_macro(get_local, 0x20, "! %")          \
   opcode_macro(set_local, 0x21, "! %")          \
   opcode_macro(tee_local, 0x22, "! %")          \
   opcode_macro(get_global, 0x23, "! %")         \
   opcode_macro(set_global, 0x24, "! %")         \
   opcode_macro(padding_va_0, 0x25, "")          \
   opcode_macro(padding_va_1, 0x26, "")          \
   opcode_macro(padding_va_2, 0x27, "")
#define EOS_VM_MEMORY_OPS(opcode_macro)                              \
   opcode_macro(i32_load, 0x28, "i32.load align=% offset=%")         \
   opcode_macro(i64_load, 0x29, "i64.load align=% offset=%")         \
   opcode_macro(f32_load, 0x2A, "f32.load align=% offset=%")         \
   opcode_macro(f64_load, 0x2B, "f64.load align=% offset=%")         \
   opcode_macro(i32_load8_s, 0x2C, "i32.load8_s align=% offset=%")   \
   opcode_macro(i32_load8_u, 0x2D, "i32.load8_u align=% offset=%")   \
   opcode_macro(i32_load16_s, 0x2E, "i32.load16_s align=% offset=%") \
   opcode_macro(i32_load16_u, 0x2F, "i32.load16_u align=% offset=%") \
   opcode_macro(i64_load8_s, 0x30, "i64.load8_s align=% offset=%")   \
   opcode_macro(i64_load8_u, 0x31, "i64.load8_u align=% offset=%")   \
   opcode_macro(i64_load16_s, 0x32, "i64.load16_s align=% offset=%") \
   opcode_macro(i64_load16_u, 0x33, "i64.load16_u align=% offset=%") \
   opcode_macro(i64_load32_s, 0x34, "i64.load32_s align=% offset=%") \
   opcode_macro(i64_load32_u, 0x35, "i64.load32_u align=% offset=%") \
   opcode_macro(i32_store, 0x36, "i32.store align=% offset=%")       \
   opcode_macro(i64_store, 0x37, "i64.store align=% offset=%")       \
   opcode_macro(f32_store, 0x38, "f32.store align=% offset=%")       \
   opcode_macro(f64_store, 0x39, "f64.store align=% offset=%")       \
   opcode_macro(i32_store8, 0x3A, "i32.store8 align=% offset=%")     \
   opcode_macro(i32_store16, 0x3B, "i32.store16 align=% offset=%")   \
   opcode_macro(i64_store8, 0x3C, "i64.store8 align=% offset=%")     \
   opcode_macro(i64_store16, 0x3D, "i64.store16 align=% offset=%")   \
   opcode_macro(i64_store32, 0x3E, "i64.store32 align=% offset=%")   \
   opcode_macro(current_memory, 0x3F, "memory.size")                 \
   opcode_macro(grow_memory, 0x40, "memory.grow")
#define EOS_VM_I32_CONSTANT_OPS(opcode_macro) \
   opcode_macro(i32_const, 0x41, "i32.const %")
#define EOS_VM_I64_CONSTANT_OPS(opcode_macro) \
   opcode_macro(i64_const, 0x42, "i64.const %")
#define EOS_VM_F32_CONSTANT_OPS(opcode_macro) \
   opcode_macro(f32_const, 0x43, "f32.const %")
#define EOS_VM_F64_CONSTANT_OPS(opcode_macro) \
   opcode_macro(f64_const, 0x44, "f64.const %")
#define EOS_VM_COMPARISON_OPS(opcode_macro) \
   opcode_macro(i32_eqz, 0x45, "i32.eqz")   \
   opcode_macro(i32_eq, 0x46, "i32.eq")     \
   opcode_macro(i32_ne, 0x47, "i32.ne")     \
   opcode_macro(i32_lt_s, 0x48, "i32.lt_s") \
   opcode_macro(i32_lt_u, 0x49, "i32.lt_u") \
   opcode_macro(i32_gt_s, 0x4A, "i32.gt_s") \
   opcode_macro(i32_gt_u, 0x4B, "i32.gt_u") \
   opcode_macro(i32_le_s, 0x4C, "i32.le_s") \
   opcode_macro(i32_le_u, 0x4D, "i32.le_u") \
   opcode_macro(i32_ge_s, 0x4E, "i32.ge_s") \
   opcode_macro(i32_ge_u, 0x4F, "i32.ge_u") \
   opcode_macro(i64_eqz, 0x50, "i64.eqz")   \
   opcode_macro(i64_eq, 0x51, "i64.eq")     \
   opcode_macro(i64_ne, 0x52, "i64.ne")     \
   opcode_macro(i64_lt_s, 0x53, "i64.lt_s") \
   opcode_macro(i64_lt_u, 0x54, "i64.lt_u") \
   opcode_macro(i64_gt_s, 0x55, "i64.gt_s") \
   opcode_macro(i64_gt_u, 0x56, "i64.gt_u") \
   opcode_macro(i64_le_s, 0x57, "i64.le_s") \
   opcode_macro(i64_le_u, 0x58, "i64.le_u") \
   opcode_macro(i64_ge_s, 0x59, "i64.ge_s") \
   opcode_macro(i64_ge_u, 0x5A, "i64.ge_u") \
   opcode_macro(f32_eq, 0x5B, "f32.eq")     \
   opcode_macro(f32_ne, 0x5C, "f32.ne")     \
   opcode_macro(f32_lt, 0x5D, "f32.lt")     \
   opcode_macro(f32_gt, 0x5E, "f32.gt")     \
   opcode_macro(f32_le, 0x5F, "f32.le")     \
   opcode_macro(f32_ge, 0x60, "f32.ge")     \
   opcode_macro(f64_eq, 0x61, "f64.eq")     \
   opcode_macro(f64_ne, 0x62, "f64.ne")     \
   opcode_macro(f64_lt, 0x63, "f64.lt")     \
   opcode_macro(f64_gt, 0x64, "f64.gt")     \
   opcode_macro(f64_le, 0x65, "f64.le")     \
   opcode_macro(f64_ge, 0x66, "f64.ge")
#define EOS_VM_NUMERIC_OPS(opcode_macro)            \
   opcode_macro(i32_clz, 0x67, "i32.clz")           \
   opcode_macro(i32_ctz, 0x68, "i32.ctz")           \
   opcode_macro(i32_popcnt, 0x69, "i32.popcnt")     \
   opcode_macro(i32_add, 0x6A, "i32.add")           \
   opcode_macro(i32_sub, 0x6B, "i32.sub")           \
   opcode_macro(i32_mul, 0x6C, "i32.mul")           \
   opcode_macro(i32_div_s, 0x6D, "i32.div_s")       \
   opcode_macro(i32_div_u, 0x6E, "i32.div_u")       \
   opcode_macro(i32_rem_s, 0x6F, "i32.rem_s")       \
   opcode_macro(i32_rem_u, 0x70, "i32.rem_u")       \
   opcode_macro(i32_and, 0x71, "i32.and")           \
   opcode_macro(i32_or, 0x72, "i32.or")             \
   opcode_macro(i32_xor, 0x73, "i32.xor")           \
   opcode_macro(i32_shl, 0x74, "i32.shl")           \
   opcode_macro(i32_shr_s, 0x75, "i32.shr_s")       \
   opcode_macro(i32_shr_u, 0x76, "i32.shr_u")       \
   opcode_macro(i32_rotl, 0x77, "i32.rotl")         \
   opcode_macro(i32_rotr, 0x78, "i32.rotr")         \
   opcode_macro(i64_clz, 0x79, "i64.clz")           \
   opcode_macro(i64_ctz, 0x7A, "i64.ctz")           \
   opcode_macro(i64_popcnt, 0x7B, "i64.popcnt")     \
   opcode_macro(i64_add, 0x7C, "i64.add")           \
   opcode_macro(i64_sub, 0x7D, "i64.sub")           \
   opcode_macro(i64_mul, 0x7E, "i64.mul")           \
   opcode_macro(i64_div_s, 0x7F, "i64.div_s")       \
   opcode_macro(i64_div_u, 0x80, "i64.div_u")       \
   opcode_macro(i64_rem_s, 0x81, "i64.rem_s")       \
   opcode_macro(i64_rem_u, 0x82, "i64.rem_u")       \
   opcode_macro(i64_and, 0x83, "i64.and")           \
   opcode_macro(i64_or, 0x84, "i64.or")             \
   opcode_macro(i64_xor, 0x85, "i64.xor")           \
   opcode_macro(i64_shl, 0x86, "i64.shl")           \
   opcode_macro(i64_shr_s, 0x87, "i64.shr_s")       \
   opcode_macro(i64_shr_u, 0x88, "i64.shr_u")       \
   opcode_macro(i64_rotl, 0x89, "i64.rotl")         \
   opcode_macro(i64_rotr, 0x8A, "i64.rotr")         \
   opcode_macro(f32_abs, 0x8B, "f32.abs")           \
   opcode_macro(f32_neg, 0x8C, "f32.neg")           \
   opcode_macro(f32_ceil, 0x8D, "f32.ceil")         \
   opcode_macro(f32_floor, 0x8E, "f32.floor")       \
   opcode_macro(f32_trunc, 0x8F, "f32.trunc")       \
   opcode_macro(f32_nearest, 0x90, "f32.nearest")   \
   opcode_macro(f32_sqrt, 0x91, "f32.sqrt")         \
   opcode_macro(f32_add, 0x92, "f32.add")           \
   opcode_macro(f32_sub, 0x93, "f32.sub")           \
   opcode_macro(f32_mul, 0x94, "f32.mul")           \
   opcode_macro(f32_div, 0x95, "f32.div")           \
   opcode_macro(f32_min, 0x96, "f32.min")           \
   opcode_macro(f32_max, 0x97, "f32.max")           \
   opcode_macro(f32_copysign, 0x98, "f32.copysign") \
   opcode_macro(f64_abs, 0x99, "f64.abs")           \
   opcode_macro(f64_neg, 0x9A, "f64.neg")           \
   opcode_macro(f64_ceil, 0x9B, "f64.ceil")         \
   opcode_macro(f64_floor, 0x9C, "f64.floor")       \
   opcode_macro(f64_trunc, 0x9D, "f64.trunc")       \
   opcode_macro(f64_nearest, 0x9E, "f64.nearest")   \
   opcode_macro(f64_sqrt, 0x9F, "f64.sqrt")         \
   opcode_macro(f64_add, 0xA0, "f64.add")           \
   opcode_macro(f64_sub, 0xA1, "f64.sub")           \
   opcode_macro(f64_mul, 0xA2, "f64.mul")           \
   opcode_macro(f64_div, 0xA3, "f64.div")           \
   opcode_macro(f64_min, 0xA4, "f64.min")           \
   opcode_macro(f64_max, 0xA5, "f64.max")           \
   opcode_macro(f64_copysign, 0xA6, "f64.copysign")
#define EOS_VM_CONVERSION_OPS(opcode_macro)                       \
   opcode_macro(i32_wrap_i64, 0xA7, "i32.wrap/i64")               \
   opcode_macro(i32_trunc_s_f32, 0xA8, "i32.trunc_s/f32")         \
   opcode_macro(i32_trunc_u_f32, 0xA9, "i32.trunc_u/f32")         \
   opcode_macro(i32_trunc_s_f64, 0xAA, "i32.trunc_s/f64")         \
   opcode_macro(i32_trunc_u_f64, 0xAB, "i32.trunc_u/f64")         \
   opcode_macro(i64_extend_s_i32, 0xAC, "i64.extend_u/i32")       \
   opcode_macro(i64_extend_u_i32, 0xAD, "i64.extend_s/i32")       \
   opcode_macro(i64_trunc_s_f32, 0xAE, "i64.trunc_s/f32")         \
   opcode_macro(i64_trunc_u_f32, 0xAF, "i64.trunc_u/f32")         \
   opcode_macro(i64_trunc_s_f64, 0xB0, "i64.trunc_s/f64")         \
   opcode_macro(i64_trunc_u_f64, 0xB1, "i64.trunc_u/f64")         \
   opcode_macro(f32_convert_s_i32, 0xB2, "f32.convert_s/i32")     \
   opcode_macro(f32_convert_u_i32, 0xB3, "f32.convert_u/i32")     \
   opcode_macro(f32_convert_s_i64, 0xB4, "f32.convert_s/i64")     \
   opcode_macro(f32_convert_u_i64, 0xB5, "f32.convert_u/i64")     \
   opcode_macro(f32_demote_f64, 0xB6, "f32.demote/f64")           \
   opcode_macro(f64_convert_s_i32, 0xB7, "f64.convert_s/i32")     \
   opcode_macro(f64_convert_u_i32, 0xB8, "f64.convert_u/i32")     \
   opcode_macro(f64_convert_s_i64, 0xB9, "f64.convert_s/i64")     \
   opcode_macro(f64_convert_u_i64, 0xBA, "f64.convert_u/i64")     \
   opcode_macro(f64_promote_f32, 0xBB, "f64.promote/f32")         \
   opcode_macro(i32_reinterpret_f32, 0xBC, "i32.reinterpret/f32") \
   opcode_macro(i64_reinterpret_f64, 0xBD, "i64.reinterpret/f64") \
   opcode_macro(f32_reinterpret_i32, 0xBE, "f32.reinterpret/i32") \
   opcode_macro(f64_reinterpret_i64, 0xBF, "f64.reinterpret/i64")
#define EOS_VM_EXIT_OP(opcode_macro) \
   opcode_macro(exit, 0xC0, "exit")
#define EOS_VM_EMPTY_OPS(opcode_macro) \
   opcode_macro(empty0xC1, 0xC1, "")   \
   opcode_macro(empty0xC2, 0xC2, "")   \
   opcode_macro(empty0xC3, 0xC3, "")   \
   opcode_macro(empty0xC4, 0xC4, "")   \
   opcode_macro(empty0xC5, 0xC5, "")   \
   opcode_macro(empty0xC6, 0xC6, "")   \
   opcode_macro(empty0xC7, 0xC7, "")   \
   opcode_macro(empty0xC8, 0xC8, "")   \
   opcode_macro(empty0xC9, 0xC9, "")   \
   opcode_macro(empty0xCA, 0xCA, "")   \
   opcode_macro(empty0xCB, 0xCB, "")   \
   opcode_macro(empty0xCC, 0xCC, "")   \
   opcode_macro(empty0xCD, 0xCD, "")   \
   opcode_macro(empty0xCE, 0xCE, "")   \
   opcode_macro(empty0xCF, 0xCF, "")   \
   opcode_macro(empty0xD0, 0xD0, "")   \
   opcode_macro(empty0xD1, 0xD1, "")   \
   opcode_macro(empty0xD2, 0xD2, "")   \
   opcode_macro(empty0xD3, 0xD3, "")   \
   opcode_macro(empty0xD4, 0xD4, "")   \
   opcode_macro(empty0xD5, 0xD5, "")   \
   opcode_macro(empty0xD6, 0xD6, "")   \
   opcode_macro(empty0xD7, 0xD7, "")   \
   opcode_macro(empty0xD8, 0xD8, "")   \
   opcode_macro(empty0xD9, 0xD9, "")   \
   opcode_macro(empty0xDA, 0xDA, "")   \
   opcode_macro(empty0xDB, 0xDB, "")   \
   opcode_macro(empty0xDC, 0xDC, "")   \
   opcode_macro(empty0xDD, 0xDD, "")   \
   opcode_macro(empty0xDE, 0xDE, "")   \
   opcode_macro(empty0xDF, 0xDF, "")   \
   opcode_macro(empty0xE0, 0xE0, "")   \
   opcode_macro(empty0xE1, 0xE1, "")   \
   opcode_macro(empty0xE2, 0xE2, "")   \
   opcode_macro(empty0xE3, 0xE3, "")   \
   opcode_macro(empty0xE4, 0xE4, "")   \
   opcode_macro(empty0xE5, 0xE5, "")   \
   opcode_macro(empty0xE6, 0xE6, "")   \
   opcode_macro(empty0xE7, 0xE7, "")   \
   opcode_macro(empty0xE8, 0xE8, "")   \
   opcode_macro(empty0xE9, 0xE9, "")   \
   opcode_macro(empty0xEA, 0xEA, "")   \
   opcode_macro(empty0xEB, 0xEB, "")   \
   opcode_macro(empty0xEC, 0xEC, "")   \
   opcode_macro(empty0xED, 0xED, "")   \
   opcode_macro(empty0xEE, 0xEE, "")   \
   opcode_macro(empty0xEF, 0xEF, "")   \
   opcode_macro(empty0xF0, 0xF0, "")   \
   opcode_macro(empty0xF1, 0xF1, "")   \
   opcode_macro(empty0xF2, 0xF2, "")   \
   opcode_macro(empty0xF3, 0xF3, "")   \
   opcode_macro(empty0xF4, 0xF4, "")   \
   opcode_macro(empty0xF5, 0xF5, "")   \
   opcode_macro(empty0xF6, 0xF6, "")   \
   opcode_macro(empty0xF7, 0xF7, "")   \
   opcode_macro(empty0xF8, 0xF8, "")   \
   opcode_macro(empty0xF9, 0xF9, "")   \
   opcode_macro(empty0xFA, 0xFA, "")   \
   opcode_macro(empty0xFB, 0xFB, "")   \
   opcode_macro(empty0xFC, 0xFC, "")   \
   opcode_macro(empty0xFD, 0xFD, "")   \
   opcode_macro(empty0xFE, 0xFE, "")
#define EOS_VM_ERROR_OPS(opcode_macro) \
   opcode_macro(error, 0xFF, "error")

/* clang-format on */

namespace eosio { namespace vm { namespace detail {
   template <std::size_t I, std::size_t Name_N, std::size_t Fmt_N>
   inline constexpr std::string format_str_impl(const char (&name)[Name_N], const char (&fmt)[Fmt_N]) {
      return std::string(fmt);
   }
   template <std::size_t I, std::size_t Name_N, std::size_t Fmt_N, typename Arg, typename... Args>
   inline constexpr std::string format_str_impl(const char (&name)[Name_N], const char (&fmt)[Fmt_N], Arg&& arg, Args&&... args) {
      if constexpr (I == Fmt_N) {
         return std::string("");
      } else {
         if (fmt[I] == '%')
            return std::to_string(std::forward<Arg>(arg)) + format_str_impl<I+1>(name, fmt, std::forward<Args>(args)...);
         else if (fmt[I] == '!')
            return std::string(name) + format_str_impl<I+1>(name, fmt, std::forward<Args>(args)...);
         else
            return std::string(1, fmt[I]) + format_str_impl<I+1>(name, fmt, std::forward<Args>(args)...);
      }
   }

   template <std::size_t Name_N, std::size_t Fmt_N, typename... Args>
   inline constexpr std::string fmt_str(const char (&name)[Name_N], const char (&fmt)[Fmt_N], Args&&... args) {
      return format_str_impl<0>(name, fmt, std::forward<Args>(args)...);
   }
}}} // ns eosio::vm::detail

#define EOS_VM_CREATE_ENUM(name, code, fmt) name = code,

#define EOS_VM_CREATE_STRINGS(name, code, fmt) #name,

#define EOS_VM_CREATE_MAP(name, code, fmt) { code, #name },

#define EOS_VM_OPCODE_NAME_if_
#define EOS_VM_OPCODE_NAME_else_
#define EOS_VM_OPCODE_NAME_return_

#define EOS_VM_OPCODE_NAME_TEST() 1
#define EOS_VM_OPCODE_NAME_TEST_EOS_VM_OPCODE_NAME_TEST 0,
#define EOS_VM_OPCODE_NAME_TEST_1 1, ignore
#define EOS_VM_EXPAND(x) x
#define EOS_VM_CAT2(x, y) x ## y
#define EOS_VM_CAT(x, y) EOS_VM_CAT2(x, y)
#define EOS_VM_APPLY(f, args) f args
#define EOS_VM_FIX_OPCODE_NAME_0(name) name ## _t
#define EOS_VM_FIX_OPCODE_NAME_1(name) name ## t
#define EOS_VM_FIX_OPCODE_NAME(iskeyword, garbage) EOS_VM_FIX_OPCODE_NAME_ ## iskeyword

#define EOS_VM_OPCODE_T(name)                                                             \
   EOS_VM_APPLY(EOS_VM_FIX_OPCODE_NAME,                                                   \
      (EOS_VM_CAT(EOS_VM_OPCODE_NAME_TEST_,                                               \
           EOS_VM_EXPAND(EOS_VM_OPCODE_NAME_TEST EOS_VM_OPCODE_NAME_ ## name ()))))(name)

#define EOS_VM_CREATE_EXIT_TYPE(name, code, fmt)                           \
   struct EOS_VM_OPCODE_T(name) {                                          \
      EOS_VM_OPCODE_T(name)() = default;                                   \
      uint32_t pc;                                                         \
      static constexpr uint8_t opcode = code;                              \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt); } \
   };

#define EOS_VM_CREATE_CONTROL_FLOW_TYPES(name, code, fmt)                        \
   struct EOS_VM_OPCODE_T(name) {                                                \
      EOS_VM_OPCODE_T(name)() {}                                                 \
      EOS_VM_OPCODE_T(name)(uint32_t data) : data(data) {}                       \
      EOS_VM_OPCODE_T(name)(uint32_t d, uint32_t pc, uint16_t i, uint16_t oi)    \
        : data(d), pc(pc), index(i), op_index(oi) {}                             \
      uint32_t data     = 0;                                                     \
      uint32_t pc       = 0;                                                     \
      uint16_t index    = 0;                                                     \
      uint16_t op_index = 0;                                                     \
      static constexpr uint8_t opcode = code;                                    \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt, data); } \
   };

#define EOS_VM_CREATE_BR_TABLE_TYPE(name, code, fmt)                       \
   struct EOS_VM_OPCODE_T(name) {                                          \
      EOS_VM_OPCODE_T(name)() = default;                                   \
      struct elem_t { uint32_t pc; uint32_t stack_pop; };                  \
      elem_t* table;                                                       \
      uint32_t  size;                                                      \
      uint32_t  offset;                                                    \
      static constexpr uint8_t opcode = code;                              \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt); } \
   };

#define EOS_VM_CREATE_TYPES(name, code, fmt)                               \
   struct EOS_VM_OPCODE_T(name) {                                          \
      EOS_VM_OPCODE_T(name)() = default;                                   \
      static constexpr uint8_t opcode = code;                              \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt); } \
   };

#define EOS_VM_CREATE_CALL_TYPES(name, code, fmt)                                 \
   struct EOS_VM_OPCODE_T(name) {                                                 \
      EOS_VM_OPCODE_T(name)() = default;                                          \
      uint32_t index;                                                             \
      static constexpr uint8_t opcode = code;                                     \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt, index); } \
   };

#define EOS_VM_CREATE_CALL_IMM_TYPES(name, code, fmt)                             \
   struct EOS_VM_OPCODE_T(name) {                                                 \
      EOS_VM_OPCODE_T(name)() = default;                                          \
      uint32_t index;                                                             \
      uint16_t locals;                                                            \
      uint16_t return_type;                                                       \
      static constexpr uint8_t opcode = code;                                     \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt, index); } \
   };

#define EOS_VM_CREATE_VARIABLE_ACCESS_TYPES(name, code, fmt)                      \
   struct EOS_VM_OPCODE_T(name) {                                                 \
      EOS_VM_OPCODE_T(name)() = default;                                          \
      uint32_t index;                                                             \
      static constexpr uint8_t opcode = code;                                     \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt, index); } \
   };

#define EOS_VM_CREATE_MEMORY_TYPES(name, code, fmt)                                             \
   struct EOS_VM_OPCODE_T(name) {                                                               \
      EOS_VM_OPCODE_T(name)() = default;                                                        \
      uint32_t flags_align;                                                                     \
      uint32_t offset;                                                                          \
      static constexpr uint8_t opcode = code;                                                   \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt, flags_align, offset); } \
   };

#define EOS_VM_CREATE_I32_CONSTANT_TYPE(name, code, fmt)                            \
   struct EOS_VM_OPCODE_T(name) {                                                   \
      EOS_VM_OPCODE_T(name)() = default;                                            \
      explicit EOS_VM_OPCODE_T(name)(uint32_t n) { data.ui = n; }                   \
      explicit EOS_VM_OPCODE_T(name)(int32_t n) { data.i = n; }                     \
      union {                                                                       \
         uint32_t ui;                                                               \
         int32_t  i;                                                                \
      } data;                                                                       \
      static constexpr uint8_t opcode = code;                                       \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt, data.ui); } \
   };

#define EOS_VM_CREATE_I64_CONSTANT_TYPE(name, code, fmt)                            \
   struct EOS_VM_OPCODE_T(name) {                                                   \
      EOS_VM_OPCODE_T(name)() = default;                                            \
      explicit EOS_VM_OPCODE_T(name)(uint64_t n) { data.ui = n; }                   \
      explicit EOS_VM_OPCODE_T(name)(int64_t n) { data.i = n; }                     \
      union {                                                                       \
         uint64_t ui;                                                               \
         int64_t  i;                                                                \
      } data;                                                                       \
      static constexpr uint8_t opcode = code;                                       \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt, data.ui); } \
   };

#define EOS_VM_CREATE_F32_CONSTANT_TYPE(name, code, fmt)                           \
   struct EOS_VM_OPCODE_T(name) {                                                  \
      EOS_VM_OPCODE_T(name)() = default;                                           \
      explicit EOS_VM_OPCODE_T(name)(uint32_t n) { data.ui = n; }                  \
      explicit EOS_VM_OPCODE_T(name)(float n) { data.f = n; }                      \
      union {                                                                      \
         uint32_t ui;                                                              \
         float    f;                                                               \
      } data;                                                                      \
      static constexpr uint8_t opcode = code;                                      \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt, data.f); } \
   };

#define EOS_VM_CREATE_F64_CONSTANT_TYPE(name, code, fmt)                           \
   struct EOS_VM_OPCODE_T(name) {                                                  \
      EOS_VM_OPCODE_T(name)() = default;                                           \
      explicit EOS_VM_OPCODE_T(name)(uint64_t n) { data.ui = n; }                  \
      explicit EOS_VM_OPCODE_T(name)(double n) { data.f = n; }                     \
      union {                                                                      \
         uint64_t ui;                                                              \
         double   f;                                                               \
      } data;                                                                      \
      static constexpr uint8_t opcode = code;                                      \
      auto wast() const { return eosio::vm::detail::fmt_str(#name, fmt, data.f); } \
   };

#define EOS_VM_IDENTITY(name, code, fmt) eosio::vm::EOS_VM_OPCODE_T(name),
#define EOS_VM_IDENTITY_END(name, code, fmt) eosio::vm::EOS_VM_OPCODE_T(name)
