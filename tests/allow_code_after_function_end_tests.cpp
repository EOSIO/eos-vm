#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *  (memory 0)
 *  (global i32 (i32.const 0))
 *  (func (export "apply") (param i64 i64 i64))
 *  (block unreachable)
 *  (loop unreachable)
 *  (i32.const 0)
 *  (nop)
 *  (local.get 0)
 *  (global.get 0)
 *  (memory.size)
 * )
 */
std::vector<uint8_t> code_after_end_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01, 0x60,
   0x03, 0x7e, 0x7e, 0x7e, 0x00, 0x03, 0x02, 0x01, 0x00, 0x05, 0x03, 0x01,
   0x00, 0x00, 0x06, 0x06, 0x01, 0x7f, 0x00, 0x41, 0x00, 0x0b, 0x07, 0x09,
   0x01, 0x05, 0x61, 0x70, 0x70, 0x6c, 0x79, 0x00, 0x00, 0x0a, 0x15, 0x01,
   0x13, 0x00, 0x0b, 0x02, 0x40, 0x00, 0x0b, 0x03, 0x40, 0x00, 0x0b, 0x41,
   0x00, 0x01, 0x20, 0x00, 0x23, 0x00, 0x3f, 0x00
};

/*
 * (module
 *  (memory 0)
 *  (table 1 funcref)
 *  (elem (i32.const 0) 0)
 *  (global (mut i32) (i32.const 0))
 *  (func (export "apply") (param i64 i64 i64))
 *  ...
 * )
 */
std::vector<uint8_t> make_wasm(std::initializer_list<uint8_t> il) {
   std::vector<uint8_t> result = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01, 0x60,
      0x03, 0x7e, 0x7e, 0x7e, 0x00, 0x03, 0x02, 0x01, 0x00, 0x04, 0x04, 0x01,
      0x70, 0x00, 0x01, 0x05, 0x03, 0x01, 0x00, 0x00, 0x06, 0x06, 0x01, 0x7f,
      0x01, 0x41, 0x00, 0x0b, 0x07, 0x09, 0x01, 0x05, 0x61, 0x70, 0x70, 0x6c,
      0x79, 0x00, 0x00, 0x09, 0x07, 0x01, 0x00, 0x41, 0x00, 0x0b, 0x01, 0x00,
      0x0a
   };
   result.push_back(il.size() + 4);
   result.push_back(1);
   result.push_back(il.size() + 2);
   result.push_back(0);
   result.push_back(0x0b);
   result.insert(result.end(), il);
   return result;
}

std::vector<std::vector<uint8_t>> fail_wasms = {
   make_wasm({0x00}), // unreachable
   make_wasm({0x0b}), // end
   make_wasm({0x0f}), // return
   make_wasm({0x41, 0x00, 0x04, 0x40, 0x0b}), // if
   make_wasm({0x05}), // else
   make_wasm({0x0C, 0x00}), // br
   make_wasm({0x41, 0x00, 0x0D, 0x00}), // br_if
   make_wasm({0x41, 0x00, 0x0E, 0x00, 0x00}), // br_table
   make_wasm({0x42, 0x00, 0x42, 0x00, 0x42, 0x00, 0x10, 0x00}), // call
   make_wasm({0x42, 0x00, 0x42, 0x00, 0x42, 0x00, 0x41, 0x00, 0x11, 0x00, 0x00}), // call_indirect
   make_wasm({0x41, 0x00, 0x1A}), // drop
   make_wasm({0x41, 0x00, 0x41, 0x00, 0x41, 0x00, 0x1B}), // select
   make_wasm({0x42, 0x00, 0x21, 0x00}), // local.set
   make_wasm({0x42, 0x00, 0x22, 0x00}), // local.tee
   make_wasm({0x41, 0x00, 0x24, 0x00}), // global.set
   make_wasm({0x41, 0x00, 0x28, 0x00, 0x00}), // i32.load
   make_wasm({0x41, 0x00, 0x41, 0x41, 0x36, 0x00, 0x00}), // i32.store
   make_wasm({0x41, 0x00, 0x40, 0x00}), // memory.grow
   make_wasm({0x41, 0x00, 0x67}), // i32.clz
   make_wasm({0x41, 0x00, 0x41, 0x00, 0x6a}), // i32.add
   make_wasm({0x41, 0x00, 0xac}), // i64.extend_i32
   make_wasm({0x25}), // illegal instruction
   make_wasm({0xff}) // illegal instruction
};

struct empty_options {};
struct static_options_false {
   static constexpr bool allow_code_after_function_end = false;
};
struct static_options_true {
   static constexpr bool allow_code_after_function_end = true;
};
struct dynamic_options {
   bool allow_code_after_function_end;
};

}

BACKEND_TEST_CASE("Test allow_code_after_function_end default", "[allow_code_after_function_end_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   CHECK_THROWS_AS(backend_t(code_after_end_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_code_after_function_end unlimited", "[allow_code_after_function_end_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   CHECK_THROWS_AS(backend_t(code_after_end_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_code_after_function_end static fail", "[allow_code_after_function_end_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_false>;
   CHECK_THROWS_AS(backend_t(code_after_end_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_code_after_function_end static pass", "[allow_code_after_function_end_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_true>;
   backend_t backend(code_after_end_wasm, &wa);
   for(auto wasm : fail_wasms) {
      CHECK_THROWS_AS(backend_t(wasm, &wa), wasm_parse_exception);
   }
}

BACKEND_TEST_CASE("Test allow_code_after_function_end dynamic fail", "[allow_code_after_function_end_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   CHECK_THROWS_AS(backend_t(code_after_end_wasm, nullptr, dynamic_options{false}), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_code_after_function_end dynamic pass", "[allow_code_after_function_end_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend(code_after_end_wasm, nullptr, dynamic_options{true});
   for(auto wasm : fail_wasms) {
      CHECK_THROWS_AS(backend_t(wasm, nullptr, dynamic_options{true}), wasm_parse_exception);
   }
}
