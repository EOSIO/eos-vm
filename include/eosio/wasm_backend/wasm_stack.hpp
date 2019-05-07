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

   template <size_t Elems, typename Allocator>
   class fixed_stack {
      public:
         fixed_stack(Allocator& alloc) : _s(managed_vector<stack_elem, Allocator>{ alloc, Elems }){}
         void push(stack_elem e) {
            _s[_index++] = e;
         }
         stack_elem& get(uint32_t index)const {
            EOS_WB_ASSERT(index <= _index, wasm_interpreter_exception, "invalid stack index");
            return _s[index];
         }
         void set(uint32_t index, const stack_elem& el) {
            EOS_WB_ASSERT(index <= _index, wasm_interpreter_exception, "invalid stack index");
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
         const stack_elem& peek()const {
            return _s[_index-1];
         }
         stack_elem& peek(size_t i) {
            return _s[_index-1-i];
         }
         stack_elem get_back(size_t i) {
            return _s[_index-1-i];
         }
         void trim(size_t amt) {
            _index -= amt;
         }
         uint16_t size()const {
            return _index;
         }
      private:
         managed_vector<stack_elem, Allocator> _s;
         uint16_t                              _index = 0;
   };

   using control_stack = fixed_stack<constants::max_nested_structures, bounded_allocator>;
   using operand_stack = fixed_stack<constants::max_stack_size, bounded_allocator>;
   using call_stack    = fixed_stack<constants::max_call_depth, bounded_allocator>;

}} // namespace eosio::wasm_backend

#define TO_INT32(X)                                      \
   std::get<eosio::wasm_backend::i32_const_t>(X).data.i

#define TO_INT64(X)                                      \
   std::get<eosio::wasm_backend::i64_const_t>(X).data.i

#define TO_UINT32(X)                                     \
   std::get<eosio::wasm_backend::i32_const_t>(X).data.ui

#define TO_UINT64(X)                                     \
   std::get<eosio::wasm_backend::i64_const_t>(X).data.ui

#define TO_FUINT32(X)                                    \
   std::get<eosio::wasm_backend::f32_const_t>(X).data.ui

#define TO_FUINT64(X)                                    \
   std::get<eosio::wasm_backend::f64_const_t>(X).data.ui

#define TO_F32(X)                                        \
   std::get<eosio::wasm_backend::f32_const_t>(X).data.f

#define TO_F64(X)                                        \
   std::get<eosio::wasm_backend::f64_const_t>(X).data.f
