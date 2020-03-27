#include <eosio/vm/backend.hpp>

#include <catch2/catch.hpp>

using namespace eosio::vm;

#include "reentry.wasm.hpp"
#include "utils.hpp"

BACKEND_TEST_CASE("test reentry", "[reentry]") {
   struct test_runner;
   using rhf_t = eosio::vm::registered_host_functions<test_runner>;
   using backend_t = eosio::vm::backend<rhf_t, TestType>;
   struct test_runner {
      backend_t* bkend;
      uint32_t test_func_0(uint32_t val) {
         return bkend->call_with_return(*this, "env", "bar", val)->to_ui32() + 50;
      }
      uint32_t test_func_1(uint32_t val) {
         return bkend->call_with_return(*this, "env", "testbar", val)->to_ui32() + 50;
      }
      void eosio_assert(uint32_t, uint32_t) {}
      void* memset(void*, int, int) { return 0; }
   };

   wasm_allocator wa;

   rhf_t::template add<&test_runner::test_func_0>("env", "testfunc0");
   rhf_t::template add<&test_runner::test_func_1>("env", "testfunc1");
   rhf_t::template add<&test_runner::eosio_assert>("env", "eosio_assert");
   rhf_t::template add<&test_runner::memset>("env", "memset");

   test_runner tr;
   backend_t bkend(reentry_wasm, tr, &wa);

   tr.bkend = &bkend;

   // level 0
   CHECK(bkend.call_with_return(tr, "env", "foo", (uint32_t)10)->to_ui32() == 52);
   // level 1
   CHECK(bkend.call_with_return(tr, "env", "testbar", (uint32_t)10)->to_ui32() == 160);
   CHECK(bkend.call_with_return(tr, "env", "testbaz", (uint32_t)10)->to_ui32() == 513);
}
