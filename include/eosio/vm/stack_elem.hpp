#pragma once

#include <eosio/vm/types.hpp>
#include <eosio/vm/variant.hpp>

#include <cstdint>
#include <string>
#include <variant>

namespace eosio { namespace vm {
   using stack_elem = variant<activation_frame, i32_const_t, i64_const_t, f32_const_t, f64_const_t, block_t,
                                   loop_t, if__t, else__t, end_t>;
   
   inline int32_t&  to_i32(stack_elem& elem) { return reinterpret_cast<i32_const_t*>(elem.get_raw())->data.i; }
   inline uint32_t& to_ui32(stack_elem& elem) { return reinterpret_cast<i32_const_t*>(elem.get_raw())->data.ui; }
   inline float&    to_f32(stack_elem& elem) { return reinterpret_cast<f32_const_t*>(elem.get_raw())->data.f; }
   inline uint32_t& to_fui32(stack_elem& elem) { return reinterpret_cast<f32_const_t*>(elem.get_raw())->data.ui; }
/*
   inline int32_t&  to_i32(stack_elem& elem) { return reinterpret_cast<i32_const_t*>(elem.get_raw())->data.i; }
   inline uint32_t& to_ui32(stack_elem& elem) { return reinterpret_cast<i32_const_t*>(elem.get_raw())->data.ui; }
   inline float&    to_f32(stack_elem& elem) { return reinterpret_cast<f32_const_t*>(elem.get_raw())->data.f; }
   inline uint32_t& to_fui32(stack_elem& elem) { return reinterpret_cast<f32_const_t*>(elem.get_raw())->data.ui; }
*/
   inline int64_t&  to_i64(stack_elem& elem) { return reinterpret_cast<i64_const_t*>(elem.get_raw())->data.i; }
   inline uint64_t& to_ui64(stack_elem& elem) { return reinterpret_cast<i64_const_t*>(elem.get_raw())->data.ui; }
   inline double&   to_f64(stack_elem& elem) { return reinterpret_cast<f64_const_t*>(elem.get_raw())->data.f; }
   inline uint64_t& to_fui64(stack_elem& elem) { return reinterpret_cast<f64_const_t*>(elem.get_raw())->data.ui; }
/*
   inline int64_t&  to_i64(stack_elem& elem) { return reinterpret_cast<i64_const_t*>(elem.get_raw())->data.i; }
   inline uint64_t& to_ui64(stack_elem& elem) { return reinterpret_cast<i64_const_t*>(elem.get_raw())->data.ui; }
   inline double&   to_f64(stack_elem& elem) { return reinterpret_cast<f64_const_t*>(elem.get_raw())->data.f; }
   inline uint64_t& to_fui64(stack_elem& elem) { return reinterpret_cast<f64_const_t*>(elem.get_raw())->data.ui; }
   */
}} // nameo::vm
