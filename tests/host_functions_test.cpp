#include <cstdint>
#include <catch2/catch.hpp>

#include <eosio/vm/backend.hpp>
#include "wasm_config.hpp"

using namespace eosio;
using namespace eosio::vm;

struct my_host_functions {
   static int test(int value) { return value + 42; }
   static int test2(int value) { return value * 42; }
};

extern wasm_allocator wa;

TEST_CASE( "Testing host functions", "[host_functions_test]" ) {
   my_host_functions host;
   registered_function<my_host_functions, std::nullptr_t, &my_host_functions::test>("host", "test");
   registered_function<my_host_functions, std::nullptr_t, &my_host_functions::test2>("host", "test2");

   using backend_t = backend<my_host_functions>;

   auto code = backend_t::read_wasm( host_wasm );
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );
   registered_host_functions<my_host_functions>::resolve(bkend.get_module());

   bkend.initialize();
   CHECK(bkend.call_with_return(&host, "env", "test", UINT32_C(5))->to_i32() == 49);
   CHECK(bkend.call_with_return(&host, "env", "test.indirect", UINT32_C(5), UINT32_C(0))->to_i32() == 47);
   CHECK(bkend.call_with_return(&host, "env", "test.indirect", UINT32_C(5), UINT32_C(1))->to_i32() == 210);
   CHECK(bkend.call_with_return(&host, "env", "test.indirect", UINT32_C(5), UINT32_C(2))->to_i32() == 49);
   CHECK_THROWS_AS(bkend.call(&host, "env", "test.indirect", UINT32_C(5), UINT32_C(3)), std::exception);
   CHECK(bkend.call_with_return(&host, "env", "test.local-call", UINT32_C(5))->to_i32() == 147);
}

struct test_exception {};

struct host_functions_throw {
   static int test(int) { throw test_exception{}; }
};

TEST_CASE( "Testing throwing host functions", "[host_functions_throw_test]" ) {
   host_functions_throw host;
   registered_function<host_functions_throw, std::nullptr_t, &host_functions_throw::test>("host", "test");
   registered_function<host_functions_throw, std::nullptr_t, &host_functions_throw::test>("host", "test2");

   using backend_t = backend<host_functions_throw>;

   auto code = backend_t::read_wasm( host_wasm );
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );
   registered_host_functions<host_functions_throw>::resolve(bkend.get_module());

   bkend.initialize();
   CHECK_THROWS_AS(bkend.call(&host, "env", "test", UINT32_C(2)), test_exception);
}

struct host_functions_exit {
   execution_context<host_functions_exit> * context;
   int test(int) { context->exit(); return 0; }
};

TEST_CASE( "Testing exiting host functions", "[host_functions_exit_test]" ) {
   registered_function<host_functions_exit, host_functions_exit, &host_functions_exit::test>("host", "test");
   registered_function<host_functions_exit, host_functions_exit, &host_functions_exit::test>("host", "test2");

   using backend_t = backend<host_functions_exit>;

   auto code = backend_t::read_wasm( host_wasm );
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );
   registered_host_functions<host_functions_exit>::resolve(bkend.get_module());
   host_functions_exit host{&bkend.get_context()};

   bkend.initialize();
   CHECK(!bkend.call_with_return(&host, "env", "test", UINT32_C(2)));
}

