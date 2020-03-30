#pragma once

#include <eosio/vm/wasm_stack.hpp>

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

      char* memory;
      operand_stack* os;
   };
}} // ns eosio::vm
