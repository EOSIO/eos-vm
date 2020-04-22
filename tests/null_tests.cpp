#include <eosio/vm/backend.hpp>
#include <catch2/catch.hpp>
#include "utils.hpp"

using namespace eosio::vm;

extern wasm_allocator wa;

TEST_CASE("Tests a null backend", "[null_backend]") {
   /*
    * (module)
    */
   std::vector<uint8_t> code = { 0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00 };

   using backend_t = backend<std::nullptr_t, null_backend>;
   backend_t bkend(code, nullptr);
}
