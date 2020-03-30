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

BACKEND_TEST_CASE( "Testing wasm <start_3_wasm>", "[start_3_wasm_tests]" ) {
   using backend_t = backend<standalone_function_t, TestType>;
   auto code = read_wasm( std::string(wasm_directory) + "start.3.wasm");
   backend_t bkend( code, get_wasm_allocator() );
   CHECK(bkend.call_with_return("env", "get")->to_ui32() == UINT32_C(68));
bkend("env", "inc");
   CHECK(bkend.call_with_return("env", "get")->to_ui32() == UINT32_C(69));
bkend("env", "inc");
   CHECK(bkend.call_with_return("env", "get")->to_ui32() == UINT32_C(70));
}

BACKEND_TEST_CASE( "Testing wasm <start_4_wasm>", "[start_4_wasm_tests]" ) {
   using backend_t = backend<standalone_function_t, TestType>;
   auto code = read_wasm( std::string(wasm_directory) + "start.4.wasm");
   backend_t bkend( code, get_wasm_allocator() );
   CHECK(bkend.call_with_return("env", "get")->to_ui32() == UINT32_C(68));
bkend("env", "inc");
   CHECK(bkend.call_with_return("env", "get")->to_ui32() == UINT32_C(69));
bkend("env", "inc");
   CHECK(bkend.call_with_return("env", "get")->to_ui32() == UINT32_C(70));
}

/*
TEST_CASE( "Testing wasm <start_5_wasm>", "[start_5_wasm_tests]" ) {
   auto code = read_wasm( std::string(wasm_directory) + "start.5.wasm");
   backend_t bkend( code, get_wasm_allocator() );
}

TEST_CASE( "Testing wasm <start_6_wasm>", "[start_6_wasm_tests]" ) {
   auto code = read_wasm( std::string(wasm_directory) + "start.6.wasm");
   backend_t bkend( code, get_wasm_allocator() );
}

TEST_CASE( "Testing wasm <start_7_wasm>", "[start_7_wasm_tests]" ) {
   auto code = read_wasm( std::string(wasm_directory) + "start.7.wasm");
   backend_t bkend( code, get_wasm_allocator() );
}
*/
