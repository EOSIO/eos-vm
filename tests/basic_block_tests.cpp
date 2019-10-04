#include <algorithm>
#include <cstdlib>
#include <limits>
#include <vector>
#include <iostream>

#include <catch2/catch.hpp>

#include <eosio/vm/compiler/basic_block.hpp>

using namespace eosio;
using namespace eosio::vm;

TEST_CASE("Testing basic_block", "[basic_block_tests]") { 
   std::array<opcode, 3> opcodes = { i32_const_t{ 32 }, i32_const_t{42}, i32_const_t{52} };
   code_interval ci(opcodes.data(), opcodes.data()+2);

   basic_block bb(0, ci);
   
   auto pred = [](opcode* op) { return op->is_a<i64_const_t>(); };

   auto vis = bb.get_visitor();
   CHECK(!vis(pred));

   std::array<opcode, 3> opcodes2 = { i32_const_t{ 32 }, i32_const_t{42}, i64_const_t{uint64_t(52)} };
   basic_block bb2(0, {opcodes2.data(), opcodes2.data()+2});

   auto vis2 = bb2.get_visitor();
   CHECK(vis2(pred));
   CHECK(vis2(pred)->template get<i64_const_t>().data.ui == 52);
}

