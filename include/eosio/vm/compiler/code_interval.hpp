#pragma once

#include <eosio/vm/types.hpp>
#include <eosio/vm/opcodes.hpp>
#include <eosio/vm/vector.hpp>
#include <iostream>

namespace eosio { namespace vm {

   // class makes the assumption that all opcodes are held in contiguous memory
   // this acts as a "view" over opcodes
   class code_interval {
      public:
         class iterator {
            public:
               iterator(opcode* ptr) noexcept : _ptr(ptr) {}
               const opcode* operator*()const { return _ptr; }
               opcode* operator*() { return _ptr; }
               opcode& operator&() { return *_ptr; }
               // prefix operator
               iterator& operator++() {
                  _ptr++;
                  return *this;
               }
               // postfix operator
               iterator operator++(int) {
                  iterator iter = *this;
                  ++_ptr;
                  return iter;
               }
               // prefix operator
               iterator& operator--() {
                  _ptr--;
                  return *this;
               }
               // postfix operator
               iterator operator--(int) {
                  iterator iter = *this;
                  --_ptr;
                  return iter;
               }

               iterator next()const {
                  iterator iter = *this;
                  return ++iter;
               }
               bool operator!=(const iterator& iter)const {
                  return iter._ptr != _ptr;
               }
               bool operator==(const iterator& iter)const {
                  return iter._ptr == _ptr;
               }
            private:
               opcode* _ptr;
         };

         code_interval(opcode* begin, opcode* end) : _begin(begin), _end(end), _size((end-begin)+1) {}
         unmanaged_vector<opcode> to_vector()const {
            unmanaged_vector<opcode> ret_vec;
            for (opcode* ptr = _begin; ptr != _end; ptr++)
               ret_vec.push_back(*ptr);
            return ret_vec;
         }
         inline bool operator==(const code_interval& ci)const { return std::tie(_begin, _end) == std::tie(ci._begin, ci._end); }
         inline bool operator!=(const code_interval& ci)const { return !(*this == ci); }

         iterator begin() { return _begin; }
         const opcode* cbegin()const { return _begin; }

         iterator end() { return _end + 1; /* one past the end */ }
         const opcode* cend()const { return _end + 1; /* one past the end */ }

         opcode* first() { return _begin; }
         const opcode* first()const { return _begin; }

         opcode* last() { return _end; }
         const opcode* last()const { return _end; }

         opcode* at(std::size_t index) {
            EOS_VM_ASSERT( (_begin + index) <= _end, wasm_compilation_exception, "trying to access opcode outside of code interval" );
            return _begin + index;
         }
         const opcode* at(std::size_t index)const {
            EOS_VM_ASSERT( (_begin + index) <= _end, wasm_compilation_exception, "trying to access opcode outside of code interval" );
            return _begin + index;
         }
         
         bool contains(const opcode* op)const { return op >= _begin && op <= _end; }
         opcode* operator[](std::size_t index) { return at(index); }
         const opcode* operator[](std::size_t index)const { return at(index); }
         
         std::size_t size()const { return _size; }
      private:
         opcode* _begin;
         opcode* _end;
         std::size_t _size;
   };

}} // namespace eosio::vm
