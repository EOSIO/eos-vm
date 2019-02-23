#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>

#include <eosio/wasm_backend/memory_manager.hpp>
#include <eosio/wasm_backend/vector.hpp>

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
         nalloc.alloc<uint8_t>();
         BOOST_CHECK_THROW( nalloc.alloc<uint8_t>(), wasm_bad_alloc );
      }
      {
         native_allocator nalloc(12);
         uint8_t* base = nalloc.alloc<uint8_t>(0);
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
         nalloc.free();
         BOOST_CHECK_EQUAL(base, nalloc.alloc<uint8_t>(0));
      }
      {
         native_allocator nalloc(30);
         simple_allocator salloc(nalloc.alloc<uint8_t>(30), 30);
         uint8_t* base = salloc.alloc<uint8_t>(0);
         uint32_t* i1 = salloc.alloc<uint32_t>();
         *i1 = 0xFEFEFEFE;
         uint32_t* i2 = salloc.alloc<uint32_t>();
         *i2 = 0x7C7C7C7C;
         uint16_t* i3 = salloc.alloc<uint16_t>();
         *i3 = 0xBBBB;
         uint8_t* i4 = salloc.alloc<uint8_t>();
         *i4 = 0xAA;
         salloc.free();
         BOOST_CHECK_EQUAL( *i1, 0xFEFEFEFE );
         BOOST_CHECK_EQUAL( *i2, 0x7C7C7C7C );
         BOOST_CHECK_EQUAL( *i3, 0xBBBB );
         BOOST_CHECK_EQUAL( *i4, 0xAA );
         BOOST_CHECK_EQUAL(base, salloc.alloc<uint8_t>(0));
      }
   } FC_LOG_AND_RETHROW() 
}

BOOST_AUTO_TEST_CASE(memory_manager_tests) { 
   try {
      {
         memory_manager::set_memory_limits( 0, 30 );
         auto& nalloc = memory_manager::get_allocator<memory_manager::types::native>();
         auto& lmalloc = memory_manager::get_allocator<memory_manager::types::linear_memory>();
         BOOST_CHECK_EQUAL( nalloc.raw.get(), lmalloc.raw );
         BOOST_CHECK_THROW( nalloc.alloc<uint16_t>(1), wasm_bad_alloc );
      }
   } FC_LOG_AND_RETHROW() 
}

BOOST_AUTO_TEST_CASE(wasm_allocator_tests) { 
   try {
      {
         memory_manager::set_memory_limits( 1, 30 );
         auto& walloc = memory_manager::get_allocator<memory_manager::types::wasm>();
         uint8_t* p = walloc.alloc<uint8_t>((64*1024)-1);
         for (int i=0; i < (64*1024); i++) {
            *p++ = 3;
         }
         walloc.alloc<uint8_t>(1);
         BOOST_CHECK_THROW( [&](){*p = 1;}(), wasm_bad_alloc );
      }
   } FC_LOG_AND_RETHROW() 
}
struct test_struct {
   uint32_t i;
   uint64_t l;
   float    f;
   double   d;
};

BOOST_AUTO_TEST_CASE(vector_tests) {
   try {
      {
         memory_manager::set_memory_limits( sizeof(uint32_t), sizeof(test_struct) );
         managed_vector<uint32_t, memory_manager::types::native> nvec(1);
         managed_vector<test_struct, memory_manager::types::linear_memory> lmvec(1);
         nvec.push_back( 33 );
         lmvec.push_back( { 33, 10, 33.3f, 10.10 } );
         BOOST_CHECK_EQUAL( nvec[0], 33 );
         BOOST_CHECK( lmvec[0].i == 33 && lmvec[0].l == 10 && lmvec[0].f == 33.3f && lmvec[0].d == 10.10 );
         BOOST_CHECK_THROW( nvec.push_back( 0 ), wasm_vector_oob_exception );
         BOOST_CHECK_THROW( lmvec.push_back( {0,0,0,0} ), wasm_vector_oob_exception );
         BOOST_CHECK_THROW( nvec.resize(3), wasm_bad_alloc );
         BOOST_CHECK_THROW( lmvec.resize(3), wasm_bad_alloc );

         memory_manager::set_memory_limits( sizeof(uint32_t)*3, sizeof(test_struct)*3 );
         nvec.resize(3);
         lmvec.resize(3);
         nvec.push_back(22);
         nvec.push_back(11);
         BOOST_CHECK( nvec[0] == 33 && nvec[1] == 22 && nvec[2] == 11 );
         nvec.resize(2);
         BOOST_CHECK_THROW( nvec[2], wasm_vector_oob_exception );
      }
   } FC_LOG_AND_RETHROW()
}
BOOST_AUTO_TEST_SUITE_END()

