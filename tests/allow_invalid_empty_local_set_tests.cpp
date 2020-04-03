#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *  (func (local /illegal/))
 * )
 */
std::vector<uint8_t> bad_local_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x0a, 0x06, 0x01, 0x04, 0x01, 0x00,
   0x00, 0x0b
};

struct empty_options {};
struct static_options_false {
   static constexpr bool allow_invalid_empty_local_set = false;
};
struct static_options_true {
   static constexpr bool allow_invalid_empty_local_set = true;
};
struct dynamic_options {
   bool allow_invalid_empty_local_set;
};

}

BACKEND_TEST_CASE("Test allow_invalid_empty_local_set default", "[allow_invalid_empty_local_set_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   CHECK_THROWS_AS(backend_t(bad_local_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_invalid_empty_local_set empty", "[allow_invalid_empty_local_set_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   CHECK_THROWS_AS(backend_t(bad_local_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_invalid_empty_local_set static fail", "[allow_invalid_empty_local_set_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_false>;
   CHECK_THROWS_AS(backend_t(bad_local_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_invalid_empty_local_set static pass", "[allow_invalid_empty_local_set_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_true>;
   backend_t backend(bad_local_wasm, &wa);
}

BACKEND_TEST_CASE("Test allow_invalid_empty_local_set dynamic fail", "[allow_invalid_empty_local_set_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   CHECK_THROWS_AS(backend_t(bad_local_wasm, nullptr, dynamic_options{false}), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_invalid_empty_local_set dynamic pass", "[allow_invalid_empty_local_set_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend(bad_local_wasm, nullptr, dynamic_options{true});
}
