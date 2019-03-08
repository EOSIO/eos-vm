#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>

#include "utils.hpp"

#include <eosio/wasm_backend/interpret_visitor.hpp>
#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/parser.hpp>
#include <eosio/wasm_backend/execution_context.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;

template <typename T>
static inline execution_context<T> create_execution_context(const std::string& fname, module& mod) {
   binary_parser bp;
   wasm_code code = read_wasm(fname);
   bp.parse_module( code, mod );
   return {mod};
}

BOOST_AUTO_TEST_SUITE(spec_tests)
BOOST_AUTO_TEST_CASE(address_tests) { 
   memory_manager::set_memory_limits( 32*1024*1024 );
   module mod;
   try {
      auto ctx = create_execution_context<interpret_visitor>("wasms/address32.wasm", mod);

      ;
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8u_good1", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8u_good2", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8u_good3", (uint32_t)0)), 'b');
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8u_good4", (uint32_t)0)), 'c');
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8u_good5", (uint32_t)0)), 'z');

      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8s_good1", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8s_good2", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8s_good3", (uint32_t)0)), 'b');
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8s_good4", (uint32_t)0)), 'c');
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8s_good5", (uint32_t)0)), 'z');

      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16u_good1", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16u_good2", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16u_good3", (uint32_t)0)), 25442);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16u_good4", (uint32_t)0)), 25699);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16u_good5", (uint32_t)0)), 122);

      
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16s_good1", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16s_good2", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16s_good3", (uint32_t)0)), 25442);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16s_good4", (uint32_t)0)), 25699);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16s_good5", (uint32_t)0)), 122);

      
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("32_good1", (uint32_t)0)), 1684234849);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("32_good2", (uint32_t)0)), 1684234849);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("32_good3", (uint32_t)0)), 1701077858);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("32_good4", (uint32_t)0)), 1717920867);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("32_good5", (uint32_t)0)), 122);

      
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8u_good1", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8u_good2", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8u_good3", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8u_good4", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8u_good5", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8s_good1", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8s_good2", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8s_good3", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8s_good4", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("8s_good5", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16u_good1", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16u_good2", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16u_good3", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16u_good4", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16u_good5", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16s_good1", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16s_good2", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16s_good3", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16s_good4", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("16s_good5", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("32_good1", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("32_good2", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("32_good3", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("32_good4", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(ctx.execute("32_good5", (uint32_t)65507)), 0);
   } FC_LOG_AND_RETHROW() 
}
BOOST_AUTO_TEST_SUITE_END()
