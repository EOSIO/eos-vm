#pragma once

/*
 * definitions from https://github.com/WebAssembly/design/blob/master/BinaryEncoding.md
 */
#include <eosio/wasm_backend/utils.hpp>
#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/vector.hpp>
#include <eosio/wasm_backend/exceptions.hpp>

namespace eosio { namespace wasm_backend {
   class operand_stack {
      public:
         operand_stack() {
            _os.resize(8*1024);
         }
         void push(control c) {
            _os[_index++] = c;
         }
         control pop() {
            EOS_WB_ASSERT(_index > 0, wasm_interpreter_exception, "empty control stack");
            return _os[--_index];
         }
         uint8_t size()const {
            return _index;
         }
      private:
         native_vector<opcode>  _os;
         uint8_t                _index = 0;
   };
}} // namespace eosio::wasm_backend
