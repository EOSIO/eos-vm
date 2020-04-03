#include <eosio/vm/backend.hpp>
#include <catch2/catch.hpp>
#include "utils.hpp"

using namespace eosio::vm;

extern wasm_allocator wa;

BACKEND_TEST_CASE("Tests that the arguments of top level calls are validated",
                  "[call_typecheck]") {
   /*
    * (module
    *  (func (export "f0"))
    *  (func (export "f1") (param i32))
    * )
    */
   std::vector<uint8_t> code = { 0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x02, 0x60,
                                 0x00, 0x00, 0x60, 0x01, 0x7f, 0x00, 0x03, 0x03, 0x02, 0x00, 0x01, 0x07,
                                 0x0b, 0x02, 0x02, 0x66, 0x30, 0x00, 0x00, 0x02, 0x66, 0x31, 0x00, 0x01,
                                 0x0a, 0x07, 0x02, 0x02, 0x00, 0x0b, 0x02, 0x00, 0x0b
   };

   using backend_t = backend<std::nullptr_t, TestType>;
   backend_t bkend(code, &wa);

   CHECK_THROWS_AS(bkend.call("env", "f0", 0), std::exception); // too many arguments
   CHECK_THROWS_AS(bkend.call("env", "f1"), std::exception); // too few arguments
   CHECK_THROWS_AS(bkend.call("env", "f1", UINT64_C(0)), std::exception); // wrong type of argument
   CHECK_THROWS_AS(bkend.call("env", "f1", UINT32_C(0), UINT32_C(0)), std::exception); // too many arguments
}
