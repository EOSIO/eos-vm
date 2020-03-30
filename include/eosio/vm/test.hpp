#pragma once
#include <eosio/vm/opcodes.hpp>

namespace eosio { namespace vm { namespace test {
   template <typename... Instructions>
   struct function {
      function(const std::string& name) : name(name) {}
      std::string name;
   };

   // simple utility for testing
   struct wasm_generator {
   };
}}} // ns eosio::vm::test
