#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

// All of these wasms use 0x3FFE/0x3FFF instead of 0x00/0x01 for the flags

/*
 * (module
 *  (memory 1)
 * )
 */
std::vector<uint8_t> mem_no_max_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x05, 0x04, 0x01, 0xFE,
   0x7F, 0x01
};

/*
 * (module
 *  (memory 1 2)
 * )
 */
std::vector<uint8_t> mem_max_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x05, 0x05, 0x01, 0xFF,
   0x7F, 0x01, 0x02
};

/*
 * (module
 *  (table 1)
 * )
 */
std::vector<uint8_t> table_no_max_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x04, 0x05, 0x01, 0x70,
   0xFE, 0x7F, 0x01
};

/*
 * (module
 *  (table 1 2)
 * )
 */
std::vector<uint8_t> table_max_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x04, 0x06, 0x01, 0x70,
   0xFF, 0x7F, 0x01, 0x02
};

struct empty_options {};
struct static_options_false {
   static constexpr bool allow_u32_limits_flags = false;
};
struct static_options_true {
   static constexpr bool allow_u32_limits_flags = true;
};
struct dynamic_options {
   bool allow_u32_limits_flags;
};

}

BACKEND_TEST_CASE("Test allow_u32_limits_flags default", "[allow_u32_limits_flags_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   CHECK_THROWS_AS(backend_t(mem_no_max_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(mem_max_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(table_no_max_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(table_max_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_u32_limits_flags empty", "[allow_u32_limits_flags_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   CHECK_THROWS_AS(backend_t(mem_no_max_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(mem_max_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(table_no_max_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(table_max_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_u32_limits_flags static fail", "[allow_u32_limits_flags_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_false>;
   CHECK_THROWS_AS(backend_t(mem_no_max_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(mem_max_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(table_no_max_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(table_max_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_u32_limits_flags static pass", "[allow_u32_limits_flags_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_true>;
   backend_t backend_mem_no_max(mem_no_max_wasm, &wa);
   backend_t backend_mem_max(mem_max_wasm, &wa);
   backend_t backend_table_no_max(table_no_max_wasm, &wa);
   backend_t backend_table_max(table_max_wasm, &wa);
}

BACKEND_TEST_CASE("Test allow_u32_limits_flags dynamic fail", "[allow_u32_limits_flags_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   CHECK_THROWS_AS(backend_t(mem_max_wasm, nullptr, dynamic_options{false}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(mem_no_max_wasm, nullptr, dynamic_options{false}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(table_max_wasm, nullptr, dynamic_options{false}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(table_no_max_wasm, nullptr, dynamic_options{false}), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test allow_u32_limits_flags dynamic pass", "[allow_u32_limits_flags_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend_mem_max(mem_max_wasm, nullptr, dynamic_options{true});
   backend_t backend_mem_no_max(mem_no_max_wasm, nullptr, dynamic_options{true});
   backend_t backend_table_max(table_max_wasm, nullptr, dynamic_options{true});
   backend_t backend_table_no_max(table_no_max_wasm, nullptr, dynamic_options{true});
}
