#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *   (table 1024 funcref)
 * )
 */

std::vector<uint8_t> _1024_elements_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x04, 0x05, 0x01, 0x70,
   0x00, 0x80, 0x08
};
/*
 * (module
 *   (table 1025 funcref)
 * )
 */

std::vector<uint8_t> _1025_elements_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x04, 0x05, 0x01, 0x70,
   0x00, 0x81, 0x08
};

struct empty_options {};
struct dynamic_options {
   std::uint32_t max_table_elements;
};
struct static_options {
   static const std::uint32_t max_table_elements = 1024;
};

}

BACKEND_TEST_CASE("Test max_table_elements default", "[max_table_elements_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   backend_t backend1024(_1024_elements_wasm, &wa);
   backend_t backend1025(_1025_elements_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_table_elements static", "[max_table_elements_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options>;
   backend_t backend(_1024_elements_wasm, &wa);
   CHECK_THROWS_AS(backend_t(_1025_elements_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_table_elements unlimited", "[max_table_elements_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   backend_t backend1024(_1024_elements_wasm, &wa);
   backend_t backend1025(_1025_elements_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_table_elements dynamic", "[max_table_elements_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend1024(_1024_elements_wasm, nullptr, dynamic_options{1024});
   CHECK_THROWS_AS(backend_t(_1025_elements_wasm, nullptr, dynamic_options{1024}), wasm_parse_exception);
   backend_t backend1025(_1025_elements_wasm, nullptr, dynamic_options{1025});
}
