#include <algorithm>
#include <cstdlib>
#include <limits>
#include <vector>
#include <iostream>

#include <catch2/catch.hpp>

#include <eosio/vm/compiler/code_interval.hpp>

using namespace eosio;
using namespace eosio::vm;

TEST_CASE("Testing code_interval iterators", "[code_interval_iterators_tests]") { 
   std::array<opcode, 3> opcodes = { i32_const_t{ 32 }, i32_const_t{42}, i32_const_t{52} };
   code_interval ci(opcodes.data(), opcodes.data()+2);
   auto iter = ci.begin();
   CHECK(iter == code_interval::iterator(opcodes.data()));
   CHECK(++iter == code_interval::iterator(opcodes.data()+1));
   iter++;
   CHECK(iter == code_interval::iterator(opcodes.data()+2));

   auto iter2 = ci.begin();
   CHECK(iter2.next() == code_interval::iterator(opcodes.data()+1));
}

TEST_CASE("Testing code_interval", "[code_interval_tests]") {
   std::array<opcode, 3> opcodes = { i32_const_t{ 32 }, i32_const_t{42}, i32_const_t{52} };
   code_interval ci(opcodes.data(), opcodes.data()+2);
   CHECK(ci.first() == opcodes.data()); 
   CHECK(ci.last() == opcodes.data()+(opcodes.size()-1)); 

   CHECK(ci.at(0) == opcodes.data());
   CHECK(ci.at(1) == opcodes.data()+1);
   CHECK(ci.at(2) == opcodes.data()+2);
   CHECK_THROWS_AS(ci.at(3), wasm_compilation_exception);

   CHECK(ci[0] == opcodes.data());
   CHECK(ci[1] == opcodes.data()+1);
   CHECK(ci[2] == opcodes.data()+2);
   CHECK_THROWS_AS(ci[3], wasm_compilation_exception);

   CHECK(ci.size() == 3);
}
