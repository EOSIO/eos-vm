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
 *   (func (param i64 i64))
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
 *  (func
 *   (i64.const 0)
 *   (i64.const 0)
 *   (drop)
 *   (i32.const 0)
 *   (i32.const 0)
 *   (drop)
 *   (drop)
 *   (drop)
 *   (return)
 *   (block
 *    (i64.const 0)
 *    (i64.const 0)
 *    (i64.const 0)
 *    (return)
 *   )
 *  )
 * )
 */

std::vector<uint8_t> stack_16_wasm = {
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
   0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x0a, 0x1b, 0x01, 0x19, 0x00, 0x42,
   0x00, 0x42, 0x00, 0x1a, 0x41, 0x00, 0x41, 0x00, 0x1a, 0x1a, 0x1a, 0x0f,
   0x02, 0x40, 0x42, 0x00, 0x42, 0x00, 0x42, 0x00, 0x0f, 0x0b, 0x0b
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

constexpr auto lp = max_func_local_bytes_flags_t::params | max_func_local_bytes_flags_t::locals;
constexpr auto ps = max_func_local_bytes_flags_t::params | max_func_local_bytes_flags_t::stack;
constexpr auto ls = max_func_local_bytes_flags_t::locals | max_func_local_bytes_flags_t::stack;
constexpr auto lps = max_func_local_bytes_flags_t::params | max_func_local_bytes_flags_t::locals | max_func_local_bytes_flags_t::stack;

struct empty_options {};
template<max_func_local_bytes_flags_t F>
struct empty_options_static_flags {
   static constexpr auto max_func_local_bytes_flags = F;
};
struct empty_options_dynamic_flags {
   max_func_local_bytes_flags_t max_func_local_bytes_flags;
};
template<int N, max_func_local_bytes_flags_t F>
struct static_options {
   static constexpr std::uint32_t max_func_local_bytes = N;
   static constexpr auto max_func_local_bytes_flags = F;
};
template<int N>
struct static_options_empty_flags {
   static constexpr std::uint32_t max_func_local_bytes = N;
};
struct dynamic_options {
   std::uint32_t max_func_local_bytes;
   max_func_local_bytes_flags_t max_func_local_bytes_flags;
};
struct dynamic_options_empty_flags {
   std::uint32_t max_func_local_bytes;
};

}

BACKEND_TEST_CASE("Test max_func_local_bytes default", "[max_func_local_bytes_test]") {
   using backend_t = backend<std::nullptr_t, TestType>;
   backend_t backend_unused(unused_type_wasm, &wa);
   backend_t backend_param(param_16_wasm, &wa);
   backend_t backend_local(local_16_wasm, &wa);
   backend_t backend_stack(stack_16_wasm, &wa);
   backend_t backend_mixed(mixed_16_wasm, &wa);
}

BACKEND_TEST_CASE("Test max_func_local_bytes unlimited", "[max_func_local_bytes_test]") {
   {
      using backend_t = backend<std::nullptr_t, TestType, empty_options>;
      backend_t backend_unused(unused_type_wasm, &wa);
      backend_t backend_param(param_16_wasm, &wa);
      backend_t backend_local(local_16_wasm, &wa);
      backend_t backend_stack(stack_16_wasm, &wa);
      backend_t backend_mixed(mixed_16_wasm, &wa);
   }
   {
      using backend_t = backend<std::nullptr_t, TestType, empty_options_static_flags<lp>>;
      backend_t backend_unused(unused_type_wasm, &wa);
      backend_t backend_param(param_16_wasm, &wa);
      backend_t backend_local(local_16_wasm, &wa);
      backend_t backend_stack(stack_16_wasm, &wa);
      backend_t backend_mixed(mixed_16_wasm, &wa);
   }
   {
      using backend_t = backend<std::nullptr_t, TestType, empty_options_static_flags<ps>>;
      backend_t backend_unused(unused_type_wasm, &wa);
      backend_t backend_param(param_16_wasm, &wa);
      backend_t backend_local(local_16_wasm, &wa);
      backend_t backend_stack(stack_16_wasm, &wa);
      backend_t backend_mixed(mixed_16_wasm, &wa);
   }
   {
      using backend_t = backend<std::nullptr_t, TestType, empty_options_static_flags<ls>>;
      backend_t backend_unused(unused_type_wasm, &wa);
      backend_t backend_param(param_16_wasm, &wa);
      backend_t backend_local(local_16_wasm, &wa);
      backend_t backend_stack(stack_16_wasm, &wa);
      backend_t backend_mixed(mixed_16_wasm, &wa);
   }
   {
      using backend_t = backend<std::nullptr_t, TestType, empty_options_dynamic_flags>;
      backend_t backend_unused(unused_type_wasm, nullptr, empty_options_dynamic_flags{lps});
      backend_t backend_param(param_16_wasm, nullptr, empty_options_dynamic_flags{lps});
      backend_t backend_local(local_16_wasm, nullptr, empty_options_dynamic_flags{lps});
      backend_t backend_stack(stack_16_wasm, nullptr, empty_options_dynamic_flags{lps});
      backend_t backend_mixed(mixed_16_wasm, nullptr, empty_options_dynamic_flags{lps});
   }
}

BACKEND_TEST_CASE("Test max_func_local_bytes static fail", "[max_func_local_bytes_test]") {
   {
      using backend_t = backend<std::nullptr_t, TestType, static_options<8, lp>>;
      backend_t backend_unused(unused_type_wasm, &wa);
      CHECK_THROWS_AS(backend_t(param_16_wasm, &wa), wasm_parse_exception);
      CHECK_THROWS_AS(backend_t(local_16_wasm, &wa), wasm_parse_exception);
      backend_t backend_stack(stack_16_wasm, &wa);
      CHECK_THROWS_AS(backend_t(mixed_16_wasm, &wa), wasm_parse_exception);
   }
   {
      using backend_t = backend<std::nullptr_t, TestType, static_options<8, ls>>;
      backend_t backend_param(param_16_wasm, &wa);
      CHECK_THROWS_AS(backend_t(local_16_wasm, &wa), wasm_parse_exception);
      CHECK_THROWS_AS(backend_t(stack_16_wasm, &wa), wasm_parse_exception);
   }
   {
      using backend_t = backend<std::nullptr_t, TestType, static_options<8, ps>>;
      CHECK_THROWS_AS(backend_t(param_16_wasm, &wa), wasm_parse_exception);
      backend_t backend_local(local_16_wasm, &wa);
      CHECK_THROWS_AS(backend_t(stack_16_wasm, &wa), wasm_parse_exception);
   }
   {
      using backend_t = backend<std::nullptr_t, TestType, static_options_empty_flags<8>>;
      backend_t backend_param(param_16_wasm, &wa);
      CHECK_THROWS_AS(backend_t(local_16_wasm, &wa), wasm_parse_exception);
      CHECK_THROWS_AS(backend_t(stack_16_wasm, &wa), wasm_parse_exception);
   }
}

BACKEND_TEST_CASE("Test max_func_local_bytes static pass", "[max_func_local_bytes_test]") {
   {
      using backend_t = backend<std::nullptr_t, TestType, static_options<16, lp>>;
      backend_t backend_unused(unused_type_wasm, &wa);
      backend_t backend_param(param_16_wasm, &wa);
      backend_t backend_local(local_16_wasm, &wa);
      backend_t backend_stack(stack_16_wasm, &wa);
      backend_t backend_mixed(mixed_16_wasm, &wa);
   }
   {
      using backend_t = backend<std::nullptr_t, TestType, static_options<16, ps>>;
      backend_t backend_param(param_16_wasm, &wa);
      backend_t backend_local(local_16_wasm, &wa);
      backend_t backend_stack(stack_16_wasm, &wa);
   }
   {
      using backend_t = backend<std::nullptr_t, TestType, static_options<16, ls>>;
      backend_t backend_param(param_16_wasm, &wa);
      backend_t backend_local(local_16_wasm, &wa);
      backend_t backend_stack(stack_16_wasm, &wa);
   }
   {
      using backend_t = backend<std::nullptr_t, TestType, static_options_empty_flags<16>>;
      backend_t backend_param(param_16_wasm, &wa);
      backend_t backend_local(local_16_wasm, &wa);
      backend_t backend_stack(stack_16_wasm, &wa);
   }
}

BACKEND_TEST_CASE("Test max_func_local_bytes dynamic fail", "[max_func_local_bytes_test]") {
   {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   CHECK_THROWS_AS(backend_t(param_16_wasm, nullptr, dynamic_options{8, lp}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(param_16_wasm, nullptr, dynamic_options{8, ps}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(local_16_wasm, nullptr, dynamic_options{8, lp}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(local_16_wasm, nullptr, dynamic_options{8, ls}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(stack_16_wasm, nullptr, dynamic_options{8, ls}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(stack_16_wasm, nullptr, dynamic_options{8, ps}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(mixed_16_wasm, nullptr, dynamic_options{8, lp}), wasm_parse_exception);
   }

   {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options_empty_flags>;
   CHECK_THROWS_AS(backend_t(local_16_wasm, nullptr, dynamic_options_empty_flags{8}), wasm_parse_exception);
   CHECK_THROWS_AS(backend_t(stack_16_wasm, nullptr, dynamic_options_empty_flags{8}), wasm_parse_exception);
   }
}

BACKEND_TEST_CASE("Test max_func_local_bytes dynamic pass", "[max_func_local_bytes_test]") {
   {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options>;
   backend_t backend_unused(unused_type_wasm, nullptr, dynamic_options{8, lp});

   backend_t backend_param1(param_16_wasm, nullptr, dynamic_options{16, lp});
   backend_t backend_param2(param_16_wasm, nullptr, dynamic_options{8, ls});
   backend_t backend_param3(param_16_wasm, nullptr, dynamic_options{16, ps});
   backend_t backend_local1(local_16_wasm, nullptr, dynamic_options{16, lp});
   backend_t backend_local2(local_16_wasm, nullptr, dynamic_options{16, ls});
   backend_t backend_local3(local_16_wasm, nullptr, dynamic_options{8, ps});
   backend_t backend_stack1(stack_16_wasm, nullptr, dynamic_options{8, lp});
   backend_t backend_stack3(stack_16_wasm, nullptr, dynamic_options{16, ps});

   backend_t backend_mixed(mixed_16_wasm, nullptr, dynamic_options{16, lp});
   }
   {
   using backend_t = backend<std::nullptr_t, TestType, dynamic_options_empty_flags>;
   backend_t backend_param2(param_16_wasm, nullptr, dynamic_options_empty_flags{8});
   backend_t backend_local2(local_16_wasm, nullptr, dynamic_options_empty_flags{16});
   backend_t backend_stack2(stack_16_wasm, nullptr, dynamic_options_empty_flags{16});
   }
}
