#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *   (memory 3 6)
 *   (func (export "f") (result i32)
 *     (i32.const 1)
 *     (memory.grow)
 *   )
 * )
 */
std::vector<uint8_t> mem_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x05, 0x01, 0x60,
   0x00, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x05, 0x04, 0x01, 0x01, 0x03,
   0x06, 0x07, 0x05, 0x01, 0x01, 0x66, 0x00, 0x00, 0x0a, 0x08, 0x01, 0x06,
   0x00, 0x41, 0x01, 0x40, 0x00, 0x0b
};

struct empty_options {};
struct static_options_2 {
   static constexpr std::uint32_t max_pages = 2;
};
struct static_options_3 {
   static constexpr std::uint32_t max_pages = 3;
};
struct dynamic_options {
   std::uint32_t max_pages;
};

}

BACKEND_TEST_CASE("Test max_pages default", "[max_pages_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   backend_t backend(mem_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_pages unlimited", "[max_pages_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   backend_t backend(mem_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_pages static fail", "[max_pages_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_2>;
   CHECK_THROWS_AS(backend_t(mem_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_pages static pass", "[max_pages_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_3>;
   backend_t backend(mem_wasm, &wa);
   CHECK(backend.call_with_return("env", "f")->to_i32() == -1);
}

BACKEND_TEST_CASE("Test max_pages dynamic fail", "[max_pages_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   CHECK_THROWS_AS(backend_t(mem_wasm, nullptr, dynamic_options{2}), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_pages dynamic pass", "[max_pages_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend(mem_wasm, &wa, dynamic_options{3});
   CHECK(backend.call_with_return("env", "f")->to_i32() == -1);
   backend.initialize(nullptr, dynamic_options{4});
   CHECK(backend.call_with_return("env", "f")->to_i32() == 3);
   backend.initialize(nullptr, dynamic_options{3});
   CHECK_THROWS_AS(backend.initialize(nullptr, dynamic_options{2}), std::exception);
}
