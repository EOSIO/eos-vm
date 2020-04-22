#include <eosio/vm/backend.hpp>

#include "utils.hpp"
#include <catch2/catch.hpp>

using namespace eosio::vm;

extern wasm_allocator wa;

namespace {

/*
 * (module
 *   (type (func))
 *   (type (func (result i32)))
 * )
 */
std::vector<uint8_t> types_2_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x02, 0x60,
   0x00, 0x00, 0x60, 0x00, 0x01, 0x7f
};

/*
 * (module
 *   (func (import "env" "f0"))
 *   (func (import "env" "f1"))
 * )
 */
std::vector<uint8_t> imports_2_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x02, 0x13, 0x02, 0x03, 0x65, 0x6e, 0x76, 0x02, 0x66, 0x30,
   0x00, 0x00, 0x03, 0x65, 0x6e, 0x76, 0x02, 0x66, 0x31, 0x00, 0x00
};

/*
 * (module
 *   (func)
 *   (func)
 * )
 */
std::vector<uint8_t> functions_2_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x03, 0x02, 0x00, 0x00, 0x0a, 0x07, 0x02, 0x02, 0x00,
   0x0b, 0x02, 0x00, 0x0b
};

/*
 * (module
 *   (global i32 (i32.const 0))
 *   (global i32 (i32.const 0))
 * )
 */
std::vector<uint8_t> globals_2_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x06, 0x0b, 0x02, 0x7f,
   0x00, 0x41, 0x00, 0x0b, 0x7f, 0x00, 0x41, 0x00, 0x0b
};

/*
 * (module
 *   (func (export "f1"))
 *   (func (export "f2"))
 * )
 */
std::vector<uint8_t> exports_2_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x03, 0x02, 0x00, 0x00, 0x07, 0x0b, 0x02, 0x02, 0x66,
   0x31, 0x00, 0x00, 0x02, 0x66, 0x32, 0x00, 0x01, 0x0a, 0x07, 0x02, 0x02,
   0x00, 0x0b, 0x02, 0x00, 0x0b
};

/*
 * (module
 *   (table 1 anyfunc)
 *   (elem (i32.const 0))
 *   (elem (i32.const 0))
 * )
 */
std::vector<uint8_t> elems_2_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x04, 0x04, 0x01, 0x70,
   0x00, 0x01, 0x09, 0x0b, 0x02, 0x00, 0x41, 0x00, 0x0b, 0x00, 0x00, 0x41,
   0x00, 0x0b, 0x00
};

/*
 * (module
 *   (memory 1)
 *   (data (i32.const 0) "")
 *   (data (i32.const 0) "")
 * )
 */
std::vector<uint8_t> data_2_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x05, 0x03, 0x01, 0x00,
   0x01, 0x0b, 0x0b, 0x02, 0x00, 0x41, 0x00, 0x0b, 0x00, 0x00, 0x41, 0x00,
   0x0b, 0x00
};

struct empty_options {};
struct dynamic_options {
   std::uint32_t max_section_elements;
};
struct static_options_1 {
   static constexpr std::uint32_t max_section_elements = 1;
};
struct static_options_2 {
   static constexpr std::uint32_t max_section_elements = 2;
};
}

#define SECTION_TEST_CASE_DEFS(name)                                      \
namespace {                                                               \
struct name ## _dynamic_options {                                         \
   std::uint32_t max_ ## name ## _section_elements;                       \
};                                                                        \
struct name ## _static_options_1 {                                        \
   static constexpr std::uint32_t max_ ## name ## _section_elements = 1;  \
};                                                                        \
struct name ## _static_options_2 {                                        \
   static constexpr std::uint32_t max_ ## name ## _section_elements = 2;  \
};                                                                        \
}

#define SECTION_TEST_CASE_IMPL(name, wasm)                                                                                   \
BACKEND_TEST_CASE("Test max_" #name "_section_elements", "[max_section_elements_test]") {                                    \
   backend<std::nullptr_t, TestType> backend_default(wasm, &wa);                                                             \
   backend<std::nullptr_t, TestType, empty_options> backend_empty(wasm, &wa);                                                \
   CHECK_THROWS_AS((backend<std::nullptr_t, TestType, static_options_1>(wasm, &wa)), wasm_parse_exception);                  \
   backend<std::nullptr_t, TestType, static_options_2> backend_static_fallback(wasm, &wa);                                   \
   CHECK_THROWS_AS((backend<std::nullptr_t, TestType, dynamic_options>(wasm, nullptr, {1})), wasm_parse_exception);          \
   backend<std::nullptr_t, TestType, dynamic_options> backend_dynamic_fallback(wasm, nullptr, {2});                          \
                                                                                                                             \
   CHECK_THROWS_AS((backend<std::nullptr_t, TestType, name ## _static_options_1>(wasm, &wa)), wasm_parse_exception);         \
   backend<std::nullptr_t, TestType, name ## _static_options_2> backend_static(wasm, &wa);                                   \
   CHECK_THROWS_AS((backend<std::nullptr_t, TestType, name ## _dynamic_options>(wasm, nullptr, {1})), wasm_parse_exception); \
   backend<std::nullptr_t, TestType, name ## _dynamic_options> backend_dynamic(wasm, nullptr, {2});                          \
}

#define SECTION_TEST_CASE(name, wasm)  \
   SECTION_TEST_CASE_DEFS(name)        \
   SECTION_TEST_CASE_IMPL(name, wasm)

SECTION_TEST_CASE(type, types_2_wasm)
SECTION_TEST_CASE(import, imports_2_wasm)
SECTION_TEST_CASE(function, functions_2_wasm)
SECTION_TEST_CASE(global, globals_2_wasm)
SECTION_TEST_CASE(export, exports_2_wasm)
SECTION_TEST_CASE(element, elems_2_wasm)
SECTION_TEST_CASE(data, data_2_wasm)
