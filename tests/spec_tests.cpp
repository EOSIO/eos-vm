#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <iostream>
#include <cmath>

#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>

#include "utils.hpp"

#include <eosio/wasm_backend/backend.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;

wasm_allocator wa;

BOOST_AUTO_TEST_SUITE(spec_tests)

BOOST_AUTO_TEST_CASE(address_tests) {
   // i32 bits
   try {
      auto code = backend::read_wasm( "wasms/address_i32.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good1", (uint32_t)0)), (uint32_t)97);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good2", (uint32_t)0)), (uint32_t)97);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good3", (uint32_t)0)), (uint32_t)98);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good4", (uint32_t)0)), (uint32_t)99);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good5", (uint32_t)0)), (uint32_t)122);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good1", (uint32_t)0)), (uint32_t)97);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good2", (uint32_t)0)), (uint32_t)97);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good3", (uint32_t)0)), (uint32_t)98);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good4", (uint32_t)0)), (uint32_t)99);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good5", (uint32_t)0)), (uint32_t)122);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good1", (uint32_t)0)), (uint32_t)25185);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good2", (uint32_t)0)), (uint32_t)25185);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good3", (uint32_t)0)), (uint32_t)25442);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good4", (uint32_t)0)), (uint32_t)25699);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good5", (uint32_t)0)), (uint32_t)122);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good1", (uint32_t)0)), (uint32_t)25185);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good2", (uint32_t)0)), (uint32_t)25185);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good3", (uint32_t)0)), (uint32_t)25442);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good4", (uint32_t)0)), (uint32_t)25699);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good5", (uint32_t)0)), (uint32_t)122);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good1", (uint32_t)0)), (uint32_t)1684234849);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good2", (uint32_t)0)), (uint32_t)1684234849);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good3", (uint32_t)0)), (uint32_t)1701077858);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good4", (uint32_t)0)), (uint32_t)1717920867);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good5", (uint32_t)0)), (uint32_t)122);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good1", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good2", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good3", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good4", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good5", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good1", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good2", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good3", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good4", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good5", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good1", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good2", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good3", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good4", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good5", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good1", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good2", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good3", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good4", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good5", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good1", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good2", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good3", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good4", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good5", (uint32_t)65507)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good1", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good2", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good3", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good4", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8u_good5", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good1", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good2", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good3", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good4", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("8s_good5", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good1", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good2", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good3", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good4", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16u_good5", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good1", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good2", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good3", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good4", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("16s_good5", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good1", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good2", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good3", (uint32_t)65508)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("32_good4", (uint32_t)65508)), (uint32_t)0);
   } FC_LOG_AND_RETHROW()

   // i64 bits
   try {
      auto code = backend::read_wasm( "wasms/address_i64.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good1", (uint32_t)0)), (uint64_t)97);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good2", (uint32_t)0)), (uint64_t)97);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good3", (uint32_t)0)), (uint64_t)98);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good4", (uint32_t)0)), (uint64_t)99);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good5", (uint32_t)0)), (uint64_t)122);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good1", (uint32_t)0)), (uint64_t)97);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good2", (uint32_t)0)), (uint64_t)97);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good3", (uint32_t)0)), (uint64_t)98);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good4", (uint32_t)0)), (uint64_t)99);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good5", (uint32_t)0)), (uint64_t)122);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good1", (uint32_t)0)), (uint64_t)25185);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good2", (uint32_t)0)), (uint64_t)25185);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good3", (uint32_t)0)), (uint64_t)25442);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good4", (uint32_t)0)), (uint64_t)25699);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good5", (uint32_t)0)), (uint64_t)122);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good1", (uint32_t)0)), (uint64_t)25185);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good2", (uint32_t)0)), (uint64_t)25185);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good3", (uint32_t)0)), (uint64_t)25442);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good4", (uint32_t)0)), (uint64_t)25699);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good5", (uint32_t)0)), (uint64_t)122);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good1", (uint32_t)0)), (uint64_t)1684234849);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good2", (uint32_t)0)), (uint64_t)1684234849);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good3", (uint32_t)0)), (uint64_t)1701077858);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good4", (uint32_t)0)), (uint64_t)1717920867);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good5", (uint32_t)0)), (uint64_t)122);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good1", (uint32_t)0)), (uint64_t)1684234849);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good2", (uint32_t)0)), (uint64_t)1684234849);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good3", (uint32_t)0)), (uint64_t)1701077858);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good4", (uint32_t)0)), (uint64_t)1717920867);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good5", (uint32_t)0)), (uint64_t)122);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good1", (uint32_t)0)), (uint64_t)0x6867666564636261);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good2", (uint32_t)0)), (uint64_t)0x6867666564636261);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good3", (uint32_t)0)), (uint64_t)0x6968676665646362);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good4", (uint32_t)0)), (uint64_t)0x6a69686766656463);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good5", (uint32_t)0)), (uint64_t)122);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good1", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good2", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good3", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good4", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good5", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good1", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good2", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good3", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good4", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good5", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good1", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good2", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good3", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good4", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good5", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good1", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good2", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good3", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good4", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good5", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good1", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good2", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good3", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good4", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good5", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good1", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good2", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good3", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good4", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good5", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good1", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good2", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good3", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good4", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good5", (uint32_t)65503)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good1", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good2", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good3", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good4", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8u_good5", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good1", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good2", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good3", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good4", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("8s_good5", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good1", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good2", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good3", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good4", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16u_good5", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good1", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good2", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good3", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good4", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("16s_good5", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good1", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good2", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good3", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good4", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32u_good5", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good1", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good2", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good3", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good4", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("32s_good5", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good1", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good2", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good3", (uint32_t)65504)), (uint64_t)0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("64_good4", (uint32_t)65504)), (uint64_t)0);
   } FC_LOG_AND_RETHROW()

   // f32 bits
   try {
      auto code = backend::read_wasm( "wasms/address_f32.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good1", (uint32_t)0)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good2", (uint32_t)0)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good3", (uint32_t)0)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good4", (uint32_t)0)), (float)0.0);
      BOOST_CHECK(std::isnan(TO_F32(*bkend("32_good5", (uint32_t)0))));
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good1", (uint32_t)65524)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good2", (uint32_t)65524)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good3", (uint32_t)65524)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good4", (uint32_t)65524)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good5", (uint32_t)65524)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good1", (uint32_t)65525)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good2", (uint32_t)65525)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good3", (uint32_t)65525)), (float)0.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("32_good4", (uint32_t)65525)), (float)0.0);
   } FC_LOG_AND_RETHROW()

   // f64 bits
   try {
      auto code = backend::read_wasm( "wasms/address_f64.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good1", (uint32_t)0)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good2", (uint32_t)0)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good3", (uint32_t)0)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good4", (uint32_t)0)), (double)0.0);
      BOOST_CHECK(std::isnan(TO_F64(*bkend("64_good5", (uint32_t)0))));
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good1", (uint32_t)65510)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good2", (uint32_t)65510)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good3", (uint32_t)65510)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good4", (uint32_t)65510)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good5", (uint32_t)65510)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good1", (uint32_t)65511)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good2", (uint32_t)65511)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good3", (uint32_t)65511)), (double)0.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("64_good4", (uint32_t)65511)), (double)0.0);
   } FC_LOG_AND_RETHROW()
}

#if 0
// tests to ensure that we are only accepting proper wasm binaries
BOOST_AUTO_TEST_CASE(binary_tests) {
   try {
      {
         memory_manager::set_memory_limits( 32*1024*1024 );
         module mod;
         create_execution_context<interpret_visitor>("wasms/binary/b0.wasm", mod);
      }
      {
         memory_manager::set_memory_limits( 32*1024*1024 );
         module mod;
         create_execution_context<interpret_visitor>("wasms/binary/b1.wasm", mod);
      }
      {
         memory_manager::set_memory_limits( 32*1024*1024 );
         module mod;
         create_execution_context<interpret_visitor>("wasms/binary/b2.wasm", mod);
      }
      {
         memory_manager::set_memory_limits( 32*1024*1024 );
         module mod;
         create_execution_context<interpret_visitor>("wasms/binary/b3.wasm", mod);
      }
      {
         memory_manager::set_memory_limits( 32*1024*1024 );
         module mod;
         BOOST_CHECK_THROW(create_execution_context<interpret_visitor>("wasms/binary/b4.wasm", mod), wasm_memory_exception);
      }
      // static constexpr const char* _wasm = "\x6d\x73\61\x00msa\x00\x01\x00\x00\x00msa\x00\x00\x00\x00\x01asm\x01\x00\x00\x00\x00wasm\x01\x00\x00\x00\x7fasm\x01\x00\x00\x00\x80asm\x01\x00\x00\x00\x82asm\x01\x00\x00\x00\xffasm\x01\x00\x00\x00\x00\x00\x00\x01msa\x00a\x00ms\x00\x01\x00\x00sm\x00a\x00\x00\x01\x00\x00ASM\x01\x00\x00\x00\x00\x81\xa2\x94\x01\x00\x00\x00\xef\xbb\xbf\x00asm\x01\x00\x00\x00\x00asm\x00asm\x01\x00asm\x01\x00\x00\x00asm\x00\x00\x00\x00\x00asm\x0d\x00\x00\x00\x00asm\x0e\x00\x00\x00\x00asm\x00\x01\x00\x00\x00asm\x00\x00\x01\x00\x00asm\x00\x00\x00\x01\x00asm\x01\x00\x00\x00\x05\x04\x01\x00\x82\x00\x00asm\x01\x00\x00\x00\x05\x07\x01\x00\x82\x80\x80\x80\x00\x00asm\x01\x00\x00\x00\x06\x07\x01\x7f\x00\x41\x80\x00\x0\x00asm\x01\x00\x00\x00\x06\x07\x01\x7f\x00\x41\xff\x7f\x0b\x00asm\x01\x00\x00\x00\x06\x0a\x01\x7f\x00\x41\x80\x80\x80\x80\x00\x0b\x00asm\x01\x00\x00\x00\x06\x0a\x01\x7f\x00\x41\xff\xff\xff\xff\x7f\x0b\x00asm\x01\x00\x00\x00\x06\x07\x01\x7e\x00\x42\x80\x00\x0b\x00asm\x01\x00\x00\x00\x06\x07\x01\x7e\x00\x42\xff\x7f\x0b\x00asm\x01\x00\x00\x00\x06\x0f\x01\x7e\x00\x42\x80\x80\x80\x80\x80\x80\x80\x80\x80\x00\x0b\x00asm\x01\x00\x00\x00\x06\x0f\x01\x7e\x00\x42\xff\xff\xff\xff\xff\xff\xff\xff\xff\x7f\x0b\x00asm\x01\x00\x00\x00\x05\x03\x01\x00\x00\x0b\x07\x01\x80\x00\x41\x00\x0b\x0\x00asm\x01\x00\x00\x00\x04\x04\x01\x70\x00\x00\x09\x07\x01\x80\x00\x41\x00\x0b\x0\x00asm\x01\x00\x00\x00\x05\x08\x01\x00\x82\x80\x80\x80\x80\x00\x00asm\x01\x00\x00\x00\x06\x0b\x01\x7f\x00\x41\x80\x80\x80\x80\x80\x00\x0b\x00asm\x01\x00\x00\x00\x06\x0b\x01\x7f\x00\x41\xff\xff\xff\xff\xff\x7f\x0b\x00asm\x01\x00\x00\x00\x06\x10\x01\x7e\x00\x42\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x00\x0b\x00asm\x01\x00\x00\x00\x06\x10\x01\x7e\x00\x42\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x7f\x0b\x00asm\x01\x00\x00\x00\x05\x07\x01\x00\x82\x80\x80\x80\x7\x00asm\x01\x00\x00\x00\x05\x07\x01\x00\x82\x80\x80\x80\x4\x00asm\x01\x00\x00\x00\x06\x0a\x01\x7f\x00\x41\x80\x80\x80\x80\x70\x0b\x00asm\x01\x00\x00\x00\x06\x0a\x01\x7f\x00\x41\xff\xff\xff\xff\x0f\x0b\x00asm\x01\x00\x00\x00\x06\x0a\x01\x7f\x00\x41\x80\x80\x80\x80\x1f\x0b\x00asm\x01\x00\x00\x00\x06\x0a\x01\x7f\x00\x41\xff\xff\xff\xff\x4f\x0b\x00asm\x01\x00\x00\x00\x06\x0f\x01\x7e\x00\x42\x80\x80\x80\x80\x80\x80\x80\x80\x80\x7e\x0b\x00asm\x01\x00\x00\x00\x06\x0f\x01\x7e\x00\x42\xff\xff\xff\xff\xff\xff\xff\xff\xff\x01\x0b\x00asm\x01\x00\x00\x00\x06\x0f\x01\x7e\x00\x42\x80\x80\x80\x80\x80\x80\x80\x80\x80\x02\x0b\x00asm\x01\x00\x00\x00\x06\x0f\x01\x7e\x00\x42\xff\xff\xff\xff\xff\xff\xff\xff\xff\x41\x0b\x00asm\x01\x00\x00\x00\x01\x04\x01\x60\x00\x00\x03\x02\x01\x00\x04\x04\x01\x70\x00\x00\x0a\x09\x01\x07\x00\x41\x00\x11\x00\x01\x0b\x00asm\x01\x00\x00\x00\x01\x04\x01\x60\x00\x00\x03\x02\x01\x00\x04\x04\x01\x70\x00\x00\x0a\x0a\x01\x07\x00\x41\x00\x11\x00\x80\x00\x0b\x00asm\x01\x00\x00\x00\x01\x04\x01\x60\x00\x00\x03\x02\x01\x00\x04\x04\x01\x70\x00\x00\x0a\x0b\x01\x08\x00\x41\x00\x11\x00\x80\x80\x00\x0b\x00asm\x01\x00\x00\x00\x03\x01\x00unctio";
    std::vector<uint8_t> code;
    //for (int i=0; i < strlen(_wasm); i++) {
       //   code.push_back((uint8_t)_wasm[i]);
    //}
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+0, 6};
      module mod;
      bp.parse_module(cp, 6, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+6, 18};
      module mod;
      bp.parse_module(cp, 18, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+24, 18};
      module mod;
      bp.parse_module(cp, 18, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+42, 18};
      module mod;
      bp.parse_module(cp, 18, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+60, 16};
      module mod;
      bp.parse_module(cp, 16, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+76, 18};
      module mod;
      bp.parse_module(cp, 18, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+94, 18};
      module mod;
      bp.parse_module(cp, 18, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+112, 18};
      module mod;
      bp.parse_module(cp, 18, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+130, 18};
      module mod;
      bp.parse_module(cp, 18, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 18};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 18, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 18};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 18, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 18};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 18, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 18};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 18, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 24};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 24, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 27};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 27, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 6};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 6, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 9};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 9, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 15};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 15, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 18};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 18, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 18};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 18, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 18};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 18, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 18};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 18, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 18};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 18, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 18};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 18, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+148, 36};
      module mod;
      bp.parse_module(cp, 36, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+184, 45};
      module mod;
      bp.parse_module(cp, 45, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+229, 44};
      module mod;
      bp.parse_module(cp, 44, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+273, 45};
      module mod;
      bp.parse_module(cp, 45, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+318, 54};
      module mod;
      bp.parse_module(cp, 54, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+372, 54};
      module mod;
      bp.parse_module(cp, 54, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+426, 45};
      module mod;
      bp.parse_module(cp, 45, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+471, 45};
      module mod;
      bp.parse_module(cp, 45, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+516, 69};
      module mod;
      bp.parse_module(cp, 69, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+585, 69};
      module mod;
      bp.parse_module(cp, 69, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+654, 59};
      module mod;
      bp.parse_module(cp, 59, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+713, 62};
      module mod;
      bp.parse_module(cp, 62, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 48};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 48, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 57};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 57, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 57};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 57, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 72};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 72, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 72};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 72, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 44};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 44, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 44};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 44, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 54};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 54, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 54};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 54, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 54};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 54, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 54};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 54, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 69};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 69, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 69};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 69, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 69};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 69, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 69};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 69, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 99};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 99, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 102};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 102, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 105};
      module mod;
      BOOST_CHECK_THROW(bp.parse_module(cp, 105, mod), wasm_parse_exception);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+775, 27};
      module mod;
      bp.parse_module(cp, 27, mod);
   }
   {
      binary_parser bp;
      wasm_code_ptr cp{code.data()+802, 6};
      module mod;
      bp.parse_module(cp, 6, mod);
   }

   } FC_LOG_AND_RETHROW()
}
#endif

BOOST_AUTO_TEST_CASE(blocks_tests) {
   try {
      auto code = backend::read_wasm( "wasms/blocks.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK(!bkend("empty"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singular")), (uint32_t)7);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multi")), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested")), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("deep")), (uint32_t)150);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-first")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-mid")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-last")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-first")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-mid")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-last")), (uint32_t)1);
      BOOST_CHECK(!bkend("as-if-condition"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-then")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-else")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-first")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-last")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-first")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-last")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-first")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-mid")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-last")), (uint32_t)1);
      BOOST_CHECK(!bkend("as-store-first"));
      BOOST_CHECK(!bkend("as-store-last"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-memory.grow-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-return-value")), (uint32_t)1);
      BOOST_CHECK(!bkend("as-drop-operand"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_local-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_local-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_global-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-load-operand")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-unary-operand")), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-binary-operand")), (uint32_t)12);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-test-operand")), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-operand")), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("break-bare")), (uint32_t)19);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("break-value")), (uint32_t)18);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("break-repeated")), (uint32_t)18);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("break-inner")), (uint32_t)0xf);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("effects")), (uint32_t)1);

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(br_tests) {
   try {
      auto code = backend::read_wasm( "wasms/br.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK(!bkend("type-i32"));
      BOOST_CHECK(!bkend("type-i64"));
      BOOST_CHECK(!bkend("type-f32"));
      BOOST_CHECK(!bkend("type-f64"));

      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("type-i32-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("type-i64-value")), (uint64_t)2);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("type-f32-value")), (float)3);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("type-f64-value")), (double)4);

      BOOST_CHECK(!bkend("as-block-first"));
      BOOST_CHECK(!bkend("as-block-mid"));
      BOOST_CHECK(!bkend("as-block-last"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-value")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-first")), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-mid")), (uint32_t)4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-last")), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br-value")), (uint32_t)9);
      BOOST_CHECK(!bkend("as-br_if-cond"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-value")), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-value-cond")), (uint32_t)9);
      BOOST_CHECK(!bkend("as-br_table-index"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-value")), (uint32_t)10);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-value-index")), (uint32_t)11);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("as-return-value")), (uint64_t)7);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-cond")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-then", (uint32_t)1, (uint32_t)6)), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-then", (uint32_t)0, (uint32_t)6)), (uint32_t)6);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-else", (uint32_t)0, (uint32_t)6)), (uint32_t)4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-else", (uint32_t)1, (uint32_t)6)), (uint32_t)6);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-first", (uint32_t)0, (uint32_t)6)), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-first", (uint32_t)1, (uint32_t)6)), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-second", (uint32_t)0, (uint32_t)6)), (uint32_t)6);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-second", (uint32_t)1, (uint32_t)6)), (uint32_t)6);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-cond")), (uint32_t)7);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call-first")), (uint32_t)12);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call-mid")), (uint32_t)13);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call-last")), (uint32_t)14);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-func")), (uint32_t)20);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-first")), (uint32_t)21);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-mid")), (uint32_t)22);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-last")), (uint32_t)23);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_local-value")), (uint32_t)17);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-tee_local-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_global-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("as-load-address")), (float)1.7);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("as-loadN-address")), (uint64_t)30);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-store-address")), (uint32_t)30);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-store-value")), (uint32_t)31);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-storeN-address")), (uint32_t)32);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-storeN-value")), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("as-unary-operand")), (float)3.4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-binary-left")), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("as-binary-right")), (uint64_t)45);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-test-operand")), (uint32_t)44);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-left")), (uint32_t)43);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-right")), (uint32_t)42);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-memory.grow-size")), (uint32_t)40);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-block-value")), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br-value")), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value")), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value-cond")), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value")), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value-index")), (uint32_t)9);

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(br_if_tests) {
   try {
      auto code = backend::read_wasm( "wasms/br_if.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK(!bkend("type-i32"));
      BOOST_CHECK(!bkend("type-i64"));
      BOOST_CHECK(!bkend("type-f32"));
      BOOST_CHECK(!bkend("type-f64"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("type-i32-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("type-i64-value")), (uint64_t)2);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("type-f32-value")), (float)3);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("type-f64-value")), (double)4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-first", (uint32_t)0)), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-first", (uint32_t)1)), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-mid", (uint32_t)0)), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-mid", (uint32_t)1)), (uint32_t)3);
      BOOST_CHECK(!bkend("as-block-last", (uint32_t)0));
      BOOST_CHECK(!bkend("as-block-last", (uint32_t)1));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-first-value", (uint32_t)0)), (uint32_t)11);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-first-value", (uint32_t)1)), (uint32_t)10);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-mid-value", (uint32_t)0)), (uint32_t)21);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-mid-value", (uint32_t)1)), (uint32_t)20);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-last-value", (uint32_t)0)), (uint32_t)11);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-last-value", (uint32_t)1)), (uint32_t)11);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-first", (uint32_t)0)), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-first", (uint32_t)1)), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-mid", (uint32_t)0)), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-mid", (uint32_t)1)), (uint32_t)4);
      BOOST_CHECK(!bkend("as-loop-last", (uint32_t)0));
      BOOST_CHECK(!bkend("as-loop-last", (uint32_t)1));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br-value")), (uint32_t)1);
      BOOST_CHECK(!bkend("as-br_if-cond"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-value-cond", (uint32_t)0)), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-value-cond", (uint32_t)1)), (uint32_t)1);
      BOOST_CHECK(!bkend("as-br_table-index"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-value-index")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("as-return-value")), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-cond", (uint32_t)0)), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-cond", (uint32_t)1)), (uint32_t)1);
      BOOST_CHECK(!bkend("as-if-then", (uint32_t)0, (uint32_t)0));
      BOOST_CHECK(!bkend("as-if-then", (uint32_t)4, (uint32_t)0));
      BOOST_CHECK(!bkend("as-if-then", (uint32_t)0, (uint32_t)1));
      BOOST_CHECK(!bkend("as-if-then", (uint32_t)4, (uint32_t)1));
      BOOST_CHECK(!bkend("as-if-else", (uint32_t)0, (uint32_t)0));
      BOOST_CHECK(!bkend("as-if-else", (uint32_t)3, (uint32_t)0));
      BOOST_CHECK(!bkend("as-if-else", (uint32_t)0, (uint32_t)1));
      BOOST_CHECK(!bkend("as-if-else", (uint32_t)3, (uint32_t)1));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-first", (uint32_t)0)), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-first", (uint32_t)1)), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-second", (uint32_t)0)), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-second", (uint32_t)1)), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-cond")), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call-first")), (uint32_t)12);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call-mid")), (uint32_t)13);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call-last")), (uint32_t)14);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-func")), (uint32_t)4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-first")), (uint32_t)4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-mid")), (uint32_t)4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-last")), (uint32_t)4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_local-value", (uint32_t)0)), (uint32_t)-1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_local-value", (uint32_t)1)), (uint32_t)17);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-tee_local-value", (uint32_t)0)), (uint32_t)-1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-tee_local-value", (uint32_t)1)), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_global-value", (uint32_t)0)), (uint32_t)-1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_global-value", (uint32_t)1)), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-load-address")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loadN-address")), (uint32_t)30);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-store-address")), (uint32_t)30);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-store-value")), (uint32_t)31);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-storeN-address")), (uint32_t)32);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-storeN-value")), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("as-unary-operand")), (double)1.0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-binary-left")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-binary-right")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-test-operand")), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-left")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-right")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-memory.grow-size")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-block-value", (uint32_t)0)), (uint32_t)21);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-block-value", (uint32_t)1)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br-value", (uint32_t)0)), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br-value", (uint32_t)1)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value", (uint32_t)0)), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value", (uint32_t)1)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value-cond", (uint32_t)0)), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value-cond", (uint32_t)1)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value", (uint32_t)0)), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value", (uint32_t)1)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value-index", (uint32_t)0)), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value-index", (uint32_t)1)), (uint32_t)9);
   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(br_table_tests) {
   try {
      auto code = backend::read_wasm( "wasms/br_table.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK(!bkend("type-i32"));
      BOOST_CHECK(!bkend("type-i64"));
      BOOST_CHECK(!bkend("type-f32"));
      BOOST_CHECK(!bkend("type-f64"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("type-i32-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("type-i64-value")), (uint64_t)2);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("type-f32-value")), (float)3);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("type-f64-value")), (double)4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty", (uint32_t)0)), (uint32_t)22);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty", (uint32_t)1)), (uint32_t)22);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty", (uint32_t)11)), (uint32_t)22);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty", (uint32_t)-1)), (uint32_t)22);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty", (uint32_t)-100)), (uint32_t)22);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty", (uint32_t)0xffffffff)), (uint32_t)22);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty-value", (uint32_t)0)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty-value", (uint32_t)1)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty-value", (uint32_t)11)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty-value", (uint32_t)-1)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty-value", (uint32_t)-100)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("empty-value", (uint32_t)0xffffffff)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton", (uint32_t)0)), (uint32_t)22);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton", (uint32_t)1)), (uint32_t)20);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton", (uint32_t)11)), (uint32_t)20);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton", (uint32_t)-1)), (uint32_t)20);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton", (uint32_t)-100)), (uint32_t)20);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton", (uint32_t)0xffffffff)), (uint32_t)20);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton-value", (uint32_t)0)), (uint32_t)32);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton-value", (uint32_t)1)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton-value", (uint32_t)11)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton-value", (uint32_t)-1)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton-value", (uint32_t)-100)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("singleton-value", (uint32_t)0xffffffff)), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple", (uint32_t)0)), (uint32_t)103);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple", (uint32_t)1)), (uint32_t)102);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple", (uint32_t)2)), (uint32_t)101);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple", (uint32_t)3)), (uint32_t)100);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple", (uint32_t)4)), (uint32_t)104);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple", (uint32_t)5)), (uint32_t)104);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple", (uint32_t)6)), (uint32_t)104);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple", (uint32_t)10)), (uint32_t)104);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple", (uint32_t)-1)), (uint32_t)104);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple", (uint32_t)0xffffffff)), (uint32_t)104);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple-value", (uint32_t)0)), (uint32_t)213);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple-value", (uint32_t)1)), (uint32_t)212);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple-value", (uint32_t)2)), (uint32_t)211);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple-value", (uint32_t)3)), (uint32_t)210);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple-value", (uint32_t)4)), (uint32_t)214);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple-value", (uint32_t)5)), (uint32_t)214);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple-value", (uint32_t)6)), (uint32_t)214);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple-value", (uint32_t)10)), (uint32_t)214);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple-value", (uint32_t)-1)), (uint32_t)214);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("multiple-value", (uint32_t)0xffffffff)), (uint32_t)214);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("large", (uint32_t)0)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("large", (uint32_t)1)), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("large", (uint32_t)100)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("large", (uint32_t)101)), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("large", (uint32_t)10000)), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("large", (uint32_t)10001)), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("large", (uint32_t)1000000)), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("large", (uint32_t)1000001)), (uint32_t)1);
      BOOST_CHECK(!bkend("as-block-first"));
      BOOST_CHECK(!bkend("as-block-mid"));
      BOOST_CHECK(!bkend("as-block-last"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-block-value")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-first")), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-mid")), (uint32_t)4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-loop-last")), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br-value")), (uint32_t)9);
      BOOST_CHECK(!bkend("as-br_if-cond"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-value")), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-value-cond")), (uint32_t)9);
      BOOST_CHECK(!bkend("as-br_table-index"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-value")), (uint32_t)10);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-value-index")), (uint32_t)11);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("as-return-value")), (uint64_t)7);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-cond")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-then", (uint32_t)1, (uint32_t)6)), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-then", (uint32_t)0, (uint32_t)6)), (uint32_t)6);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-else", (uint32_t)0, (uint32_t)6)), (uint32_t)4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-else", (uint32_t)1, (uint32_t)6)), (uint32_t)6);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-first", (uint32_t)0, (uint32_t)6)), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-first", (uint32_t)1, (uint32_t)6)), (uint32_t)5);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-second", (uint32_t)0, (uint32_t)6)), (uint32_t)6);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-second", (uint32_t)1, (uint32_t)6)), (uint32_t)6);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-cond")), (uint32_t)7);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call-first")), (uint32_t)12);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call-mid")), (uint32_t)13);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call-last")), (uint32_t)14);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-first")), (uint32_t)20);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-mid")), (uint32_t)21);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-last")), (uint32_t)22);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-func")), (uint32_t)23);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_local-value")), (uint32_t)17);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-tee_local-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_global-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("as-load-address")), (float)1.7);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("as-loadN-address")), (uint64_t)30);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-store-address")), (uint32_t)30);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-store-value")), (uint32_t)31);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-storeN-address")), (uint32_t)32);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-storeN-value")), (uint32_t)33);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("as-unary-operand")), (float)3.4);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-binary-left")), (uint32_t)3);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("as-binary-right")), (uint64_t)45);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-test-operand")), (uint32_t)44);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-left")), (uint32_t)43);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-right")), (uint32_t)42);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-convert-operand")), (uint32_t)41);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-memory.grow-size")), (uint32_t)40);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-block-value", (uint32_t)0)), (uint32_t)19);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-block-value", (uint32_t)1)), (uint32_t)17);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-block-value", (uint32_t)2)), (uint32_t)16);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-block-value", (uint32_t)10)), (uint32_t)16);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-block-value", (uint32_t)-1)), (uint32_t)16);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-block-value", (uint32_t)100000)), (uint32_t)16);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br-value", (uint32_t)0)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br-value", (uint32_t)1)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br-value", (uint32_t)2)), (uint32_t)17);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br-value", (uint32_t)11)), (uint32_t)17);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br-value", (uint32_t)-4)), (uint32_t)17);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br-value", (uint32_t)10213210)), (uint32_t)17);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value", (uint32_t)0)), (uint32_t)17);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value", (uint32_t)1)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value", (uint32_t)2)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value", (uint32_t)9)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value", (uint32_t)-9)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value", (uint32_t)999999)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value-cond", (uint32_t)0)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value-cond", (uint32_t)1)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value-cond", (uint32_t)2)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value-cond", (uint32_t)3)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value-cond", (uint32_t)-1000000)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_if-value-cond", (uint32_t)9423975)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value", (uint32_t)0)), (uint32_t)17);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value", (uint32_t)1)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value", (uint32_t)2)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value", (uint32_t)9)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value", (uint32_t)-9)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value", (uint32_t)999999)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value-index", (uint32_t)0)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value-index", (uint32_t)1)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value-index", (uint32_t)2)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value-index", (uint32_t)3)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value-index", (uint32_t)-1000000)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-value-index", (uint32_t)9423975)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("nested-br_table-loop-block", (uint32_t)1)), (uint32_t)3);

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(break_drop_tests) {
   try {
      auto code = backend::read_wasm( "wasms/break_drop.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK(!bkend("br"));
      BOOST_CHECK(!bkend("br_if"));
      BOOST_CHECK(!bkend("br_table"));

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(call_tests) {
   try {
      auto code = backend::read_wasm( "wasms/call.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("type-i32")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("type-i64")), (uint64_t)0x164);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("type-f32")), (float)0xf32);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("type-f64")), (double)0xf64);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("type-first-i32")), (uint32_t)32);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("type-first-i64")), (uint64_t)64);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("type-first-f32")), (float)1.32);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("type-first-f64")), (double)1.64);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("type-second-i32")), (uint32_t)32);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("type-second-i64")), (uint64_t)64);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("type-second-f32")), (float)32);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("type-second-f64")), (double)64.1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac", (uint64_t)0)), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac", (uint64_t)1)), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac", (uint64_t)2)), (uint64_t)2);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac", (uint64_t)5)), (uint64_t)120);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac", (uint64_t)25)), (uint64_t)7034535277573963776);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac-acc", (uint64_t)0, (uint64_t)1)), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac-acc", (uint64_t)1, (uint64_t)1)), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac-acc", (uint64_t)5, (uint64_t)1)), (uint64_t)120);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac-acc", (uint64_t)25, (uint64_t)1)), (uint64_t)7034535277573963776);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fib", (uint64_t)0)), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fib", (uint64_t)1)), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fib", (uint64_t)2)), (uint64_t)2);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fib", (uint64_t)5)), (uint64_t)8);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fib", (uint64_t)20)), (uint64_t)10946);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("even", (uint64_t)0)), (uint32_t)44);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("even", (uint64_t)1)), (uint32_t)99);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("even", (uint64_t)100)), (uint32_t)44);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("even", (uint64_t)77)), (uint32_t)99);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("odd", (uint64_t)0)), (uint32_t)99);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("odd", (uint64_t)1)), (uint32_t)44);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("odd", (uint64_t)200)), (uint32_t)99);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("odd", (uint64_t)77)), (uint32_t)44);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-first")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-mid")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-last")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-condition")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-first")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-last")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-first")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-last")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-first")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-call_indirect-mid")), (uint32_t)2);
      BOOST_CHECK(!bkend("as-store-first"));
      BOOST_CHECK(!bkend("as-store-last"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-memory.grow-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-return-value")), (uint32_t)0x132);
      BOOST_CHECK(!bkend("as-drop-operand"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br-value")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_local-value")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-tee_local-value")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-set_global-value")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-load-operand")), (uint32_t)'A');
      BOOST_CHECK_EQUAL(TO_F32(*bkend("as-unary-operand")), (float)0x0p+0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-binary-left")), (uint32_t)11);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-binary-right")), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-test-operand")), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-left")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-right")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("as-convert-operand")), (uint64_t)1);

      //bkend("runaway");
   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(call_indirect_tests) {
   try {
      auto code = backend::read_wasm( "wasms/call_indirect.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("type-i32")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("type-i64")), (uint64_t)0x164);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("type-f32")), (float)0xf32);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("type-f64")), (double)0xf64);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("type-index")), (uint64_t)100);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("type-first-i32")), (uint32_t)32);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("type-first-i64")), (uint64_t)64);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("type-first-f32")), (float)1.32);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("type-first-f64")), (double)1.64);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("type-second-i32")), (uint32_t)32);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("type-second-i64")), (uint64_t)64);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("type-second-f32")), (float)32);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("type-second-f64")), (double)64.1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("dispatch", (uint32_t)5, (uint64_t)2)), (uint64_t)2);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("dispatch", (uint32_t)5, (uint64_t)5)), (uint64_t)5);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("dispatch", (uint32_t)12, (uint64_t)5)), (uint64_t)120);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("dispatch", (uint32_t)13, (uint64_t)5)), (uint64_t)8);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("dispatch", (uint32_t)20, (uint64_t)2)), (uint64_t)2);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("dispatch-structural-i64", (uint32_t)5)), (uint64_t)9);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("dispatch-structural-i64", (uint32_t)12)), (uint64_t)362880);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("dispatch-structural-i64", (uint32_t)13)), (uint64_t)55);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("dispatch-structural-i64", (uint32_t)20)), (uint64_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("dispatch-structural-i32", (uint32_t)4)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("dispatch-structural-i32", (uint32_t)23)), (uint32_t)362880);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("dispatch-structural-i32", (uint32_t)26)), (uint32_t)55);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("dispatch-structural-i32", (uint32_t)19)), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("dispatch-structural-f32", (uint32_t)6)), (float)9.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("dispatch-structural-f32", (uint32_t)24)), (float)362880.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("dispatch-structural-f32", (uint32_t)27)), (float)55.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("dispatch-structural-f32", (uint32_t)21)), (float)9.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("dispatch-structural-f64", (uint32_t)7)), (double)9.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("dispatch-structural-f64", (uint32_t)25)), (double)362880.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("dispatch-structural-f64", (uint32_t)28)), (double)55.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("dispatch-structural-f64", (uint32_t)22)), (double)9.0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac-i64", (uint64_t)0)), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac-i64", (uint64_t)1)), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac-i64", (uint64_t)5)), (uint64_t)120);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fac-i64", (uint64_t)25)), (uint64_t)7034535277573963776);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("fac-i32", (uint32_t)0)), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("fac-i32", (uint32_t)1)), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("fac-i32", (uint32_t)5)), (uint32_t)120);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("fac-i32", (uint32_t)10)), (uint32_t)3628800);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("fac-f32", (float)0.0)), (float)1.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("fac-f32", (float)1.0)), (float)1.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("fac-f32", (float)5.0)), (float)120.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("fac-f32", (float)10.0)), (float)3628800.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("fac-f64", (double)0.0)), (double)1.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("fac-f64", (double)1.0)), (double)1.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("fac-f64", (double)5.0)), (double)120.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("fac-f64", (double)10.0)), (double)3628800.0);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fib-i64", (uint64_t)0)), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fib-i64", (uint64_t)1)), (uint64_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fib-i64", (uint64_t)2)), (uint64_t)2);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fib-i64", (uint64_t)5)), (uint64_t)8);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("fib-i64", (uint64_t)20)), (uint64_t)10946);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("fib-i32", (uint32_t)0)), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("fib-i32", (uint32_t)1)), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("fib-i32", (uint32_t)2)), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("fib-i32", (uint32_t)5)), (uint32_t)8);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("fib-i32", (uint32_t)20)), (uint32_t)10946);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("fib-f32", (float)0.0)), (float)1.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("fib-f32", (float)1.0)), (float)1.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("fib-f32", (float)2.0)), (float)2.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("fib-f32", (float)5.0)), (float)8.0);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("fib-f32", (float)20.0)), (float)10946.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("fib-f64", (double)0.0)), (double)1.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("fib-f64", (double)1.0)), (double)1.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("fib-f64", (double)2.0)), (double)2.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("fib-f64", (double)5.0)), (double)8.0);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("fib-f64", (double)20.0)), (double)10946.0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("even", (uint32_t)0)), (uint32_t)44);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("even", (uint32_t)1)), (uint32_t)99);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("even", (uint32_t)100)), (uint32_t)44);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("even", (uint32_t)77)), (uint32_t)99);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("odd", (uint32_t)0)), (uint32_t)99);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("odd", (uint32_t)1)), (uint32_t)44);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("odd", (uint32_t)200)), (uint32_t)99);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("odd", (uint32_t)77)), (uint32_t)44);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-first")), (uint32_t)0x132);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-mid")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-select-last")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-if-condition")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("as-br_if-first")), (uint64_t)0x164);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_if-last")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("as-br_table-first")), (float)0xf32);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-br_table-last")), (uint32_t)2);
      BOOST_CHECK(!bkend("as-store-first"));
      BOOST_CHECK(!bkend("as-store-last"));
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-memory.grow-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-return-value")), (uint32_t)1);
      BOOST_CHECK(!bkend("as-drop-operand"));
      BOOST_CHECK_EQUAL(TO_F32(*bkend("as-br-value")), (float)1);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("as-set_local-value")), (double)1);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("as-tee_local-value")), (double)1);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("as-set_global-value")), (double)1.0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-load-operand")), (uint32_t)'A');
      BOOST_CHECK_EQUAL(TO_F32(*bkend("as-unary-operand")), (float)0x0p+0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-binary-left")), (uint32_t)11);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-binary-right")), (uint32_t)9);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-test-operand")), (uint32_t)0);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-left")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT32(*bkend("as-compare-right")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*bkend("as-convert-operand")), (uint64_t)1);

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(conversions_tests) {
   try {
      auto code = backend::read_wasm( "wasms/conversions.wasm" );
         backend bkend( code, wa );

         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_s", (uint32_t)0)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_s", (uint32_t)10000)), (uint64_t)10000);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_s", (uint32_t)-10000)), (uint64_t)-10000);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_s", (uint32_t)-1)), (uint64_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_s", (uint32_t)0x7fffffff)), (uint64_t)0x000000007fffffff);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_s", (uint32_t)0x80000000)), (uint64_t)0xffffffff80000000);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_u", (uint32_t)0)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_u", (uint32_t)10000)), (uint64_t)10000);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_u", (uint32_t)-10000)), (uint64_t)0x00000000ffffd8f0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_u", (uint32_t)-1)), (uint64_t)0xffffffff);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_u", (uint32_t)0x7fffffff)), (uint64_t)0x000000007fffffff);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.extend_i32_u", (uint32_t)0x80000000)), (uint64_t)0x0000000080000000);

         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)-1)), (uint32_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)-100000)), (uint32_t)-100000);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)0x80000000)), (uint32_t)0x80000000);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)0xffffffff7fffffff)), (uint32_t)0x7fffffff);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)0xffffffff00000000)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)0xfffffffeffffffff)), (uint32_t)0xffffffff);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)0xffffffff00000001)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)0)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)1311768467463790320)), (uint32_t)0x9abcdef0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)0x00000000ffffffff)), (uint32_t)0xffffffff);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)0x0000000100000000)), (uint32_t)0x00000000);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.wrap_i64", (uint64_t)0x0000000100000001)), (uint32_t)0x00000001);

         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)0.0)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)-0.0)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)0x1p-149)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)-0x1p-149)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)1.0)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)0x1.19999ap+0)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)1.5)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)-1.0)), (uint32_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)-0x1.19999ap+0)), (uint32_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)-1.5)), (uint32_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)-1.9)), (uint32_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)-2.0)), (uint32_t)-2);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)2147483520.0)), (uint32_t)2147483520);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_s", (float)-2147483648.0)), (uint32_t)-2147483648);

         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)0.0)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)-0.0)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)0x1p-149)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)-0x1p-149)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)1.0)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)0x1.19999ap+0)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)1.5)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)1.9)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)2.0)), (uint32_t)2);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)2147483648)), (uint32_t)-2147483648);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)4294967040.0)), (uint32_t)-256);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)-0x1.ccccccp-1)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f32_u", (float)-0x1.fffffep-1)), (uint32_t)0);

         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)0.0)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)-0.0)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)0x0.0000000000001p-1022)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)-0x0.0000000000001p-1022)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)1.0)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)0x1.199999999999ap+0)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)1.5)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)-1.0)), (uint32_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)-0x1.199999999999ap+0)), (uint32_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)-1.5)), (uint32_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)-1.9)), (uint32_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)-2.0)), (uint32_t)-2);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)2147483647.0)), (uint32_t)2147483647);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_s", (double)-2147483648.0)), (uint32_t)-2147483648);

         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)0.0)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)-0.0)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)0x0.0000000000001p-1022)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)-0x0.0000000000001p-1022)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)1.0)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)0x1.199999999999ap+0)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)1.5)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)1.9)), (uint32_t)1);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)2.0)), (uint32_t)2);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)2147483648)), (uint32_t)2147483648);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)4294967295.0)), (uint32_t)4294967295);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)-0x1.ccccccccccccdp-1)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)-0x1.fffffffffffffp-1)), (uint32_t)0);
         BOOST_CHECK_EQUAL(TO_UINT32(*bkend("i32.trunc_f64_u", (double)1e8)), (uint32_t)100000000);

         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)0.0)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)-0.0)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)0x1p-149)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)-0x1p-149)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)1.0)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)0x1.19999ap+0)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)1.5)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)-1.0)), (uint64_t)-1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)-0x1.19999ap+0)), (uint64_t)-1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)-1.5)), (uint64_t)-1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)-1.9)), (uint64_t)-1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)-2.0)), (uint64_t)-2);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)4294967296)), (uint64_t)4294967296);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)-4294967296)), (uint64_t)-4294967296);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)9223371487098961920.0)), (uint64_t)9223371487098961920ull);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_s", (float)-9223372036854775808.0)), (uint64_t)-9223372036854775808ll);

         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)0.0)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)-0.0)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)0x1p-149)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)-0x1p-149)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)1.0)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)0x1.19999ap+0)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)1.5)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)4294967296)), (uint64_t)4294967296);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)18446742974197923840.0)), (uint64_t)18446742974197923840ull);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)-0x1.ccccccp-1)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("i64.trunc_f32_u", (float)-0x1.fffffep-1)), (uint64_t)0);

         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)0.0)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)-0.0)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)0x0.0000000000001p-1022)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)-0x0.0000000000001p-1022)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)1.0)), (uint64_t)1.0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)0x1.199999999999ap+0)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)1.5)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)-1.0)), (uint64_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)-0x1.199999999999ap+0)), (uint64_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)-1.5)), (uint64_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)-1.9)), (uint64_t)-1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)-2.0)), (uint64_t)-2);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)4294967296)), (uint64_t)4294967296);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)-4294967296)), (uint64_t)-4294967296);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)9223372036854774784.0)), (uint64_t)9223372036854774784);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_s", (double)-9223372036854775808.0)), (uint64_t)-9223372036854775808);

         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)0.0)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)-0.0)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)0x0.0000000000001p-1022)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)-0x0.0000000000001p-1022)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)1.0)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)0x1.199999999999ap+0)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)1.5)), (uint64_t)1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)4294967295)), (uint64_t)4294967295);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)4294967296)), (uint64_t)4294967296);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)18446744073709549568.0)), (uint64_t)18446744073709549568);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)-0x1.ccccccccccccdp-1)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)-0x1.fffffffffffffp-1)), (uint64_t)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)1e8)), (uint64_t)1000000000);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)1e16)), (uint64_t)1000000000000000000);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("i64.trunc_f64_u", (double)9223372036854775808)), (uint64_t)-9223372036854775808);

         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_s", (uint32_t)1)), (float)1.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_s", (uint32_t)-1)), (float)-1.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_s", (uint32_t)0)), (float)0.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_s", (uint32_t)2147483647)), (float)2147483648);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_s", (uint32_t)-2147483648)), (float)-2147483648);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_s", (uint32_t)1234567890)), (float)0x1.26580cp+30);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_s", (uint32_t)16777217)), (float)16777216.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_s", (uint32_t)-16777217)), (float)-16777216.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_s", (uint32_t)16777219)), (float)16777220.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_s", (uint32_t)-16777219)), (float)-16777220.0);

         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_s", (uint64_t)1)), (float)1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_s", (uint64_t)-1)), (float)-1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_s", (uint64_t)0)), (float)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_s", (uint64_t)9223372036854775807)), (float)9223372036854775807);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_s", (uint64_t)-9223372036854775808)), (float)-9223372036854775808);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_s", (uint64_t)314159265358979)), (float)0x1.1db9e8p+48);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_s", (uint64_t)16777217)), (float)16777216.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_s", (uint64_t)-16777217)), (float)-16777216.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_s", (uint64_t)16777219)), (float)16777220);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_s", (uint64_t)-16777219)), (float)-16777220);

         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_s", (uint32_t)1)), (double)1.0);
         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_s", (uint32_t)-1)), (double)-1.0);
         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_s", (uint32_t)0)), (double)0.0);
         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_s", (uint32_t)2147483647)), (double)2147483647);
         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_s", (uint32_t)-2147483648)), (double)-2147483648);
         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_s", (uint32_t)987654321)), (double)987654321);

         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_s", (uint64_t)1)), (double)1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_s", (uint64_t)-1)), (double)-1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_s", (uint64_t)0)), (double)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_s", (uint64_t)9223372036854775807)), (double)9223372036854775807);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_s", (uint64_t)-9223372036854775808)), (double)-9223372036854775808);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_s", (uint64_t)4669201609102990)), (double)4669201609102990);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_s", (uint64_t)9007199254740993)), (double)9007199254740992);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_s", (uint64_t)-9007199254740993)), (double)-9007199254740992);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_s", (uint64_t)9007199254740995)), (double)9007199254740996);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_s", (uint64_t)-9007199254740995)), (double)-9007199254740996);

         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)1)), (float)1.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)0)), (float)0.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)2147483647)), (float)2147483648);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)-2147483648)), (float)2147483648);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)0x12345678)), (float)0x1.234568p+28);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)0xffffffff)), (float)4294967296.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)0x80000080)), (float)0x1.000000p+31);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)0x80000081)), (float)0x1.000002p+31);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)0x80000082)), (float)0x1.000002p+31);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)0xfffffe80)), (float)0x1.fffffcp+31);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)0xfffffe81)), (float)0x1.fffffep+31);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)0xfffffe82)), (float)0x1.fffffep+31);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)16777217)), (float)16777216.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i32_u", (uint32_t)16777219)), (float)16777220.0);

         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_u", (uint64_t)1)), (float)1);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_u", (uint64_t)0)), (float)0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_u", (uint64_t)9223372036854775807)), (float)9223372036854775807);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_u", (uint64_t)-9223372036854775808)), (float)-9223372036854775808);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_u", (uint64_t)0xffffffffffffffff)), (float)18446744073709551616.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_u", (uint64_t)16777217)), (float)16777216);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f32.convert_i64_u", (uint64_t)16777219)), (float)16777220);

         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_u", (uint32_t)1)), (double)1.0);
         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_u", (uint32_t)0)), (double)0.0);
         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_u", (uint32_t)2147483647)), (double)2147483647);
         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_u", (uint32_t)-2147483648)), (double)2147483648);
         BOOST_CHECK_EQUAL(TO_F64(*bkend("f64.convert_i32_u", (uint32_t)0xffffffff)), (double)4294967295.0);

         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)1)), (double)1);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)0)), (double)0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)9223372036854775807)), (double)9223372036854775807);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)-9223372036854775808)), (double)9223372036854775808);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)0xffffffffffffffff)), (double)18446744073709551616.0);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)0x8000000000000400)), (double)0x1.0000000000000p+63);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)0x8000000000000401)), (double)0x1.0000000000001p+63);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)0x8000000000000402)), (double)0x1.0000000000001p+63);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)0xfffffffffffff400)), (double)0x1.ffffffffffffep+63);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)0xfffffffffffff401)), (double)0x1.fffffffffffffp+63);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)0xfffffffffffff402)), (double)0x1.fffffffffffffp+63);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)9007199254740993)), (double)9007199254740992);
         BOOST_CHECK_EQUAL(TO_UINT64(*bkend("f64.convert_i64_u", (uint64_t)9007199254740995)), (double)9007199254740996);

         BOOST_CHECK_EQUAL(TO_F32(*bkend("f64.promote_f32", (float)0.0)), (double)0.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f64.promote_f32", (float)-0.0)), (double)-0.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f64.promote_f32", (float)0x1p-149)), (double)0x1p-149);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f64.promote_f32", (float)-0x1p-149)), (double)-0x1p-149);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f64.promote_f32", (float)1.0)), (double)1.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f64.promote_f32", (float)-1.0)), (double)-1.0);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f64.promote_f32", (float)-0x1.fffffep+127)), (double)-0x1.fffffep+127);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f64.promote_f32", (float)0x1.fffffep+127)), (double)0x1.fffffep+127);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f64.promote_f32", (float)0x1p-119)), (double)0x1p-119);
         BOOST_CHECK_EQUAL(TO_F32(*bkend("f64.promote_f32", (float)0x1.8f867ep+125)), (double)6.6382536710104395e+37);

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(endianness_tests) {
   try {
      auto code = backend::read_wasm( "wasms/endianness.wasm" );
      backend bkend( code, wa );

      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load16_s", (int32_t)-1)), (int32_t)65535);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load16_s", (int32_t)-4242)), (int32_t)61294);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load16_s", (int32_t)42)), (int32_t)42);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load16_s", (int32_t)0x3210)), (int32_t)0x3210);

      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load16_u", (int32_t)-1)), (uint16_t)0xFFFF);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load16_u", (int32_t)-4242)), (uint16_t)61294);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load16_u", (int32_t)42)), (uint16_t)42);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load16_u", (int32_t)0xCAFE)), (uint16_t)0xCAFE);

      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load", (int32_t)-1)), (uint32_t)-1);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load", (int32_t)-42424242)), (uint32_t)-42424242);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load", (int32_t)42424242)), (uint32_t)42424242);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_load", (int32_t)0xABAD1DEA)), (uint32_t)0xABAD1DEA);

      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load16_s", (int64_t)-1)), (int32_t)65535);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load16_s", (int64_t)-4242)), (int32_t)61294);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load16_s", (int64_t)42)), (int32_t)42);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load16_s", (int64_t)0x3210)), (int32_t)0x3210);

      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load16_u", (int64_t)-1)), (uint32_t)65535);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load16_u", (int64_t)-4242)), (uint32_t)61294);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load16_u", (int64_t)42)), (uint32_t)42);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load16_u", (int64_t)0xCAFE)), (uint32_t)0xCAFE);

      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load32_s", (int64_t)-1)), (int64_t)-1);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load32_s", (int64_t)-42424242)), (int64_t)-42424242);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load32_s", (int64_t)42424242)), (int64_t)42424242);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load32_s", (int64_t)0x12345678)), (int64_t)0x12345678);

      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load32_u", (int64_t)-1)), (uint32_t)-1);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load32_u", (int64_t)-42424242)), (uint32_t)-42424242);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load32_u", (int64_t)42424242)), (uint32_t)42424242);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load32_u", (int64_t)0xABAD1DEA)), (uint32_t)0xABAD1DEA);

      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load", (int64_t)-1)), (uint64_t)-1);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load", (int64_t)-42424242)), (uint64_t)-42424242);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load", (int64_t)0xABAD1DEA)), (uint64_t)0xABAD1DEA);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_load", (int64_t)0xABADCAFEDEAD1DEA)), (uint64_t)0xABADCAFEDEAD1DEA);

      BOOST_CHECK_EQUAL(TO_F32(*bkend("f32_load", (float)-1)), (float)-1);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("f32_load", (float)1234e-5)), (float)1234e-5);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("f32_load", (float)4242.4242)), (float)4242.4242);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("f32_load", (float)0x1.fffffep+127)), (float)0x1.fffffep+127);

      BOOST_CHECK_EQUAL(TO_F64(*bkend("f64_load", (double)-1)), (double)-1);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("f64_load", (double)123456789e-5)), (double)123456789e-5);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("f64_load", (double)424242.424242)), (double)424242.424242);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("f64_load", (double)0x1.fffffffffffffp+1023)), (double)0x1.fffffffffffffp+1023);

      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_store16", (int32_t)-1)), (uint32_t)0xFFFF);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_store16", (int32_t)-4242)), (uint32_t)61294);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_store16", (int32_t)42)), (uint32_t)42);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_store16", (int32_t)0xCAFE)), (uint32_t)0xCAFE);

      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_store", (int32_t)-1)), (uint32_t)-1);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_store", (int32_t)-4242)), (uint32_t)-4242);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_store", (int32_t)42424242)), (uint32_t)42424242);
      BOOST_CHECK_EQUAL(TO_INT32(*bkend("i32_store", (int32_t)0xDEADCAFE)), (uint32_t)0xDEADCAFE);

      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store16", (int64_t)-1)), (uint64_t)-1);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store16", (int64_t)-4242)), (uint64_t)-4242);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store16", (int64_t)42)), (uint64_t)42);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store16", (int64_t)0xCAFE)), (uint64_t)0xCAFE);

      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store32", (int64_t)-1)), (uint64_t)-1);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store32", (int64_t)-4242)), (uint64_t)-4242);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store32", (int64_t)42424242)), (uint64_t)42424242);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store32", (int64_t)0xDEADCAFE)), (uint64_t)0xDEADCAFE);

      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store", (int64_t)-1)), (uint64_t)-1);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store", (int64_t)-42424242)), (uint64_t)-42424242);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store", (int64_t)0xABAD1DEA)), (uint64_t)0xABAD1DEA);
      BOOST_CHECK_EQUAL(TO_INT64(*bkend("i64_store", (int64_t)0xABADCAFEDEAD1DEA)), (uint64_t)0xABADCAFEDEAD1DEA);

      BOOST_CHECK_EQUAL(TO_F32(*bkend("f32_store", (float)-1)), (float)-1);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("f32_store", (float)1234e-5)), (float)1234e-5);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("f32_store", (float)4242.4242)), (float)4242.4242);
      BOOST_CHECK_EQUAL(TO_F32(*bkend("f32_store", (float)0x1.fffffep+127)), (float)0x1.fffffep+127);

      BOOST_CHECK_EQUAL(TO_F64(*bkend("f64_store", (double)-1)), (double)-1);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("f64_store", (double)123456789e-5)), (double)123456789e-5);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("f64_store", (double)424242.424242)), (double)424242.424242);
      BOOST_CHECK_EQUAL(TO_F64(*bkend("f64_store", (double)0x1.fffffffffffffp+1023)), (double)0x1.fffffffffffffp+1023);

   } FC_LOG_AND_RETHROW()
}
BOOST_AUTO_TEST_SUITE_END()
