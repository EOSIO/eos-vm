#pragma once

/*
 * definitions from https://github.com/WebAssembly/design/blob/master/BinaryEncoding.md
 */

#include <string>
#include <eosio/wasm_backend/utils.hpp>
#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/vector.hpp>
#include <eosio/wasm_backend/exceptions.hpp>

namespace eosio { namespace wasm_backend {
   enum ctrl_type : uint8_t {
      block_c = 0,
      loop_c  = 1,
      if_c    = 2,
      else_c  = 4
   };
   
   struct control {
      uint8_t ctrl;
      uint8_t ret;
   };

   class control_stack {
      public:
         control_stack() {
            _cs.resize(1024);
         }
         void push(control c) {
            _cs[_index++] = c;
         }
         control pop() {
            EOS_WB_ASSERT(_index > 0, wasm_interpreter_exception, "empty control stack");
            return _cs[--_index];
         }
         uint8_t size()const {
            return _index;
         }
      private:
         native_vector<control> _cs;
         uint8_t                _index = 0;
   };
}} // namespace eosio::wasm_backend
