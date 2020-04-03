#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <fstream>
#include <string>

#include <catch2/catch.hpp>

#include <eosio/vm/backend.hpp>
#include "wasm_config.hpp"
#include "utils.hpp"

using namespace eosio;
using namespace eosio::vm;

void host_call() {}

#include "implementation_limits.hpp"

namespace {

wasm_code implementation_limits_wasm_code{
   implementation_limits_wasm + 0,
   implementation_limits_wasm + sizeof(implementation_limits_wasm)};

struct dynamic_options {
   std::uint32_t max_call_depth;
};

}

BACKEND_TEST_CASE( "Test call depth", "[call_depth]") {
   wasm_allocator wa;
   using rhf_t     = eosio::vm::registered_host_functions<standalone_function_t>;
   using backend_t = eosio::vm::backend<rhf_t, TestType>;

   rhf_t::add<&host_call>("env", "host.call");

   backend_t bkend(implementation_limits_wasm_code, get_wasm_allocator());

   rhf_t::resolve(bkend.get_module());

   CHECK(!bkend.call_with_return("env", "call", (uint32_t)250));
   CHECK_THROWS_AS(bkend.call("env", "call", (uint32_t)251), std::exception);
   CHECK(!bkend.call_with_return("env", "call.indirect", (uint32_t)250));
   CHECK_THROWS_AS(bkend.call("env", "call.indirect", (uint32_t)251), std::exception);
   // The host call is added to the recursive function, so we have one fewer frames
   CHECK(!bkend.call_with_return("env", "call.host", (uint32_t)249));
   CHECK_THROWS_AS(bkend.call("env", "call.host", (uint32_t)250), std::exception);
   CHECK(!bkend.call_with_return("env", "call.indirect.host", (uint32_t)249));
   CHECK_THROWS_AS(bkend.call("env", "call.indirect.host", (uint32_t)250), std::exception);
}

BACKEND_TEST_CASE( "Test call depth dynamic", "[call_depth]") {
   wasm_allocator wa;
   using backend_t = eosio::vm::backend<standalone_function_t, TestType, dynamic_options>;
   using rhf_t     = eosio::vm::registered_host_functions<standalone_function_t>;
   rhf_t::add<&host_call>("env", "host.call");

   backend_t bkend(implementation_limits_wasm_code, nullptr, dynamic_options{151});
   bkend.set_wasm_allocator(&wa);
   bkend.initialize(nullptr);

   rhf_t::resolve(bkend.get_module());

   CHECK(!bkend.call_with_return(nullptr, "env", "call", (uint32_t)150));
   CHECK_THROWS_AS(bkend.call(nullptr, "env", "call", (uint32_t)151), std::exception);
   CHECK(!bkend.call_with_return(nullptr, "env", "call.indirect", (uint32_t)150));
   CHECK_THROWS_AS(bkend.call(nullptr, "env", "call.indirect", (uint32_t)151), std::exception);
   // The host call is added to the recursive function, so we have one fewer frames
   CHECK(!bkend.call_with_return(nullptr, "env", "call.host", (uint32_t)149));
   CHECK_THROWS_AS(bkend.call(nullptr, "env", "call.host", (uint32_t)150), std::exception);
   CHECK(!bkend.call_with_return(nullptr, "env", "call.indirect.host", (uint32_t)149));
   CHECK_THROWS_AS(bkend.call(nullptr, "env", "call.indirect.host", (uint32_t)150), std::exception);

   bkend.initialize(nullptr, dynamic_options{51});

   CHECK(!bkend.call_with_return(nullptr, "env", "call", (uint32_t)50));
   CHECK_THROWS_AS(bkend.call(nullptr, "env", "call", (uint32_t)51), std::exception);
   CHECK(!bkend.call_with_return(nullptr, "env", "call.indirect", (uint32_t)50));
   CHECK_THROWS_AS(bkend.call(nullptr, "env", "call.indirect", (uint32_t)51), std::exception);
   // The host call is added to the recursive function, so we have one fewer frames
   CHECK(!bkend.call_with_return(nullptr, "env", "call.host", (uint32_t)49));
   CHECK_THROWS_AS(bkend.call(nullptr, "env", "call.host", (uint32_t)50), std::exception);
   CHECK(!bkend.call_with_return(nullptr, "env", "call.indirect.host", (uint32_t)49));
   CHECK_THROWS_AS(bkend.call(nullptr, "env", "call.indirect.host", (uint32_t)50), std::exception);
}
