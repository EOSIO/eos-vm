#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <fstream>
#include <string>
#include <catch2/catch.hpp>

#include <eosio/vm/leb128.hpp>
#include <eosio/vm/backend.hpp>
#include <eosio/vm/types.hpp>
#include <eosio/vm/opcodes.hpp>
#include <eosio/vm/parser.hpp>
#include <eosio/vm/constants.hpp>
#include <eosio/vm/sections.hpp>
//#include <eosio/vm/disassembly_visitor.hpp>
//#include <eosio/vm/interpret_visitor.hpp>

using namespace eosio;
using namespace eosio::vm;

struct my_host_functions {
   static int test(int value) { return value + 42; }
};

extern wasm_allocator wa;

TEST_CASE( "Testing host functions", "[host_functions_test]" ) {
   my_host_functions host;
   registered_function<my_host_functions, std::nullptr_t, &my_host_functions::test>("host", "test");

   using backend_t = backend<my_host_functions>;

   auto code = backend_t::read_wasm( "host.wasm" );
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );

   bkend.initialize();
   CHECK(bkend.call_with_return(&host, "env", "test", UINT32_C(5))->to_i32() == 49);
   CHECK(bkend.call_with_return(&host, "env", "test.indirect", UINT32_C(5), UINT32_C(0))->to_i32() == 47);
   CHECK(bkend.call_with_return(&host, "env", "test.indirect", UINT32_C(5), UINT32_C(1))->to_i32() == 49);
}

struct test_exception {};

struct host_functions_throw {
   static int test(int) { throw test_exception{}; }
};

TEST_CASE( "Testing throwing host functions", "[host_functions_throw_test]" ) {
   host_functions_throw host;
   registered_function<host_functions_throw, std::nullptr_t, &host_functions_throw::test>("host", "test");

   using backend_t = backend<host_functions_throw>;

   auto code = backend_t::read_wasm( "host.wasm" );
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );

   bkend.initialize();
   CHECK_THROWS_AS(bkend.call(&host, "env", "test", UINT32_C(2)), test_exception);
}

struct host_functions_exit {
   execution_context<host_functions_exit> * context;
   int test(int) { context->exit(); return 0; }
};

TEST_CASE( "Testing exiting host functions", "[host_functions_exit_test]" ) {
   registered_function<host_functions_exit, host_functions_exit, &host_functions_exit::test>("host", "test");

   using backend_t = backend<host_functions_exit>;

   auto code = backend_t::read_wasm( "host.wasm" );
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );
   host_functions_exit host{&bkend.get_context()};

   bkend.initialize();
   CHECK(!bkend.call_with_return(&host, "env", "test", UINT32_C(2)));
}

