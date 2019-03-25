#pragma once
#define CONTROL_FLOW_OPS(opcode_macro) \
      opcode_macro(unreachable, 0x00) \
      opcode_macro(nop, 0x01) \
      opcode_macro(block, 0x02) \
      opcode_macro(loop, 0x03) \
      opcode_macro(if_, 0x04) \
      opcode_macro(else_, 0x05) \
      opcode_macro(end, 0x0B) \
      opcode_macro(br, 0x0C) \
      opcode_macro(br_if, 0x0D)
#define BR_TABLE_OP(opcode_macro) \
      opcode_macro(br_table, 0x0E)
#define RETURN_OP(opcode_macro) \
      opcode_macro(return_, 0x0F)
#define CALL_OPS(opcode_macro) \
      opcode_macro(call, 0x10) \
      opcode_macro(call_indirect, 0x11)
#define PARAMETRIC_OPS(opcode_macro) \
      opcode_macro(drop, 0x1A) \
      opcode_macro(select, 0x1B)
#define VARIABLE_ACCESS_OPS(opcode_macro) \
      opcode_macro(get_local, 0x20) \
      opcode_macro(set_local, 0x21) \
      opcode_macro(tee_local, 0x22) \
      opcode_macro(get_global, 0x23) \
      opcode_macro(set_global, 0x24) 
#define MEMORY_OPS(opcode_macro) \
      opcode_macro(i32_load, 0x28) \
      opcode_macro(i64_load, 0x29) \
      opcode_macro(f32_load, 0x2A) \
      opcode_macro(f64_load, 0x2B) \
      opcode_macro(i32_load8_s, 0x2C) \
      opcode_macro(i32_load8_u, 0x2D) \
      opcode_macro(i32_load16_s, 0x2E) \
      opcode_macro(i32_load16_u, 0x2F) \
      opcode_macro(i64_load8_s, 0x30) \
      opcode_macro(i64_load8_u, 0x31) \
      opcode_macro(i64_load16_s, 0x32) \
      opcode_macro(i64_load16_u, 0x33) \
      opcode_macro(i64_load32_s, 0x34) \
      opcode_macro(i64_load32_u, 0x35) \
      opcode_macro(i32_store, 0x36) \
      opcode_macro(i64_store, 0x37) \
      opcode_macro(f32_store, 0x38) \
      opcode_macro(f64_store, 0x39) \
      opcode_macro(i32_store8, 0x3A) \
      opcode_macro(i32_store16, 0x3B) \
      opcode_macro(i64_store8, 0x3C) \
      opcode_macro(i64_store16, 0x3D) \
      opcode_macro(i64_store32, 0x3E) \
      opcode_macro(current_memory, 0x3F) \
      opcode_macro(grow_memory, 0x40)
#define I32_CONSTANT_OPS(opcode_macro) \
      opcode_macro(i32_const, 0x41) 
#define I64_CONSTANT_OPS(opcode_macro) \
      opcode_macro(i64_const, 0x42) 
#define F32_CONSTANT_OPS(opcode_macro) \
      opcode_macro(f32_const, 0x43) 
#define F64_CONSTANT_OPS(opcode_macro) \
      opcode_macro(f64_const, 0x44) 
#define COMPARISON_OPS(opcode_macro) \
      opcode_macro(i32_eqz, 0x45) \
      opcode_macro(i32_eq, 0x46) \
      opcode_macro(i32_ne, 0x47) \
      opcode_macro(i32_lt_s, 0x48) \
      opcode_macro(i32_lt_u, 0x49) \
      opcode_macro(i32_gt_s, 0x4A) \
      opcode_macro(i32_gt_u, 0x4B) \
      opcode_macro(i32_le_s, 0x4C) \
      opcode_macro(i32_le_u, 0x4D) \
      opcode_macro(i32_ge_s, 0x4E) \
      opcode_macro(i32_ge_u, 0x4F) \
      opcode_macro(i64_eqz, 0x50) \
      opcode_macro(i64_eq, 0x51) \
      opcode_macro(i64_ne, 0x52) \
      opcode_macro(i64_lt_s, 0x53) \
      opcode_macro(i64_lt_u, 0x54) \
      opcode_macro(i64_gt_s, 0x55) \
      opcode_macro(i64_gt_u, 0x56) \
      opcode_macro(i64_le_s, 0x57) \
      opcode_macro(i64_le_u, 0x58) \
      opcode_macro(i64_ge_s, 0x59) \
      opcode_macro(i64_ge_u, 0x5A) \
      opcode_macro(f32_eq, 0x5B) \
      opcode_macro(f32_ne, 0x5C) \
      opcode_macro(f32_lt, 0x5D) \
      opcode_macro(f32_gt, 0x5E) \
      opcode_macro(f32_le, 0x5F) \
      opcode_macro(f32_ge, 0x60) \
      opcode_macro(f64_eq, 0x61) \
      opcode_macro(f64_ne, 0x62) \
      opcode_macro(f64_lt, 0x63) \
      opcode_macro(f64_gt, 0x64) \
      opcode_macro(f64_le, 0x65) \
      opcode_macro(f64_ge, 0x66) 
#define NUMERIC_OPS(opcode_macro) \
      opcode_macro(i32_clz, 0x67) \
      opcode_macro(i32_ctz, 0x68) \
      opcode_macro(i32_popcnt, 0x69) \
      opcode_macro(i32_add, 0x6A) \
      opcode_macro(i32_sub, 0x6B) \
      opcode_macro(i32_mul, 0x6C) \
      opcode_macro(i32_div_s, 0x6D) \
      opcode_macro(i32_div_u, 0x6E) \
      opcode_macro(i32_rem_s, 0x6F) \
      opcode_macro(i32_rem_u, 0x70) \
      opcode_macro(i32_and, 0x71) \
      opcode_macro(i32_or, 0x72) \
      opcode_macro(i32_xor, 0x73) \
      opcode_macro(i32_shl, 0x74) \
      opcode_macro(i32_shr_s, 0x75) \
      opcode_macro(i32_shr_u, 0x76) \
      opcode_macro(i32_rotl, 0x77) \
      opcode_macro(i32_rotr, 0x78) \
      opcode_macro(i64_clz, 0x79) \
      opcode_macro(i64_ctz, 0x7A) \
      opcode_macro(i64_popcnt, 0x7B) \
      opcode_macro(i64_add, 0x7C) \
      opcode_macro(i64_sub, 0x7D) \
      opcode_macro(i64_mul, 0x7E) \
      opcode_macro(i64_div_s, 0x7F) \
      opcode_macro(i64_div_u, 0x80) \
      opcode_macro(i64_rem_s, 0x81) \
      opcode_macro(i64_rem_u, 0x82) \
      opcode_macro(i64_and, 0x83) \
      opcode_macro(i64_or, 0x84) \
      opcode_macro(i64_xor, 0x85) \
      opcode_macro(i64_shl, 0x86) \
      opcode_macro(i64_shr_s, 0x87) \
      opcode_macro(i64_shr_u, 0x88) \
      opcode_macro(i64_rotl, 0x89) \
      opcode_macro(i64_rotr, 0x8A) \
      opcode_macro(f32_abs, 0x8B) \
      opcode_macro(f32_neg, 0x8C) \
      opcode_macro(f32_ceil, 0x8D) \
      opcode_macro(f32_floor, 0x8E) \
      opcode_macro(f32_trunc, 0x8F) \
      opcode_macro(f32_nearest, 0x90) \
      opcode_macro(f32_sqrt, 0x91) \
      opcode_macro(f32_add, 0x92) \
      opcode_macro(f32_sub, 0x93) \
      opcode_macro(f32_mul, 0x94) \
      opcode_macro(f32_div, 0x95) \
      opcode_macro(f32_min, 0x96) \
      opcode_macro(f32_max, 0x97) \
      opcode_macro(f32_copysign, 0x98) \
      opcode_macro(f64_abs, 0x99) \
      opcode_macro(f64_neg, 0x9A) \
      opcode_macro(f64_ceil, 0x9B) \
      opcode_macro(f64_floor, 0x9C) \
      opcode_macro(f64_trunc, 0x9D) \
      opcode_macro(f64_nearest, 0x9E) \
      opcode_macro(f64_sqrt, 0x9F) \
      opcode_macro(f64_add, 0xA0) \
      opcode_macro(f64_sub, 0xA1) \
      opcode_macro(f64_mul, 0xA2) \
      opcode_macro(f64_div, 0xA3) \
      opcode_macro(f64_min, 0xA4) \
      opcode_macro(f64_max, 0xA5) \
      opcode_macro(f64_copysign, 0xA6) 
#define CONVERSION_OPS(opcode_macro) \
      opcode_macro(i32_wrap_i64, 0xA7) \
      opcode_macro(i32_trunc_s_f32, 0xA8) \
      opcode_macro(i32_trunc_u_f32, 0xA9) \
      opcode_macro(i32_trunc_s_f64, 0xAA) \
      opcode_macro(i32_trunc_u_f64, 0xAB) \
      opcode_macro(i64_extend_s_i32, 0xAC) \
      opcode_macro(i64_extend_u_i32, 0xAD) \
      opcode_macro(i64_trunc_s_f32, 0xAE) \
      opcode_macro(i64_trunc_u_f32, 0xAF) \
      opcode_macro(i64_trunc_s_f64, 0xB0) \
      opcode_macro(i64_trunc_u_f64, 0xB1) \
      opcode_macro(f32_convert_s_i32, 0xB2) \
      opcode_macro(f32_convert_u_i32, 0xB3) \
      opcode_macro(f32_convert_s_i64, 0xB4) \
      opcode_macro(f32_convert_u_i64, 0xB5) \
      opcode_macro(f32_demote_f64, 0xB6) \
      opcode_macro(f64_convert_s_i32, 0xB7) \
      opcode_macro(f64_convert_u_i32, 0xB8) \
      opcode_macro(f64_convert_s_i64, 0xB9) \
      opcode_macro(f64_convert_u_i64, 0xBA) \
      opcode_macro(f64_promote_f32, 0xBB) \
      opcode_macro(i32_reinterpret_f32, 0xBC) \
      opcode_macro(i64_reinterpret_f64, 0xBD) \
      opcode_macro(f32_reinterpret_i32, 0xBE) \
      opcode_macro(f64_reinterpret_i64, 0xBF) 
#define SYNTHETIC_OPS(opcode_macro) \
      opcode_macro(fend, 0xC0)
#define ERROR_OPS(opcode_macro) \
      opcode_macro(error, 0xC2) 

#define CREATE_ENUM(name, code) \
   name = code,

#define CREATE_STRINGS(name, code) \
   #name,

#define CREATE_SYNTHETIC_TYPES(name, code)       \
   struct name##_t {                            \
      uint32_t pc;                              \
   };

#define CREATE_MAP(name, code) \
  {code, #name},

#define CREATE_CONTROL_FLOW_TYPES(name, code) \
   struct name##_t {                          \
      uint32_t data;                          \
      uint32_t pc;                            \
      uint16_t index;                         \
      uint16_t op_index;                      \
   };

#define CREATE_BR_TABLE_TYPE(name, code) \
   struct name##_t {                     \
      uint32_t* table;                   \
      uint32_t  size;                    \
      uint32_t  default_target;          \
   };

#define CREATE_TYPES(name, code) \
   struct name##_t {};

#define CREATE_CALL_TYPES(name, code) \
   struct name##_t {                  \
      uint32_t index;                 \
   };

#define CREATE_VARIABLE_ACCESS_TYPES(name, code) \
   struct name##_t {                             \
      uint32_t index;                            \
   };

#define CREATE_MEMORY_TYPES(name, code) \
   struct name##_t {                    \
      uint32_t flags_align;             \
      uint32_t offset;                  \
   };


#define CREATE_I32_CONSTANT_TYPE(name, code)          \
   struct name##_t {                                  \
      explicit name##_t (uint32_t n) { data.ui = n; } \
      explicit name##_t (int32_t n) { data.i = n; }   \
      union {                                         \
         uint32_t ui;                                 \
         int32_t  i;                                  \
      } data;                                         \
   };

#define CREATE_I64_CONSTANT_TYPE(name, code)          \
   struct name##_t {                                  \
      explicit name##_t (uint64_t n) { data.ui = n; } \
      explicit name##_t (int64_t n) { data.i = n; }   \
      union {                                         \
         uint64_t ui;                                 \
         int64_t  i;                                  \
      } data;                                         \
   };

#define CREATE_F32_CONSTANT_TYPE(name, code)          \
   struct name##_t {                                  \
      explicit name##_t (uint32_t n) { data.ui = n; } \
      explicit name##_t (float n) { data.f = n; }     \
      union {                                         \
         uint32_t ui;                                 \
         float f;                                     \
      } data;                                         \
   };

#define CREATE_F64_CONSTANT_TYPE(name, code)          \
   struct name##_t {                                  \
      explicit name##_t (uint64_t n) { data.ui = n; } \
      explicit name##_t (double n) { data.f = n; }    \
      union {                                         \
         uint64_t ui;                                 \
         double f;                                    \
      } data;                                         \
   };

#define IDENTITY(name, code) \
   eosio::wasm_backend::name##_t,
#define IDENTITY_END(name, code) \
   eosio::wasm_backend::name##_t

#define DBG_VISIT(name, code)                                           \
   void operator()( name##_t& op ) {                              \
      std::cout << "Found an " << #name << " at " << interpret_visitor<Backend>::get_context().get_pc() << "\n"; \
      interpret_visitor<Backend>::operator()(op);                                \
   }
