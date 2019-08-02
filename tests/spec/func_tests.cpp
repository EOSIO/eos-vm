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
using backend_t = backend<std::nullptr_t>;

TEST_CASE( "Testing wasm <func_0_wasm>", "[func_0_wasm_tests]" ) {
   auto code = backend_t::read_wasm( std::string(wasm_directory) + "func.0.wasm");
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );
   bkend.initialize(nullptr);

   CHECK(!bkend.call_with_return(nullptr, "env", "type-use-1"));
   CHECK(bkend.call_with_return(nullptr, "env", "type-use-2")->to_ui32() == UINT32_C(0));
   CHECK(!bkend.call_with_return(nullptr, "env", "type-use-3", UINT32_C(1)));
   CHECK(bkend.call_with_return(nullptr, "env", "type-use-4", UINT32_C(1), bit_cast<double>(UINT64_C(4607182418800017408)), UINT32_C(1))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "type-use-5")->to_ui32() == UINT32_C(0));
   CHECK(!bkend.call_with_return(nullptr, "env", "type-use-6", UINT32_C(1)));
   CHECK(bkend.call_with_return(nullptr, "env", "type-use-7", UINT32_C(1), bit_cast<double>(UINT64_C(4607182418800017408)), UINT32_C(1))->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "local-first-i32")->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "local-first-i64")->to_ui64() == UINT32_C(0));
   CHECK(bit_cast<uint32_t>(bkend.call_with_return(nullptr, "env", "local-first-f32")->to_f32()) == UINT32_C(0));
   CHECK(bit_cast<uint64_t>(bkend.call_with_return(nullptr, "env", "local-first-f64")->to_f64()) == UINT64_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "local-second-i32")->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "local-second-i64")->to_ui64() == UINT32_C(0));
   CHECK(bit_cast<uint32_t>(bkend.call_with_return(nullptr, "env", "local-second-f32")->to_f32()) == UINT32_C(0));
   CHECK(bit_cast<uint64_t>(bkend.call_with_return(nullptr, "env", "local-second-f64")->to_f64()) == UINT64_C(0));
   CHECK(bit_cast<uint64_t>(bkend.call_with_return(nullptr, "env", "local-mixed")->to_f64()) == UINT64_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "param-first-i32", UINT32_C(2), UINT32_C(3))->to_ui32() == UINT32_C(2));
   CHECK(bkend.call_with_return(nullptr, "env", "param-first-i64", UINT64_C(2), UINT64_C(3))->to_ui64() == UINT32_C(2));
   CHECK(bit_cast<uint32_t>(bkend.call_with_return(nullptr, "env", "param-first-f32", bit_cast<float>(UINT32_C(1073741824)), bit_cast<float>(UINT32_C(1077936128)))->to_f32()) == UINT32_C(1073741824));
   CHECK(bit_cast<uint64_t>(bkend.call_with_return(nullptr, "env", "param-first-f64", bit_cast<double>(UINT64_C(4611686018427387904)), bit_cast<double>(UINT64_C(4613937818241073152)))->to_f64()) == UINT64_C(4611686018427387904));
   CHECK(bkend.call_with_return(nullptr, "env", "param-second-i32", UINT32_C(2), UINT32_C(3))->to_ui32() == UINT32_C(3));
   CHECK(bkend.call_with_return(nullptr, "env", "param-second-i64", UINT64_C(2), UINT64_C(3))->to_ui64() == UINT32_C(3));
   CHECK(bit_cast<uint32_t>(bkend.call_with_return(nullptr, "env", "param-second-f32", bit_cast<float>(UINT32_C(1073741824)), bit_cast<float>(UINT32_C(1077936128)))->to_f32()) == UINT32_C(1077936128));
   CHECK(bit_cast<uint64_t>(bkend.call_with_return(nullptr, "env", "param-second-f64", bit_cast<double>(UINT64_C(4611686018427387904)), bit_cast<double>(UINT64_C(4613937818241073152)))->to_f64()) == UINT64_C(4613937818241073152));
   CHECK(bit_cast<uint64_t>(bkend.call_with_return(nullptr, "env", "param-mixed", bit_cast<float>(UINT32_C(1065353216)), UINT32_C(2), UINT64_C(3), UINT32_C(4), bit_cast<double>(UINT64_C(4617878467915022336)), UINT32_C(6))->to_f64()) == UINT64_C(4617878467915022336));
   CHECK(!bkend.call_with_return(nullptr, "env", "empty"));
   CHECK(!bkend.call_with_return(nullptr, "env", "value-void"));
   CHECK(bkend.call_with_return(nullptr, "env", "value-i32")->to_ui32() == UINT32_C(77));
   CHECK(bkend.call_with_return(nullptr, "env", "value-i64")->to_ui64() == UINT32_C(7777));
   CHECK(bit_cast<uint32_t>(bkend.call_with_return(nullptr, "env", "value-f32")->to_f32()) == UINT32_C(1117480550));
   CHECK(bit_cast<uint64_t>(bkend.call_with_return(nullptr, "env", "value-f64")->to_f64()) == UINT64_C(4635172994171566817));
   CHECK(!bkend.call_with_return(nullptr, "env", "value-block-void"));
   CHECK(bkend.call_with_return(nullptr, "env", "value-block-i32")->to_ui32() == UINT32_C(77));
   CHECK(!bkend.call_with_return(nullptr, "env", "return-empty"));
   CHECK(bkend.call_with_return(nullptr, "env", "return-i32")->to_ui32() == UINT32_C(78));
   CHECK(bkend.call_with_return(nullptr, "env", "return-i64")->to_ui64() == UINT32_C(7878));
   CHECK(bit_cast<uint32_t>(bkend.call_with_return(nullptr, "env", "return-f32")->to_f32()) == UINT32_C(1117611622));
   CHECK(bit_cast<uint64_t>(bkend.call_with_return(nullptr, "env", "return-f64")->to_f64()) == UINT64_C(4635244066603186258));
   CHECK(bkend.call_with_return(nullptr, "env", "return-block-i32")->to_ui32() == UINT32_C(77));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-empty"));
   CHECK(bkend.call_with_return(nullptr, "env", "break-i32")->to_ui32() == UINT32_C(79));
   CHECK(bkend.call_with_return(nullptr, "env", "break-i64")->to_ui64() == UINT32_C(7979));
   CHECK(bit_cast<uint32_t>(bkend.call_with_return(nullptr, "env", "break-f32")->to_f32()) == UINT32_C(1117768909));
   CHECK(bit_cast<uint64_t>(bkend.call_with_return(nullptr, "env", "break-f64")->to_f64()) == UINT64_C(4635315139034805699));
   CHECK(bkend.call_with_return(nullptr, "env", "break-block-i32")->to_ui32() == UINT32_C(77));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-br_if-empty", UINT32_C(0)));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-br_if-empty", UINT32_C(2)));
   CHECK(bkend.call_with_return(nullptr, "env", "break-br_if-num", UINT32_C(0))->to_ui32() == UINT32_C(51));
   CHECK(bkend.call_with_return(nullptr, "env", "break-br_if-num", UINT32_C(1))->to_ui32() == UINT32_C(50));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-br_table-empty", UINT32_C(0)));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-br_table-empty", UINT32_C(1)));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-br_table-empty", UINT32_C(5)));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-br_table-empty", UINT32_C(4294967295)));
   CHECK(bkend.call_with_return(nullptr, "env", "break-br_table-num", UINT32_C(0))->to_ui32() == UINT32_C(50));
   CHECK(bkend.call_with_return(nullptr, "env", "break-br_table-num", UINT32_C(1))->to_ui32() == UINT32_C(50));
   CHECK(bkend.call_with_return(nullptr, "env", "break-br_table-num", UINT32_C(10))->to_ui32() == UINT32_C(50));
   CHECK(bkend.call_with_return(nullptr, "env", "break-br_table-num", UINT32_C(4294967196))->to_ui32() == UINT32_C(50));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-br_table-nested-empty", UINT32_C(0)));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-br_table-nested-empty", UINT32_C(1)));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-br_table-nested-empty", UINT32_C(3)));
   CHECK(!bkend.call_with_return(nullptr, "env", "break-br_table-nested-empty", UINT32_C(4294967294)));
   CHECK(bkend.call_with_return(nullptr, "env", "break-br_table-nested-num", UINT32_C(0))->to_ui32() == UINT32_C(52));
   CHECK(bkend.call_with_return(nullptr, "env", "break-br_table-nested-num", UINT32_C(1))->to_ui32() == UINT32_C(50));
   CHECK(bkend.call_with_return(nullptr, "env", "break-br_table-nested-num", UINT32_C(2))->to_ui32() == UINT32_C(52));
   CHECK(bkend.call_with_return(nullptr, "env", "break-br_table-nested-num", UINT32_C(4294967293))->to_ui32() == UINT32_C(52));
   CHECK(bkend.call_with_return(nullptr, "env", "init-local-i32")->to_ui32() == UINT32_C(0));
   CHECK(bkend.call_with_return(nullptr, "env", "init-local-i64")->to_ui64() == UINT32_C(0));
   CHECK(bit_cast<uint32_t>(bkend.call_with_return(nullptr, "env", "init-local-f32")->to_f32()) == UINT32_C(0));
   CHECK(bit_cast<uint64_t>(bkend.call_with_return(nullptr, "env", "init-local-f64")->to_f64()) == UINT64_C(0));
}

TEST_CASE( "Testing wasm <func_1_wasm>", "[func_1_wasm_tests]" ) {
   auto code = backend_t::read_wasm( std::string(wasm_directory) + "func.1.wasm");
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );
   bkend.initialize(nullptr);

}

TEST_CASE( "Testing wasm <func_3_wasm>", "[func_3_wasm_tests]" ) {
   auto code = backend_t::read_wasm( std::string(wasm_directory) + "func.3.wasm");
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );
   bkend.initialize(nullptr);

   CHECK(!bkend.call_with_return(nullptr, "env", "signature-explicit-reused"));
   CHECK(!bkend.call_with_return(nullptr, "env", "signature-implicit-reused"));
   CHECK(!bkend.call_with_return(nullptr, "env", "signature-explicit-duplicate"));
   CHECK(!bkend.call_with_return(nullptr, "env", "signature-implicit-duplicate"));
}

