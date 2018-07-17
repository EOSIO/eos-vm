#pragma once

namespace eosio { namespace wasm_backend {
   struct opcode {
      static constexpr uint16_t unreachable_code      = 0x00,
      static constexpr uint16_t nop                   = 0x01,
      static constexpr uint16_t block                 = 0x02,
      static constexpr uint16_t loop                  = 0x03,
      static constexpr uint16_t if_                   = 0x04,
      static constexpr uint16_t else_                 = 0x05,
      static constexpr uint16_t end                   = 0x0B,
      static constexpr uint16_t br                    = 0x0C,
      static constexpr uint16_t br_if                 = 0x0D,
      static constexpr uint16_t br_table              = 0x0E,
      static constexpr uint16_t return_               = 0x0F,
      static constexpr uint16_t call                  = 0x10,
      static constexpr uint16_t call_indirect         = 0x11,
      static constexpr uint16_t drop                  = 0x1A,
      static constexpr uint16_t select                = 0x1B,
      static constexpr uint16_t get_local             = 0x20,
      static constexpr uint16_t set_local             = 0x21,
      static constexpr uint16_t tee_local             = 0x22,
      static constexpr uint16_t get_global            = 0x23,
      static constexpr uint16_t set_global            = 0x24,
      static constexpr uint16_t i32_load              = 0x28,
      static constexpr uint16_t i64_load              = 0x29,
      static constexpr uint16_t f32_load              = 0x2A,
      static constexpr uint16_t f64_load              = 0x2B,
      static constexpr uint16_t i32_load8_s           = 0x2C,
      static constexpr uint16_t i32_load8_u           = 0x2D,
      static constexpr uint16_t i32_load16_s          = 0x2E,
      static constexpr uint16_t i32_load16_u          = 0x2F,
      static constexpr uint16_t i64_load8_s           = 0x30,
      static constexpr uint16_t i64_load8_u           = 0x31,
      static constexpr uint16_t i64_load16_s          = 0x32,
      static constexpr uint16_t i64_load16_u          = 0x33,
      static constexpr uint16_t i64_load32_s          = 0x34,
      static constexpr uint16_t i64_load32_u          = 0x35,
      static constexpr uint16_t i32_store             = 0x36,
      static constexpr uint16_t i64_store             = 0x37,
      static constexpr uint16_t f32_store             = 0x38,
      static constexpr uint16_t f64_store             = 0x39,
      static constexpr uint16_t i32_store8            = 0x3A,
      static constexpr uint16_t i32_store16           = 0x3B,
      static constexpr uint16_t i64_store8            = 0x3C,
      static constexpr uint16_t i64_store16           = 0x3D,
      static constexpr uint16_t i64_store32           = 0x3E,
      static constexpr uint16_t current_memory        = 0x3F,
      static constexpr uint16_t grow_memory           = 0x40,
      static constexpr uint16_t i32_const             = 0x41,
      static constexpr uint16_t i64_const             = 0x42,
      static constexpr uint16_t f32_const             = 0x43,
      static constexpr uint16_t f64_const             = 0x44,
      static constexpr uint16_t i32_eqz               = 0x45,
      static constexpr uint16_t i32_eq                = 0x46,
      static constexpr uint16_t i32_ne                = 0x47,
      static constexpr uint16_t i32_lt_s              = 0x48,
      static constexpr uint16_t i32_lt_u              = 0x49,
      static constexpr uint16_t i32_gt_s              = 0x4A,
      static constexpr uint16_t i32_gt_u              = 0x4B,
      static constexpr uint16_t i32_le_s              = 0x4C,
      static constexpr uint16_t i32_le_u              = 0x4D,
      static constexpr uint16_t i32_ge_s              = 0x4E,
      static constexpr uint16_t i32_ge_u              = 0x4F,
      static constexpr uint16_t i64_eqz               = 0x50,
      static constexpr uint16_t i64_eq                = 0x51,
      static constexpr uint16_t i64_ne                = 0x52,
      static constexpr uint16_t i64_lt_s              = 0x53,
      static constexpr uint16_t i64_lt_u              = 0x54,
      static constexpr uint16_t i64_gt_s              = 0x55,
      static constexpr uint16_t i64_gt_u              = 0x56,
      static constexpr uint16_t i64_le_s              = 0x57,
      static constexpr uint16_t i64_le_u              = 0x58,
      static constexpr uint16_t i64_ge_s              = 0x59,
      static constexpr uint16_t i64_ge_u              = 0x5A,
      static constexpr uint16_t f32_eq                = 0x5B,
      static constexpr uint16_t f32_ne                = 0x5C,
      static constexpr uint16_t f32_lt                = 0x5D,
      static constexpr uint16_t f32_gt                = 0x5E,
      static constexpr uint16_t f32_le                = 0x5F,
      static constexpr uint16_t f32_ge                = 0x60,
      static constexpr uint16_t f64_eq                = 0x61,
      static constexpr uint16_t f64_ne                = 0x62,
      static constexpr uint16_t f64_lt                = 0x63,
      static constexpr uint16_t f64_gt                = 0x64,
      static constexpr uint16_t f64_le                = 0x65,
      static constexpr uint16_t f64_ge                = 0x66,
      static constexpr uint16_t i32_clz               = 0x67,
      static constexpr uint16_t i32_ctz               = 0x68,
      static constexpr uint16_t i32_popcnt            = 0x69,
      static constexpr uint16_t i32_add               = 0x6A,
      static constexpr uint16_t i32_sub               = 0x6B,
      static constexpr uint16_t i32_mul               = 0x6C,
      static constexpr uint16_t i32_div_s             = 0x6D,
      static constexpr uint16_t i32_div_u             = 0x6E,
      static constexpr uint16_t i32_rem_s             = 0x6F,
      static constexpr uint16_t i32_rem_u             = 0x70,
      static constexpr uint16_t i32_and               = 0x71,
      static constexpr uint16_t i32_or                = 0x72,
      static constexpr uint16_t i32_xor               = 0x73,
      static constexpr uint16_t i32_shl               = 0x74,
      static constexpr uint16_t i32_shr_s             = 0x75,
      static constexpr uint16_t i32_shr_u             = 0x76,
      static constexpr uint16_t i32_rotl              = 0x77,
      static constexpr uint16_t i32_rotr              = 0x78,
      static constexpr uint16_t i64_clz               = 0x79,
      static constexpr uint16_t i64_ctz               = 0x7A,
      static constexpr uint16_t i64_popcnt            = 0x7B,
      static constexpr uint16_t i64_add               = 0x7C,
      static constexpr uint16_t i64_sub               = 0x7D,
      static constexpr uint16_t i64_mul               = 0x7E,
      static constexpr uint16_t i64_div_s             = 0x7F,
      static constexpr uint16_t i64_div_u             = 0x80,
      static constexpr uint16_t i64_rem_s             = 0x81,
      static constexpr uint16_t i64_rem_u             = 0x82,
      static constexpr uint16_t i64_and               = 0x83,
      static constexpr uint16_t i64_or                = 0x84,
      static constexpr uint16_t i64_xor               = 0x85,
      static constexpr uint16_t i64_shl               = 0x86,
      static constexpr uint16_t i64_shr_s             = 0x87,
      static constexpr uint16_t i64_shr_u             = 0x88,
      static constexpr uint16_t i64_rotl              = 0x89,
      static constexpr uint16_t i64_rotr              = 0x8A,
      static constexpr uint16_t f32_abs               = 0x8B,
      static constexpr uint16_t f32_neg               = 0x8C,
      static constexpr uint16_t f32_ceil              = 0x8D,
      static constexpr uint16_t f32_floor             = 0x8E,
      static constexpr uint16_t f32_trunc             = 0x8F,
      static constexpr uint16_t f32_nearest           = 0x90,
      static constexpr uint16_t f32_sqrt              = 0x91,
      static constexpr uint16_t f32_add               = 0x92,
      static constexpr uint16_t f32_sub               = 0x93,
      static constexpr uint16_t f32_mul               = 0x94,
      static constexpr uint16_t f32_div               = 0x95,
      static constexpr uint16_t f32_min               = 0x96,
      static constexpr uint16_t f32_max               = 0x97,
      static constexpr uint16_t f32_copysign          = 0x98,
      static constexpr uint16_t f64_abs               = 0x99,
      static constexpr uint16_t f64_neg               = 0x9A,
      static constexpr uint16_t f64_ceil              = 0x9B,
      static constexpr uint16_t f64_floor             = 0x9C,
      static constexpr uint16_t f64_trunc             = 0x9D,
      static constexpr uint16_t f64_nearest           = 0x9E,
      static constexpr uint16_t f64_sqrt              = 0x9F,
      static constexpr uint16_t f64_add               = 0xA0,
      static constexpr uint16_t f64_sub               = 0xA1,
      static constexpr uint16_t f64_mul               = 0xA2,
      static constexpr uint16_t f64_div               = 0xA3,
      static constexpr uint16_t f64_min               = 0xA4,
      static constexpr uint16_t f64_max               = 0xA5,
      static constexpr uint16_t f64_copysign          = 0xA6,
      static constexpr uint16_t i32_wrap_i64          = 0xA7,
      static constexpr uint16_t i32_trunc_s_f32       = 0xA8,
      static constexpr uint16_t i32_trunc_u_f32       = 0xA9,
      static constexpr uint16_t i32_trunc_s_f64       = 0xAA,
      static constexpr uint16_t i32_trunc_u_f64       = 0xAB,
      static constexpr uint16_t i64_extend_s_i32      = 0xAC,
      static constexpr uint16_t i64_extend_u_i32      = 0xAD,
      static constexpr uint16_t i64_trunc_s_f32       = 0xAE,
      static constexpr uint16_t i64_trunc_u_f32       = 0xAF,
      static constexpr uint16_t i64_trunc_s_f64       = 0xB0,
      static constexpr uint16_t i64_trunc_u_f64       = 0xB1,
      static constexpr uint16_t f32_convert_s_i32     = 0xB2,
      static constexpr uint16_t f32_convert_u_i32     = 0xB3,
      static constexpr uint16_t f32_convert_s_i64     = 0xB4,
      static constexpr uint16_t f32_convert_u_i64     = 0xB5,
      static constexpr uint16_t f32_demote_f64        = 0xB6,
      static constexpr uint16_t f64_convert_s_i32     = 0xB7,
      static constexpr uint16_t f64_convert_u_i32     = 0xB8,
      static constexpr uint16_t f64_convert_s_i64     = 0xB9,
      static constexpr uint16_t f64_convert_u_i64     = 0xBA,
      static constexpr uint16_t f64_promote_f32       = 0xBB,
      static constexpr uint16_t i32_reinterpret_f32   = 0xBC,
      static constexpr uint16_t i64_reinterpret_f64   = 0xBD,
      static constexpr uint16_t f32_reinterpret_i32   = 0xBE,
      static constexpr uint16_t f64_reinterpret_i64   = 0xBF,
      static constexpr uint16_t error                 = 0xFF
   }; // code
}} // namespace eosio::wasm_backend
