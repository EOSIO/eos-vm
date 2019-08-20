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

BACKEND_TEST_CASE( "Testing wasm <store_0_wasm>", "[store_0_wasm_tests]" ) {
   using backend_t = backend<std::nullptr_t, TestType>;
   auto code = backend_t::read_wasm( std::string(wasm_directory) + "store.0.wasm");
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );
   bkend.initialize(nullptr);

   CHECK(!bkend.call_with_return(nullptr, "env", "as-block-value"));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-loop-value"));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-br-value"));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-br_if-value"));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-br_if-value-cond"));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-br_table-value"));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-return-value"));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-if-then"));
   CHECK(!bkend.call_with_return(nullptr, "env", "as-if-else"));
}

