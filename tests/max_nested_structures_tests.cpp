#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *   (func (block)
 * )
 */
std::vector<uint8_t> depth_2_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x0a, 0x07, 0x01, 0x05, 0x00, 0x02,
   0x40, 0x0b, 0x0b
};

struct empty_options {};
struct static_options_1 {
   static constexpr std::uint32_t max_nested_structures = 1;
};
struct static_options_2 {
   static constexpr std::uint32_t max_nested_structures = 2;
};
struct dynamic_options {
   std::uint32_t max_nested_structures;
};

}

BACKEND_TEST_CASE("Test max_nested_structures default", "[max_nested_structures_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   backend_t backend(depth_2_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_nested_structures unlimited", "[max_nested_structures_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   backend_t backend(depth_2_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_nested_structures static fail", "[max_nested_structures_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_1>;
   CHECK_THROWS_AS(backend_t(depth_2_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_nested_structures static pass", "[max_nested_structures_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_2>;
   backend_t backend(depth_2_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_nested_structures dynamic fail", "[max_nested_structures_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   CHECK_THROWS_AS(backend_t(depth_2_wasm, nullptr, dynamic_options{1}), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_nested_structures dynamic pass", "[max_nested_structures_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend(depth_2_wasm, nullptr, dynamic_options{2});
}
