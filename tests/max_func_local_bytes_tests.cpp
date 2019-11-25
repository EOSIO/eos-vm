#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *   (type (func (param i64 i64)))
 * )
 */
std::vector<uint8_t> unused_type_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x06, 0x01, 0x60,
   0x02, 0x7e, 0x7e, 0x00
};
  
/*
 * (module
 *   (table 1025 funcref)
 * )
 */

std::vector<uint8_t> param_16_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x06, 0x01, 0x60,
   0x02, 0x7e, 0x7e, 0x00, 0x03, 0x02, 0x01, 0x00, 0x0a, 0x04, 0x01, 0x02,
   0x00, 0x0b
};
  
/*
 * (module
 *   (func (local i64 i64))
 * )
 */

std::vector<uint8_t> local_16_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x0a, 0x06, 0x01, 0x04, 0x01, 0x02,
   0x7e, 0x0b
};
  
/*
 * (module
 *   (func (param i64) (local i64))
 * )
 */

std::vector<uint8_t> mixed_16_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x05, 0x01, 0x60,
   0x01, 0x7e, 0x00, 0x03, 0x02, 0x01, 0x00, 0x0a, 0x06, 0x01, 0x04, 0x01,
   0x01, 0x7e, 0x0b
};

struct empty_options {};
struct static_options_8 {
   static constexpr std::uint32_t max_func_local_bytes = 8;
};
struct static_options_16 {
   static constexpr std::uint32_t max_func_local_bytes = 16;
};
struct dynamic_options {
   std::uint32_t max_func_local_bytes;
};

}

BACKEND_TEST_CASE("Test max_func_local_bytes default", "[max_func_local_bytes_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   backend_t backend_unused(unused_type_wasm);
   backend_t backend_param(param_16_wasm);
   backend_t backend_local(local_16_wasm);
   backend_t backend_mixed(mixed_16_wasm);
}

BACKEND_TEST_CASE("Test max_func_local_bytes unlimited", "[max_func_local_bytes_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   backend_t backend_unused(unused_type_wasm);
   backend_t backend_param(param_16_wasm);
   backend_t backend_local(local_16_wasm);
   backend_t backend_mixed(mixed_16_wasm);
}

BACKEND_TEST_CASE("Test max_func_local_bytes static fail", "[max_func_local_bytes_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_8>;
   backend_t backend_unused(unused_type_wasm);
   CHECK_THROWS_AS(backend_t(param_16_wasm), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(local_16_wasm), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(mixed_16_wasm), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_func_local_bytes static pass", "[max_func_local_bytes_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_16>;
   backend_t backend_unused(unused_type_wasm);
   backend_t backend_param(param_16_wasm);
   backend_t backend_local(local_16_wasm);
   backend_t backend_mixed(mixed_16_wasm);
}

BACKEND_TEST_CASE("Test max_func_local_bytes dynamic fail", "[max_func_local_bytes_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend_unused(unused_type_wasm, nullptr, dynamic_options{8});
   CHECK_THROWS_AS(backend_t(param_16_wasm, nullptr, dynamic_options{8}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(local_16_wasm, nullptr, dynamic_options{8}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(mixed_16_wasm, nullptr, dynamic_options{8}), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_func_local_bytes dynamic pass", "[max_func_local_bytes_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend_unused(unused_type_wasm, nullptr, dynamic_options{16});
   backend_t backend_param(param_16_wasm, nullptr, dynamic_options{16});
   backend_t backend_local(local_16_wasm, nullptr, dynamic_options{16});
   backend_t backend_mixed(mixed_16_wasm, nullptr, dynamic_options{16});
}
