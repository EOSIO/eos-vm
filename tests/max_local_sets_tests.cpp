#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *   (func (local i32 i64))
 * )
 */
std::vector<uint8_t> two_sets_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x0a, 0x08, 0x01, 0x06, 0x02, 0x01,
   0x7f, 0x01, 0x7e, 0x0b
};

struct empty_options {};
struct static_options_1 {
   static constexpr std::uint32_t max_local_sets = 1;
};
struct static_options_2 {
   static constexpr std::uint32_t max_local_sets = 2;
};
struct dynamic_options {
   std::uint32_t max_local_sets;
};

}

BACKEND_TEST_CASE("Test max_local_sets default", "[max_local_sets_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   backend_t backend(two_sets_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_local_sets unlimited", "[max_local_sets_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   backend_t backend(two_sets_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_local_sets static fail", "[max_local_sets_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_1>;
   CHECK_THROWS_AS(backend_t(two_sets_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_local_sets static pass", "[max_local_sets_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_2>;
   backend_t backend(two_sets_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_local_sets dynamic fail", "[max_local_sets_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   CHECK_THROWS_AS(backend_t(two_sets_wasm, nullptr, dynamic_options{1}), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_local_sets dynamic pass", "[max_local_sets_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend(two_sets_wasm, nullptr, dynamic_options{2});
}
