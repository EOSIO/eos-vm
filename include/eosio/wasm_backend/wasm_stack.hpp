#pragma once

/*
 * definitions from https://github.com/WebAssembly/design/blob/master/BinaryEncoding.md
 */

#include <string>
#include <variant>
#include <eosio/wasm_backend/utils.hpp>
#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/vector.hpp>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/types.hpp>

namespace eosio { namespace wasm_backend {
   using stack_elem = std::variant<activation_frame, i32_const_t, i64_const_t, f32_const_t, f64_const_t, block_t, loop_t, if__t, else__t>;

   template <typename T>
   inline bool is_a( const stack_elem& el ){ return std::holds_alternative<T>(el); }
   template <size_t Elems>
   class fixed_stack {
      public:
         fixed_stack() {
            _s.resize(Elems);
         }
         void push(stack_elem e) {
            _s[_index++] = e;
         }
         stack_elem& get(uint32_t index)const {
            EOS_WB_ASSERT(index <= _index, wasm_interpreter_exception, "invalid stack index");
            return _s[index];
         }
         void set(uint32_t index, const stack_elem& el) {
            EOS_WB_ASSERT(index <= _index, wasm_interpreter_exception, "invalid stack index");
            EOS_WB_ASSERT(el.index() == _s[index].index(), wasm_invalid_element, "set element must match the type of held element");
            _s[index] = el;
         }
         stack_elem pop() {
            return _s[--_index];
         }
         void eat(uint32_t index) {
            _index = index;
         }
         uint16_t current_index()const { return _index; }
         stack_elem& peek() {
            return _s[_index-1];
         }
         stack_elem& peek(size_t i) {
            return _s[_index-1-i];
         }
         uint8_t size()const {
            return _index;
         }
      private:
         native_vector<stack_elem> _s;
         uint16_t                  _index = 0;
   };
      /*
   template <typename Stream>
   Stream& operator<<(Stream& os, const stack_elem& el) {
      std::visit(overloaded {
            [&](const i32_const_t& i){
               os << "i32:" << i.data.i;
            }, [&](const i64_const_t& i){
               os << "i64:" << i.data.i;
            }, [&](const f32_const_t& f){
               os << "f32:" << f.data.f;
            }, [&](const f64_const_t& f){
               os << "f64:" << f.data.f;
            }, [&](auto){
               throw wasm_interpreter_exception{"stream error"};
            }
         }, el);
      return os;
   }
      */
   using control_stack = fixed_stack<constants::max_nested_structures>;
   using operand_stack = fixed_stack<constants::max_stack_size>;
   using call_stack    = fixed_stack<constants::max_call_depth>;

}} // namespace eosio::wasm_backend
