#pragma once

#include <string>
#include <variant>
#include <eosio/wasm_backend/utils.hpp>
#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/types.hpp>

namespace eosio { namespace wasm_backend {
   using stack_elem = std::variant<activation_frame, i32_const_t, i64_const_t, f32_const_t, f64_const_t, block_t, loop_t, if__t, else__t>;

   template <typename T>
   inline bool is_a( const stack_elem& el ){ return std::holds_alternative<T>(el); }

   template <typename Backend>
   class fixed_stack {
      public:
         fixed_stack(size_t max_size, Backend& backend) : _alloc(max_size) {}
         void push(stack_elem e) {
            _alloc.get_base_ptr()[_index++] = e;
         }
         stack_elem& get(uint32_t index)const {
            return _alloc.get_base_ptr()[index];
         }
         void set(uint32_t index, const stack_elem& el) {
            _alloc.get_base_ptr()[index] = el;
         }
         stack_elem pop() {
            return _alloc.get_base_ptr()[--_index];
         }
         void eat(uint32_t index) {
            _index = index;
         }
         uint16_t current_index()const { return _index; }
         stack_elem& peek() {
            return _alloc.get_base_ptr()[_index-1];
         }
         const stack_elem& peek()const {
            return _alloc.get_base_ptr()[_index-1];
         }
         stack_elem& peek(size_t i) {
            return _alloc.get_base_ptr()[_index-1-i];
         }
         uint16_t size()const {
            return _index;
         }
      private:
         fixed_stack_allocator<stack_elem> _alloc;
         uint16_t              _index = 0;
   };

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
