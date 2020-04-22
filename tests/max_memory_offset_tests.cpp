#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *   (memory 1)
 *   (func (i32.const 0) (i32.load offset=0xFFFFFFFF) drop)
 * )
 */
std::vector<uint8_t> max_load_offset_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x05, 0x03, 0x01, 0x00, 0x01, 0x0a,
   0x0e, 0x01, 0x0c, 0x00, 0x41, 0x00, 0x28, 0x02, 0xff, 0xff, 0xff, 0xff,
   0x0f, 0x1a, 0x0b
};

/*
 * (module
 *   (memory 1)
 *   (func (i32.const 0) (i32.const 0) (i32.store offset=0xFFFFFFFF))
 * )
 */
std::vector<uint8_t> max_store_offset_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x05, 0x03, 0x01, 0x00, 0x01, 0x0a,
   0x0f, 0x01, 0x0d, 0x00, 0x41, 0x00, 0x41, 0x00, 0x36, 0x02, 0xff, 0xff,
   0xff, 0xff, 0x0f, 0x0b
};

/*
 * (module
 *   (memory 1)
 *   (func (i32.const 0) (i32.load offset=2) drop)
 * )
 */
std::vector<uint8_t> load_offset_2_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x05, 0x03, 0x01, 0x00, 0x01, 0x0a,
   0x0a, 0x01, 0x08, 0x00, 0x41, 0x00, 0x28, 0x02, 0x02, 0x1a, 0x0b
};

/*
 * (module
 *   (memory 1)
 *   (func (i32.const 0) (i32.const 0) (i32.store offset=2))
 * )
 */
std::vector<uint8_t> store_offset_2_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x05, 0x03, 0x01, 0x00, 0x01, 0x0a,
   0x0b, 0x01, 0x09, 0x00, 0x41, 0x00, 0x41, 0x00, 0x36, 0x02, 0x02, 0x0b
};

struct empty_options {};
struct static_options_1 {
   static constexpr std::uint32_t max_memory_offset = 1;
};
struct static_options_2 {
   static constexpr std::uint32_t max_memory_offset = 2;
};
struct dynamic_options {
   std::uint32_t max_memory_offset;
};

}

BACKEND_TEST_CASE("Test max_memory_offset default", "[max_memory_offset_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   backend_t backend_load_max(max_load_offset_wasm, &wa);
   backend_t backend_store_max(max_store_offset_wasm, &wa);
   backend_t backend_load_2(load_offset_2_wasm, &wa);
   backend_t backend_store_2(load_offset_2_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_memory_offset unlimited", "[max_memory_offset_test]") {
   using backend_t = backend<std::nullptr_t, TestType, empty_options>;
   backend_t backend_load_max(max_load_offset_wasm, &wa);
   backend_t backend_store_max(max_store_offset_wasm, &wa);
   backend_t backend_load_2(load_offset_2_wasm, &wa);
   backend_t backend_store_2(load_offset_2_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_memory_offset static fail", "[max_memory_offset_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_1>;
   CHECK_THROWS_AS(backend_t(max_load_offset_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(max_store_offset_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(load_offset_2_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(store_offset_2_wasm, &wa), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_memory_offset static pass", "[max_memory_offset_test]") {
   using backend_t = backend<std::nullptr_t, TestType, static_options_2>;
   CHECK_THROWS_AS(backend_t(max_load_offset_wasm, &wa), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(max_store_offset_wasm, &wa), wasm_parse_exception);
   backend_t backend_load_2(load_offset_2_wasm, &wa);
   backend_t backend_store_2(store_offset_2_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_memory_offset dynamic fail", "[max_memory_offset_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   CHECK_THROWS_AS(backend_t(max_load_offset_wasm, nullptr, dynamic_options{1}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(max_store_offset_wasm, nullptr, dynamic_options{1}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(load_offset_2_wasm, nullptr, dynamic_options{1}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(store_offset_2_wasm, nullptr, dynamic_options{1}), wasm_parse_exception);
}

BACKEND_TEST_CASE("Test max_memory_offset dynamic pass", "[max_memory_offset_test]") {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   CHECK_THROWS_AS(backend_t(max_load_offset_wasm, nullptr, dynamic_options{2}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(max_store_offset_wasm, nullptr, dynamic_options{2}), wasm_parse_exception);
   backend_t backend_load_2(load_offset_2_wasm, nullptr, dynamic_options{2});
   backend_t backend_store_2(store_offset_2_wasm, nullptr, dynamic_options{2});
}
