#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <fstream>
#include <string>

#include <catch2/catch.hpp>

#include <eosio/vm/backend.hpp>
#include "wasm_config.hpp"

using namespace eosio;
using namespace eosio::vm;

// host functions that are C-style functions
// wasm hex
/* Code used to generate test, compile with eosio-cpp v1.6.2 with minor manual edits to remove unneeded imports
 * extern "C" {
      struct state_t { float f; int i; };
      [[eosio::wasm_import]]
      void c_style_host_function_0();
      [[eosio::wasm_import]]
      void c_style_host_function_1(int);
      [[eosio::wasm_import]]
      void c_style_host_function_2(int, int);
      [[eosio::wasm_import]]
      void c_style_host_function_3(int, float);
      [[eosio::wasm_import]]
      void c_style_host_function_4(const state_t&);

      [[eosio::wasm_entry]]
      void apply(unsigned long long a, unsigned long long b, unsigned long long c) {
         if (a == 0)
            c_style_host_function_0(); 
         else if (a == 1)
            c_style_host_function_1((int)b); 
         else if (a == 2)
            c_style_host_function_2((int)b, (int)c); 
         else if (a == 3)
            c_style_host_function_3((int)b, *((int*)&c)); 
         else if (a == 4) {
            state_t s = {*((float*)&c), (int)b};
            c_style_host_function_4(s); 
         }
      }
   } */

#include "host_functions_tests_0.wasm.hpp"
// no return value and no input parameters
int c_style_host_function_state = 0;
struct state_t {
   float f = 0;
   int   i = 0;
};
void c_style_host_function_0() {
   c_style_host_function_state = 1; 
}
void c_style_host_function_1(int s) {
   c_style_host_function_state = s;
}
void c_style_host_function_2(int a, int b) {
   c_style_host_function_state = a+b;
}
void c_style_host_function_3(int a, float b) {
   c_style_host_function_state = a+b;
}
void c_style_host_function_4(const state_t& ss) {
   c_style_host_function_state = ss.i;
}

TEST_CASE( "Test C-style host function system", "[C-style_host_functions_tests]") { 
   wasm_allocator wa;
   using backend_t = eosio::vm::backend<nullptr_t>;
   using rhf_t     = eosio::vm::registered_host_functions<nullptr_t>;
   rhf_t::add<nullptr_t, &c_style_host_function_0, wasm_allocator>("env", "c_style_host_function_0");
   rhf_t::add<nullptr_t, &c_style_host_function_1, wasm_allocator>("env", "c_style_host_function_1");
   rhf_t::add<nullptr_t, &c_style_host_function_2, wasm_allocator>("env", "c_style_host_function_2");
   rhf_t::add<nullptr_t, &c_style_host_function_3, wasm_allocator>("env", "c_style_host_function_3");
   rhf_t::add<nullptr_t, &c_style_host_function_4, wasm_allocator>("env", "c_style_host_function_4");

   backend_t bkend(host_functions_test_0_wasm);
   bkend.set_wasm_allocator(&wa);
   bkend.initialize(nullptr);

   rhf_t::resolve(bkend.get_module());

   bkend.call(nullptr, "env", "apply", (uint64_t)0, (uint64_t)0, (uint64_t)0);
   CHECK(c_style_host_function_state == 1);

   bkend.call(nullptr, "env", "apply", (uint64_t)1, (uint64_t)2, (uint64_t)0);
   CHECK(c_style_host_function_state == 2);

   bkend.call(nullptr, "env", "apply", (uint64_t)2, (uint64_t)1, (uint64_t)2);
   CHECK(c_style_host_function_state == 3);

   float f = 2.4f;
   bkend.call(nullptr, "env", "apply", (uint64_t)3, (uint64_t)2, *(uint64_t*)&f);
   CHECK(c_style_host_function_state == 0x40199980);

   bkend.call(nullptr, "env", "apply", (uint64_t)4, (uint64_t)5, *(uint64_t*)&f);
   CHECK(c_style_host_function_state == 5);
}

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
   jit_execution_context<host_functions_exit> * context;
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
