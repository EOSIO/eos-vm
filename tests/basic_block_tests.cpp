#include <algorithm>
#include <cstdlib>
#include <limits>
#include <vector>
#include <iostream>

#include <catch2/catch.hpp>

#include <eosio/vm/compiler/basic_block.hpp>
#include <eosio/vm/compiler/control_flow_graph.hpp>
#include <eosio/vm/compiler/debug.hpp>
#include <eosio/vm/backend.hpp>

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

#include "control_flow_tests.wasm.hpp"

TEST_CASE("Testing control_flow_graph", "[cfg_tests]") { 
   std::array<opcode, 3> opcodes = { i32_const_t{ 32 }, i32_const_t{42}, i64_const_t{uint64_t(52)} };
   code_interval ci(opcodes.data(), opcodes.data()+2);
   control_flow_graph cfg(ci);

   const auto& blocks = cfg.get_blocks();
   CHECK( blocks.size() == 1 );
   CHECK( blocks[0].get_interval().size() == opcodes.size() );
   CHECK( blocks[0].get_interval() == ci );

   wasm_allocator wa;
   using backend_t = eosio::vm::backend<nullptr_t>;
   backend_t bkend(control_flow_tests_wasm);
   auto& mod = bkend.get_module();
  
   uint32_t full_code_size = 0;
   for (int i=0; i < mod.code.size(); i++) {
      full_code_size += mod.code[i].size;
   }
   code_interval cci(mod.code[1].code, mod.code[1].code + (mod.code[1].size-1));

   control_flow_graph cfg2(cci);

   const auto& blocks2 = cfg2.get_blocks();
   cfg_writer cw(cfg2);
   cw.write("cfg.dot");
   CHECK( blocks2.size() == 0 );
}
