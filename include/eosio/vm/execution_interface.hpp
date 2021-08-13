#pragma once

#include <eosio/vm/wasm_stack.hpp>
#include <eosio/vm/utils.hpp>
#include <eosio/vm/exceptions.hpp>
#include <cstring>
#include <limits>

namespace eosio { namespace vm {

   // interface used for the host function system to use
   // clients can create their own interface to overlay their own implementations
   struct execution_interface {
      inline execution_interface( char* memory, operand_stack* os ) : memory(memory), os(os) {}
      inline void* get_memory() const { return memory; }
      inline void trim_operands(std::size_t amt) { os->trim(amt); }

      template <typename T>
      inline void push_operand(T&& op) { os->push(std::forward<T>(op)); }
      inline auto pop_operand() { return os->pop(); }
      inline const auto& operand_from_back(std::size_t index) const { return os->get_back(index); }

      template <typename T>
      inline void* validate_pointer(wasm_ptr_t ptr, wasm_size_t len) const {
         auto result = memory + ptr;
         validate_pointer<T>(result, len);
         return result;
      }

      template <typename T>
      inline void validate_pointer(const void* ptr, wasm_size_t len) const {
         EOS_VM_ASSERT( len <= std::numeric_limits<wasm_size_t>::max() / (wasm_size_t)sizeof(T), wasm_interpreter_exception, "length will overflow" );
         volatile auto check_addr = *(reinterpret_cast<const char*>(ptr) + (len * sizeof(T)) - 1);
         ignore_unused_variable_warning(check_addr);
      }

      inline void* validate_null_terminated_pointer(wasm_ptr_t ptr) const {
         auto result = memory + ptr;
         validate_null_terminated_pointer(result);
         return result;
      }

      inline void validate_null_terminated_pointer(const void* ptr) const {
         volatile auto check_addr = std::strlen(static_cast<const char*>(ptr));
         ignore_unused_variable_warning(check_addr);
      }
      char* memory;
      operand_stack* os;
   };
}} // ns eosio::vm
