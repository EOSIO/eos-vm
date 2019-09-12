#pragma once

/*
 * definitions from https://github.com/WebAssembly/design/blob/master/BinaryEncoding.md
 */

#include <eosio/vm/allocator.hpp>
#include <eosio/vm/exceptions.hpp>
#include <eosio/vm/stack_elem.hpp>
#include <eosio/vm/types.hpp>
#include <eosio/vm/utils.hpp>
#include <eosio/vm/vector.hpp>

namespace eosio { namespace vm {
   template <typename ElemT, typename Allocator = nullptr_t, size_t Elems = std::numeric_limits<size_t>::max() >
   class stack {
    public:
      template <typename Alloc=Allocator, typename = std::enable_if_t<std::is_same_v<Alloc, nullptr_t>, int>>
      stack(Alloc) 
         : _store(std::vector<ElemT>{ constants::max_stack_size }) {}

      template <typename Alloc=Allocator, typename = std::enable_if_t<!std::is_same_v<Alloc, nullptr_t>, int>>
      stack(Alloc* alloc) 
         : _store(managed_vector<ElemT, Alloc>{*alloc, Elems }) {}
      void   push(ElemT e) { _store[_index++] = e; }

      ElemT& get(uint32_t index) const {
         EOS_VM_ASSERT(index <= _index, wasm_interpreter_exception, "invalid stack index");
         return (ElemT&)_store[index];
      }
      void set(uint32_t index, const ElemT& el) {
         EOS_VM_ASSERT(index <= _index, wasm_interpreter_exception, "invalid stack index");
         _store[index] = el;
      }
      ElemT        pop() { return _store[--_index]; }
      void         eat(uint32_t index) { _index = index; }
      // compact the last element to the element pointed to by index
      void         compact(uint32_t index) { 
         _store[index] = _store[_index-1];
         _index = index+1;
      }
      uint16_t     current_index() const { return _index; }
      ElemT&       peek() { return _store[_index - 1]; }
      const ElemT& peek() const { return _store[_index - 1]; }
      ElemT&       peek(size_t i) { return _store[_index - 1 - i]; }
      ElemT        get_back(size_t i) { return _store[_index - 1 - i]; }
      void         trim(size_t amt) { _index -= amt; }
      uint16_t     size() const { return _index; }

    private:
      template <bool WhichVector, typename Elem, typename Alloc>
      struct base_data_store {
         using type = std::vector<Elem>;
      };
      template <typename Elem, typename Alloc>
      struct base_data_store<false, Elem, Alloc> {
         using type = managed_vector<Elem, Alloc>;
      };

      using base_data_store_t = typename base_data_store<Elems == std::numeric_limits<size_t>::max() && std::is_same_v<Allocator, nullptr_t>, ElemT, Allocator>::type;

      base_data_store_t _store;
      uint16_t          _index = 0;
   };

   using operand_stack = stack<operand_stack_elem>;
   using call_stack    = stack<activation_frame,   bounded_allocator, constants::max_call_depth + 1 >;

}} // namespace eosio::vm
