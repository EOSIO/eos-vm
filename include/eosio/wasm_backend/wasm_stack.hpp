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

namespace eosio { namespace wasm_backend {
   using stack_elem = std::variant<uint32_t, i32_const_t, i64_const_t, f32_const_t, f64_const_t, block_t, loop_t, if__t>;

   template <size_t Elems>
   class fixed_stack {
      public:
         fixed_stack() {
            _s.resize(Elems);
         }
         void push(stack_elem e) {
            _s[_index++] = e;
         }
         stack_elem pop() {
            EOS_WB_ASSERT(_index > 0, wasm_interpreter_exception, "empty stack");
            return _s[--_index];
         }
         uint8_t size()const {
            return _index;
         }
      private:
         native_vector<stack_elem> _s;
         uint8_t                   _index = 0;
   };

   using control_stack = fixed_stack<constants::max_nested_structures>;
   using operand_stack = fixed_stack<constants::max_stack_size>;
   using call_stack    = fixed_stack<constants::max_call_depth>;

}} // namespace eosio::wasm_backend
