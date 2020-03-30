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

BACKEND_TEST_CASE( "Testing wasm <forward_0_wasm>", "[forward_0_wasm_tests]" ) {
   using backend_t = backend<standalone_function_t, TestType>;
   auto code = read_wasm( std::string(wasm_directory) + "forward.0.wasm");
   backend_t bkend( code, get_wasm_allocator() );
   CHECK(bkend.call_with_return("env", "even", UINT32_C(13))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return("env", "even", UINT32_C(20))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return("env", "odd", UINT32_C(13))->to_ui32() == UINT32_C(1));
   CHECK(bkend.call_with_return("env", "odd", UINT32_C(20))->to_ui32() == UINT32_C(0));
}
