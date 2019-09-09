#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>
#include <cmath>
#include <cstdlib>
#include <catch2/catch.hpp>
#include <utils.hpp>
#include <wasm_config.hpp>
#include <eosio/vm/backend.hpp>

using namespace eosio;
using namespace eosio::vm;
extern wasm_allocator wa;

BACKEND_TEST_CASE( "Testing wasm <if_0_wasm>", "[if_0_wasm_tests]" ) {
   using backend_t = backend<std::nullptr_t, TestType>;
   auto code = backend_t::read_wasm( std::string(wasm_directory) + "if.0.wasm");
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );
   bkend.initialize(nullptr);

   CHECK(!bkend.call_with_return(nullptr, "env", "empty", UINT32_C(0)));
   CHECK(!bkend.call_with_return(nullptr, "env", "empty", UINT32_C(1)));
   CHECK(!bkend.call_with_return(nullptr, "env", "empty", UINT32_C(100)));
   CHECK(!bkend.call_with_return(nullptr, "env", "empty", UINT32_C(4294967294)));
   CHECK(bkend.call_with_return(nullptr, "env", "singular", UINT32_C(0))->to_ui32() == UINT32_C(8));
   CHECK(bkend.call_with_return(nullptr, "env", "singular", UINT32_C(1))->to_ui32() == UINT32_C(7));
   CHECK(bkend.call_with_return(nullptr, "env", "singular", UINT32_C(10))->to_ui32() == UINT32_C(7));
   CHECK(bkend.call_with_return(nullptr, "env", "singular", UINT32_C(4294967286))->to_ui32() == UINT32_C(7));
   CHECK(bkend.call_with_return(nullptr, "env", "multi", UINT32_C(0))->to_ui32() == UINT32_C(9));
   CHECK(bkend.call_with_return(nullptr, "env", "multi", UINT32_C(1))->to_ui32() == UINT32_C(8));
   CHECK(bkend.call_with_return(nullptr, "env", "multi", UINT32_C(13))->to_ui32() == UINT32_C(8));
   CHECK(bkend.call_with_return(nullptr, "env", "multi", UINT32_C(4294967291))->to_ui32() == UINT32_C(8));
   CHECK(bkend.call_with_return(nullptr, "env", "nested", UINT32_C(0), UINT32_C(0))->to_ui32() == UINT32_C(11));
   CHECK(bkend.call_with_return(nullptr, "env", "nested", UINT32_C(1), UINT32_C(0))->to_ui32() == UINT32_C(10));
   CHECK(bkend.call_with_return(nullptr, "env", "nested", UINT32_C(0), UINT32_C(1))->to_ui32() == UINT32_C(10));
   CHECK(bkend.call_with_return(nullptr, "env", "nested", UINT32_C(3), UINT32_C(2))->to_ui32() == UINT32_C(9));
   CHECK(bkend.call_with_return(nullptr, "env", "nested", UINT32_C(0), UINT32_C(4294967196))->to_ui32() == UINT32_C(10));
   CHECK(bkend.call_with_return(nullptr, "env", "nested", UINT32_C(10), UINT32_C(10))->to_ui32() == UINT32_C(9));
   CHECK(bkend.call_with_return(nullptr, "env", "nested", UINT32_C(0), UINT32_C(4294967295))->to_ui32() == UINT32_C(10));
   CHECK(bkend.call_with_return(nullptr, "env", "nested", UINT32_C(4294967185), UINT32_C(4294967294))->to_ui32() == UINT32_C(9));
   CHECK(bkend.call_with_return(nullptr, "env", "as-select-first", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-select-first", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-select-mid", UINT32_C(0))->to_ui32() == UINT32_C(2));
   CHECK(bkend.call_with_return(nullptr, "env", "as-select-mid", UINT32_C(1))->to_ui32() == UINT32_C(2));
   CHECK(bkend.call_with_return(nullptr, "env", "as-select-last", UINT32_C(0))->to_ui32() == UINT32_C(3));
   CHECK(bkend.call_with_return(nullptr, "env", "as-select-last", UINT32_C(1))->to_ui32() == UINT32_C(2));
   CHECK(bkend.call_with_return(nullptr, "env", "as-loop-first", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-loop-first", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-loop-mid", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-loop-mid", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-loop-last", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-loop-last", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-if-condition", UINT32_C(0))->to_ui32() == UINT32_C(3));
   CHECK(bkend.call_with_return(nullptr, "env", "as-if-condition", UINT32_C(1))->to_ui32() == UINT32_C(2));
   CHECK(bkend.call_with_return(nullptr, "env", "as-br_if-first", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-br_if-first", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-br_if-last", UINT32_C(0))->to_ui32() == UINT32_C(3));
   CHECK(bkend.call_with_return(nullptr, "env", "as-br_if-last", UINT32_C(1))->to_ui32() == UINT32_C(2));
   CHECK(bkend.call_with_return(nullptr, "env", "as-br_table-first", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-br_table-first", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-br_table-last", UINT32_C(0))->to_ui32() == UINT32_C(2));
   CHECK(bkend.call_with_return(nullptr, "env", "as-br_table-last", UINT32_C(1))->to_ui32() == UINT32_C(2));
   CHECK(bkend.call_with_return(nullptr, "env", "as-call_indirect-first", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-call_indirect-first", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-call_indirect-mid", UINT32_C(0))->to_ui32() == UINT32_C(2));
   CHECK(bkend.call_with_return(nullptr, "env", "as-call_indirect-mid", UINT32_C(1))->to_ui32() == UINT32_C(2));
   CHECK(bkend.call_with_return(nullptr, "env", "as-call_indirect-last", UINT32_C(0))->to_ui32() == UINT32_C(2));
   CHECK_THROWS_AS(bkend(nullptr, "env", "as-call_indirect-last", UINT32_C(1)), std::exception);
   CHECK(!bkend.call_with_return(nullptr, "env", "as-store-first", UINT32_C(0)));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-store-first", UINT32_C(1)));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-store-last", UINT32_C(0)));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-store-last", UINT32_C(1)));
   CHECK(bkend.call_with_return(nullptr, "env", "as-memory.grow-value", UINT32_C(0))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-memory.grow-value", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-call-value", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-call-value", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-return-value", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-return-value", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-drop-operand", UINT32_C(0)));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-drop-operand", UINT32_C(1)));
   CHECK(bkend.call_with_return(nullptr, "env", "as-br-value", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-br-value", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-local.set-value", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-local.set-value", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-local.tee-value", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-local.tee-value", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-global.set-value", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-global.set-value", UINT32_C(1))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-load-operand", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-load-operand", UINT32_C(1))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-unary-operand", UINT32_C(0))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-unary-operand", UINT32_C(1))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-unary-operand", UINT32_C(4294967295))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-binary-operand", UINT32_C(0), UINT32_C(0))->to_ui32() == UINT32_C(15));
   CHECK(bkend.call_with_return(nullptr, "env", "as-binary-operand", UINT32_C(0), UINT32_C(1))->to_ui32() == UINT32_C(4294967284));
   CHECK(bkend.call_with_return(nullptr, "env", "as-binary-operand", UINT32_C(1), UINT32_C(0))->to_ui32() == UINT32_C(4294967281));
   CHECK(bkend.call_with_return(nullptr, "env", "as-binary-operand", UINT32_C(1), UINT32_C(1))->to_ui32() == UINT32_C(12));
   CHECK(bkend.call_with_return(nullptr, "env", "as-test-operand", UINT32_C(0))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-test-operand", UINT32_C(1))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-compare-operand", UINT32_C(0), UINT32_C(0))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-compare-operand", UINT32_C(0), UINT32_C(1))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "as-compare-operand", UINT32_C(1), UINT32_C(0))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return(nullptr, "env", "as-compare-operand", UINT32_C(1), UINT32_C(1))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "break-bare")->to_ui32() == UINT32_C(19));
   CHECK(bkend.call_with_return(nullptr, "env", "break-value", UINT32_C(1))->to_ui32() == UINT32_C(18));
   CHECK(bkend.call_with_return(nullptr, "env", "break-value", UINT32_C(0))->to_ui32() == UINT32_C(21));
   CHECK(bkend.call_with_return(nullptr, "env", "effects", UINT32_C(1))->to_ui32() == UINT32_C(4294967282));
   CHECK(bkend.call_with_return(nullptr, "env", "effects", UINT32_C(0))->to_ui32() == UINT32_C(4294967290));
}

