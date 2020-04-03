#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *   (memory 1)
 *   (data (i32.const 0) "abcdefghijkl")
 * )
 */

std::vector<uint8_t> full_12_data_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x05, 0x03, 0x01, 0x00,
   0x01, 0x0b, 0x12, 0x01, 0x00, 0x41, 0x00, 0x0b, 0x0c, 0x61, 0x62, 0x63,
   0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c
};

/*
 * (module
 *   (memory 1)
 *   (data (i32.const 12) "")
 * )
 */

std::vector<uint8_t> empty_12_data_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x05, 0x03, 0x01, 0x00,
   0x01, 0x0b, 0x06, 0x01, 0x00, 0x41, 0x0c, 0x0b, 0x00
};

/*
 * (module
 *   (memory 1)
 *   (data (i32.const 13) "")
 * )
 */

std::vector<uint8_t> empty_13_data_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x05, 0x03, 0x01, 0x00,
   0x01, 0x0b, 0x06, 0x01, 0x00, 0x41, 0x0d, 0x0b, 0x00
};

/*
 * (module
 *   (memory 1)
 *   (data (i32.const 11) "a")
 * )
 */

std::vector<uint8_t> one_11_data_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x05, 0x03, 0x01, 0x00,
   0x01, 0x0b, 0x07, 0x01, 0x00, 0x41, 0x0b, 0x0b, 0x01, 0x61
};

/*
 * (module
 *   (memory 1)
 *   (data (i32.const 12) "a")
 * )
 */

std::vector<uint8_t> one_12_data_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x05, 0x03, 0x01, 0x00,
   0x01, 0x0b, 0x07, 0x01, 0x00, 0x41, 0x0c, 0x0b, 0x01, 0x61
};

/*
 * (module
 *   (memory 1)
 *   (data (i32.const 0xFFFFFFFF) "ab")
 * )
 */

std::vector<uint8_t> wrap_data_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x05, 0x03, 0x01, 0x00,
   0x01, 0x0b, 0x08, 0x01, 0x00, 0x41, 0x7f, 0x0b, 0x02, 0x61, 0x62
};

struct empty_options {};
struct dynamic_options {
   std::uint32_t max_linear_memory_init;
};
struct static_options {
   static constexpr std::uint32_t max_linear_memory_init = 12;
};

}

BACKEND_TEST_CASE("Test max_linear_memory_init default", "[max_linear_memory_init_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   backend_t backendfull12(full_12_data_wasm, &wa);
   backend_t backendempty12(empty_12_data_wasm, &wa);
   backend_t backendempty13(empty_13_data_wasm, &wa);
   backend_t backendone11(one_11_data_wasm, &wa);
   backend_t backendone12(one_12_data_wasm, &wa);
   // This is well formed but should fail linking
   CHECK_THROWS_AS(backend_t(wrap_data_wasm, &wa), wasm_memory_exception);
}

BACKEND_TEST_CASE("Test max_linear_memory_init static", "[max_linear_memory_init_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options>;
   backend_t backendfull12(full_12_data_wasm, &wa);
   backend_t backendempty12(empty_12_data_wasm, &wa);
   CHECK_THROWS_AS(backend_t(empty_13_data_wasm, &wa), wasm_parse_exception);
   backend_t backendone11(one_11_data_wasm, &wa);
   CHECK_THROWS_AS(backend_t(one_12_data_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(wrap_data_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_linear_memory_init unlimited", "[max_linear_memory_init_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   backend_t backendfull12(full_12_data_wasm, &wa);
   backend_t backendempty12(empty_12_data_wasm, &wa);
   backend_t backendempty13(empty_13_data_wasm, &wa);
   backend_t backendone11(one_11_data_wasm, &wa);
   backend_t backendone12(one_12_data_wasm, &wa);
   CHECK_THROWS_AS(backend_t(wrap_data_wasm, &wa), wasm_memory_exception);
}

BACKEND_TEST_CASE("Test max_linear_memory_init dynamic", "[max_linear_memory_init_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backendfull12(full_12_data_wasm, nullptr, dynamic_options{12});
   backend_t backendempty12(empty_12_data_wasm, nullptr, dynamic_options{12});
   CHECK_THROWS_AS(backend_t(empty_13_data_wasm, nullptr, dynamic_options{12}), wasm_parse_exception);
   backend_t backendone11(one_11_data_wasm, nullptr, dynamic_options{12});
   CHECK_THROWS_AS(backend_t(one_12_data_wasm, nullptr, dynamic_options{12}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(wrap_data_wasm, nullptr, dynamic_options{12}), wasm_parse_exception);
}
