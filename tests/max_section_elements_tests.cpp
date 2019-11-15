#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *   (type (func))
 * )
 */

std::vector<uint8_t> _1_element_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00
};
/*
 * (module
 *   (type (func))
 *   (type (func (result i32)))
 * )
 */

std::vector<uint8_t> _2_elements_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x02, 0x60,
   0x00, 0x00, 0x60, 0x00, 0x01, 0x7f
};

struct empty_options {};
struct dynamic_options {
   std::uint32_t max_section_elements;
};
struct static_options {
   static constexpr std::uint32_t max_section_elements = 1;
};

}

BACKEND_TEST_CASE("Test max_section_elements default", "[max_section_elements_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   backend_t backend1024(_1_element_wasm);
   backend_t backend1025(_2_elements_wasm);
}

BACKEND_TEST_CASE("Test max_section_elements static", "[max_section_elements_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options>;
   backend_t backend(_1_element_wasm);
   CHECK_THROWS_AS(backend_t(_2_elements_wasm), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_section_elements unlimited", "[max_section_elements_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   backend_t backend1024(_1_element_wasm);
   backend_t backend1025(_2_elements_wasm);
}

BACKEND_TEST_CASE("Test max_section_elements dynamic", "[max_table_elements_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend1024(_1_element_wasm, nullptr, dynamic_options{1});
   CHECK_THROWS_AS(backend_t(_2_elements_wasm, nullptr, dynamic_options{1}), wasm_parse_exception);
   backend_t backend1025(_2_elements_wasm, nullptr, dynamic_options{2});
}
