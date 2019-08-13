#include <eosio/vm/backend.hpp>

#include <catch2/catch.hpp>

using namespace eosio::vm;

#include "reentry.wasm.hpp"

TEST_CASE("test reentry level 1", "[reentry_lvl1]") {
   wasm_allocator wa;
   using backend_t = eosio::vm::backend<nullptr_t>;
   backend_t bkend(reentry_wasm);
   bkend.set_wasm_allocator(&wa);
   bkend.initialize();
   
   uint32_t res = bkend.call_with_return(nullptr, "env", "foo", (uint32_t)14)->to_ui32();
   std::cout << "Result " << res << "\n";
}
