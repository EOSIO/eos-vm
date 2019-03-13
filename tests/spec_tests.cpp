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
   // 32 bits
   try {
      memory_manager::set_memory_limits( 32*1024*1024 );
      module mod;
      auto ctx = create_execution_context<interpret_visitor>("wasms/address.wasm", mod);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8u_good1", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8u_good2", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8u_good3", (uint32_t)0)), 'b');
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8u_good4", (uint32_t)0)), 'c');
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8u_good5", (uint32_t)0)), 'z');

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8s_good1", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8s_good2", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8s_good3", (uint32_t)0)), 'b');
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8s_good4", (uint32_t)0)), 'c');
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8s_good5", (uint32_t)0)), 'z');

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16u_good1", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16u_good2", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16u_good3", (uint32_t)0)), 25442);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16u_good4", (uint32_t)0)), 25699);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16u_good5", (uint32_t)0)), 122);

      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16s_good1", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16s_good2", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16s_good3", (uint32_t)0)), 25442);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16s_good4", (uint32_t)0)), 25699);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16s_good5", (uint32_t)0)), 122);

      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good1", (uint32_t)0)), 1684234849);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good2", (uint32_t)0)), 1684234849);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good3", (uint32_t)0)), 1701077858);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good4", (uint32_t)0)), 1717920867);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good5", (uint32_t)0)), 122);

      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8u_good1", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8u_good2", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8u_good3", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8u_good4", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8u_good5", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8s_good1", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8s_good2", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8s_good3", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8s_good4", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("8s_good5", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16u_good1", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16u_good2", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16u_good3", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16u_good4", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16u_good5", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16s_good1", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16s_good2", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16s_good3", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16s_good4", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("16s_good5", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good1", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good2", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good3", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good4", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good5", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("32_good5", (uint32_t)65507)), 0);

      BOOST_CHECK_THROW(ctx.execute("32_good5", (uint32_t)65508), wasm_memory_exception);
      
      BOOST_CHECK_THROW(ctx.execute("8u_bad", (uint32_t)0), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("8u_bad", (uint32_t)0), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("16u_bad", (uint32_t)0), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("16u_bad", (uint32_t)0), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("32_bad", (uint32_t)0), wasm_memory_exception);

      BOOST_CHECK_THROW(ctx.execute("8u_bad", (uint32_t)1), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("8u_bad", (uint32_t)1), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("16u_bad", (uint32_t)1), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("16u_bad", (uint32_t)1), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("32_bad", (uint32_t)1), wasm_memory_exception);

      // 64 bits
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8u_good641", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8u_good642", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8u_good643", (uint32_t)0)), 'b');
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8u_good644", (uint32_t)0)), 'c');
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8u_good645", (uint32_t)0)), 'z');

      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8s_good641", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8s_good642", (uint32_t)0)), 'a');
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8s_good643", (uint32_t)0)), 'b');
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8s_good644", (uint32_t)0)), 'c');
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8s_good645", (uint32_t)0)), 'z');

      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16u_good641", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16u_good642", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16u_good643", (uint32_t)0)), 25442);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16u_good644", (uint32_t)0)), 25699);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16u_good645", (uint32_t)0)), 122);

      
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16s_good641", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16s_good642", (uint32_t)0)), 25185);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16s_good643", (uint32_t)0)), 25442);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16s_good644", (uint32_t)0)), 25699);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16s_good645", (uint32_t)0)), 122);

      
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good641", (uint32_t)0)), 1684234849);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good642", (uint32_t)0)), 1684234849);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good643", (uint32_t)0)), 1701077858);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good644", (uint32_t)0)), 1717920867);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good645", (uint32_t)0)), 122);

      
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8u_good641", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8u_good642", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8u_good643", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8u_good644", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8u_good645", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8s_good641", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8s_good642", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8s_good643", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8s_good644", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("8s_good645", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16u_good641", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16u_good642", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16u_good643", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16u_good644", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16u_good645", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16s_good641", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16s_good642", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16s_good643", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16s_good644", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("16s_good645", (uint32_t)65507)), 0);

      
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good641", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good642", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good643", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good644", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good645", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32u_good645", (uint32_t)65507)), 0);

      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32s_good641", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32s_good642", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32s_good643", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32s_good644", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32s_good645", (uint32_t)65507)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("32s_good645", (uint32_t)65507)), 0);

      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("64_good641", (uint32_t)65503)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("64_good642", (uint32_t)65503)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("64_good643", (uint32_t)65503)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("64_good644", (uint32_t)65503)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("64_good645", (uint32_t)65503)), 0);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("64_good645", (uint32_t)65503)), 0);

      BOOST_CHECK_THROW(ctx.execute("32u_good645", (uint32_t)65508), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("32s_good645", (uint32_t)65508), wasm_memory_exception);
      
      BOOST_CHECK_THROW(ctx.execute("64_good645", (uint32_t)65504), wasm_memory_exception);

      BOOST_CHECK_THROW(ctx.execute("8u_bad64", (uint32_t)0), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("8u_bad64", (uint32_t)0), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("16u_bad64", (uint32_t)0), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("16u_bad64", (uint32_t)0), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("32u_bad64", (uint32_t)0), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("32s_bad64", (uint32_t)0), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("64_bad64", (uint32_t)0), wasm_memory_exception);

      BOOST_CHECK_THROW(ctx.execute("8u_bad64", (uint32_t)1), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("8u_bad64", (uint32_t)1), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("16u_bad64", (uint32_t)1), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("16u_bad64", (uint32_t)1), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("32u_bad64", (uint32_t)1), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("32s_bad64", (uint32_t)1), wasm_memory_exception);
      BOOST_CHECK_THROW(ctx.execute("64_bad64", (uint32_t)1), wasm_memory_exception);

      // f32
      BOOST_CHECK_EQUAL(TO_F32(*ctx.execute("f32_good1", (uint32_t)65503)), 0.0f);
      BOOST_CHECK_EQUAL(TO_F32(*ctx.execute("f32_good2", (uint32_t)65503)), 0.0f);
      BOOST_CHECK_EQUAL(TO_F32(*ctx.execute("f32_good3", (uint32_t)65503)), 0.0f);
      BOOST_CHECK_EQUAL(TO_F32(*ctx.execute("f32_good4", (uint32_t)65503)), 0.0f);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("f32_good5", (uint32_t)65503)), 0x500001);
 
      //      BOOST_CHECK_EQUAL(TO_UINT64(ctx.execute("f32_good1", (uint32_t)65503)), 0);
      //      BOOST_CHECK_EQUAL(TO_UINT64(ctx.execute("f32_good2", (uint32_t)65503)), 0);
      //      BOOST_CHECK_EQUAL(TO_UINT64(ctx.execute("f32_good3", (uint32_t)65503)), 0);
      //      BOOST_CHECK_EQUAL(TO_UINT64(ctx.execute("f32_good4", (uint32_t)65503)), 0);
      //      BOOST_CHECK_EQUAL(TO_UINT64(ctx.execute("f32_good5", (uint32_t)65503)), 0);
      // 
   } FC_LOG_AND_RETHROW() 
}

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

BOOST_AUTO_TEST_CASE(blocks_tests) { 
   try {
      memory_manager::set_memory_limits( 32*1024*1024 );
      module mod;
      auto ctx = create_execution_context<interpret_visitor>("wasms/blocks.wasm", mod);

      BOOST_CHECK(!ctx.execute("empty"));
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("singular")), (uint32_t)7);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("multi")), (uint32_t)8);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("nested")), (uint32_t)9);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("deep")), (uint32_t)150);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-select-first")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-select-mid")), (uint32_t)2);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-select-last")), (uint32_t)2);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-loop-first")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-loop-mid")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-loop-last")), (uint32_t)1);

      BOOST_CHECK(!ctx.execute("as-if-condition"));
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-if-then")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-if-else")), (uint32_t)2);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-br_if-first")), (uint32_t)1);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-br_if-last")), (uint32_t)2);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-br_table-first")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-br_table-last")), (uint32_t)2);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call_indirect-first")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call_indirect-mid")), (uint32_t)2);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call_indirect-last")), (uint32_t)1);
      
      BOOST_CHECK(!ctx.execute("as-store-first"));
      
      BOOST_CHECK(!ctx.execute("as-store-last"));
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-memory.grow-value")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call-value")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-return-value")), (uint32_t)1);
      
      BOOST_CHECK(!ctx.execute("as-drop-operand"));
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-br-value")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-set_local-value")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-set_local-value")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-set_global-value")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-load-operand")), (uint32_t)1);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-unary-operand")), (uint32_t)0);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-binary-operand")), (uint32_t)12);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-test-operand")), (uint32_t)0);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-compare-operand")), (uint32_t)0);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("break-bare")), (uint32_t)19);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("break-value")), (uint32_t)18);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("break-repeated")), (uint32_t)18);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("break-inner")), (uint32_t)0xf);
      
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("effects")), (uint32_t)1);

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(br_tests) { 
   try {
      memory_manager::set_memory_limits( 32*1024*1024 );
      module mod;
      auto ctx = create_execution_context<interpret_visitor>("wasms/br.wasm", mod);

      BOOST_CHECK(!ctx.execute("type-i32"));
      BOOST_CHECK(!ctx.execute("type-i64"));
      BOOST_CHECK(!ctx.execute("type-f32"));
      BOOST_CHECK(!ctx.execute("type-f64"));

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("type-i32-value")), (uint32_t)1);
      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("type-i64-value")), (uint64_t)2);
      BOOST_CHECK_EQUAL(TO_F32(*ctx.execute("type-f32-value")), (float)3);
      BOOST_CHECK_EQUAL(TO_F64(*ctx.execute("type-f64-value")), (double)4);

      BOOST_CHECK(!ctx.execute("as-block-first"));
      BOOST_CHECK(!ctx.execute("as-block-mid"));
      BOOST_CHECK(!ctx.execute("as-block-last"));
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-block-value")), (uint32_t)2);
      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-loop-first")), (uint32_t)3);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-loop-mid")), (uint32_t)4);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-loop-last")), (uint32_t)5);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-br-value")), (uint32_t)9);

      BOOST_CHECK(!ctx.execute("as-br_if-cond"));

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-br_if-value")), (uint32_t)8);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-br_if-value-cond")), (uint32_t)9);

      BOOST_CHECK(!ctx.execute("as-br_table-index"));

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-br_table-value")), (uint32_t)10);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-br_table-value-index")), (uint32_t)11);

      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("as-return-value")), (uint64_t)7);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-if-cond")), (uint32_t)2);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-if-then", (uint32_t)1, (uint32_t)6)), (uint32_t)3);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-if-then", (uint32_t)0, (uint32_t)6)), (uint32_t)6);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-if-else", (uint32_t)0, (uint32_t)6)), (uint32_t)4);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-if-else", (uint32_t)1, (uint32_t)6)), (uint32_t)6);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-select-first", (uint32_t)0, (uint32_t)6)), (uint32_t)5);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-select-first", (uint32_t)1, (uint32_t)6)), (uint32_t)5);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-select-second", (uint32_t)0, (uint32_t)6)), (uint32_t)6);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-select-second", (uint32_t)1, (uint32_t)6)), (uint32_t)6);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-select-cond")), (uint32_t)7);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call-first")), (uint32_t)12);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call-mid")), (uint32_t)13);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call-last")), (uint32_t)14);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call_indirect-func")), (uint32_t)20);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call_indirect-first")), (uint32_t)21);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call_indirect-mid")), (uint32_t)22);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-call_indirect-last")), (uint32_t)23);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-set_local-value")), (uint32_t)17);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-tee_local-value")), (uint32_t)1);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-set_global-value")), (uint32_t)1);

      BOOST_CHECK_EQUAL(TO_F32(*ctx.execute("as-load-address")), (float)1.7);

      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("as-loadN-address")), (uint64_t)30);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-store-address")), (uint32_t)30);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-store-value")), (uint32_t)31);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-storeN-address")), (uint32_t)32);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-storeN-value")), (uint32_t)33);

      BOOST_CHECK_EQUAL(TO_F32(*ctx.execute("as-unary-operand")), (float)3.4);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-binary-left")), (uint32_t)3);

      BOOST_CHECK_EQUAL(TO_UINT64(*ctx.execute("as-binary-right")), (uint64_t)45);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-test-operand")), (uint32_t)44);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-compare-left")), (uint32_t)43);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-compare-right")), (uint32_t)42);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("as-memory.grow-size")), (uint32_t)40);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("nested-block-value")), (uint32_t)9);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("nested-br-value")), (uint32_t)9);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("nested-br_if-value")), (uint32_t)9);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("nested-br_if-value-cond")), (uint32_t)9);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("nested-br_table-value")), (uint32_t)9);

      BOOST_CHECK_EQUAL(TO_UINT32(*ctx.execute("nested-br_table-value-index")), (uint32_t)9);

   } FC_LOG_AND_RETHROW()
}
BOOST_AUTO_TEST_SUITE_END()
