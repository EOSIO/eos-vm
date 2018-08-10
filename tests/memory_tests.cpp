#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>

#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>

#include <eosio/wasm_backend/memory_manager.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;

BOOST_AUTO_TEST_SUITE(memory_tests)
BOOST_AUTO_TEST_CASE(allocator_tests) { 
   try {
      {
         native_allocator nalloc(5);
         uint32_t* base = nalloc.alloc<uint32_t>(0);
         BOOST_CHECK_EQUAL( base, nalloc.alloc<uint32_t>(0) );
         uint32_t* i1 = nalloc.alloc<uint32_t>();
         BOOST_CHECK_EQUAL( i1, base );
         BOOST_CHECK_THROW( nalloc.alloc<uint8_t>(), wasm_bad_alloc );
      }
      {
         native_allocator nalloc(12);
         uint32_t* i1 = nalloc.alloc<uint32_t>();
         *i1 = 0xFEFEFEFE;
         uint32_t* i2 = nalloc.alloc<uint32_t>();
         *i2 = 0x7C7C7C7C;
         uint16_t* i3 = nalloc.alloc<uint16_t>();
         *i3 = 0xBBBB;
         uint8_t* i4 = nalloc.alloc<uint8_t>();
         *i4 = 0xAA;
         BOOST_CHECK_EQUAL( *i1, 0xFEFEFEFE );
         BOOST_CHECK_EQUAL( *i2, 0x7C7C7C7C );
         BOOST_CHECK_EQUAL( *i3, 0xBBBB );
         BOOST_CHECK_EQUAL( *i4, 0xAA );
      }
      {
         native_allocator nalloc(30);
         simple_allocator salloc(nalloc.alloc<uint8_t>(0), 31);
      }

//      BOOST_CHECK( i2-i1
   } FC_LOG_AND_RETHROW() 
}

BOOST_AUTO_TEST_SUITE_END()

