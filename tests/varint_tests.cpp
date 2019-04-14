#include <algorithm>
#include <vector>
//#include <iterator>
#include <cstdlib>

#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>

#include <eosio/wasm_backend/leb128.hpp>
#include <eosio/wasm_backend/wasm_interpreter.hpp>
#include <eosio/wasm_backend/types.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;

BOOST_AUTO_TEST_SUITE(leb128_tests)
BOOST_AUTO_TEST_CASE(varuint_test) { 
   try {
      {
         std::vector<uint8_t> tv = {0};
         guarded_ptr<uint8_t> gp1_0(tv.data(), 5);
         guarded_ptr<uint8_t> gp7_0(tv.data(), 5);
         guarded_ptr<uint8_t> gp32_0(tv.data(), 5);
         varuint<1> v1(gp1_0); 
         varuint<7> v7(gp7_0); 
         varuint<32> v32(gp32_0); 

         BOOST_CHECK_EQUAL( v1.to(),  0 );
         BOOST_CHECK_EQUAL( v7.to(),  0 );
         BOOST_CHECK_EQUAL( v32.to(), 0 );

         tv[0] = 1;
         guarded_ptr<uint8_t> gp1_1(tv.data(), 5);
         guarded_ptr<uint8_t> gp7_1(tv.data(), 5);
         guarded_ptr<uint8_t> gp32_1(tv.data(), 5);

         varuint<1> v1_1(gp1_1); 
         varuint<7> v7_1(gp7_1); 
         varuint<32> v32_1(gp32_1); 
         BOOST_CHECK_EQUAL( v1_1.to(),  1 );
         BOOST_CHECK_EQUAL( v7_1.to(),  1 );
         BOOST_CHECK_EQUAL( v32_1.to(), 1 );
      }

      {
         std::vector<uint8_t> tv = {0x7f};
         guarded_ptr<uint8_t> gp7_0(tv.data(), 5);
         guarded_ptr<uint8_t> gp32_0(tv.data(), 5);
         varuint<7> v7(gp7_0); 
         varuint<32> v32(gp32_0); 

         BOOST_CHECK_EQUAL( v7.to(),  127 );
         BOOST_CHECK_EQUAL( v32.to(), 127 );

         tv[0] = 1;
         std::vector<uint8_t> tv2 = {0x80, 0x7f};
         guarded_ptr<uint8_t> gp7_1(tv2.data(), 5);
         guarded_ptr<uint8_t> gp32_1(tv2.data(), 5);

         varuint<7> v7_1(gp7_1); 
         varuint<32> v32_1(gp32_1); 
         BOOST_CHECK_EQUAL( v32_1.to(), 16256 );
      }

      {
         std::vector<uint8_t> tv0 = {0xb4, 0x7};
         guarded_ptr<uint8_t> gp32_0(tv0.data(), 5);

         std::vector<uint8_t> tv1 = {0x8c, 0x8};
         guarded_ptr<uint8_t> gp32_1(tv1.data(), 5);

         std::vector<uint8_t> tv2 = {0xff, 0xff, 0xff, 0xff, 0xf};
         guarded_ptr<uint8_t> gp32_2(tv2.data(), 5);

         varuint<32> v32_0(gp32_0); 
         varuint<32> v32_1(gp32_1); 
         varuint<32> v32_2(gp32_2); 

         BOOST_CHECK_EQUAL( v32_0.to(), 0x3b4 );
         BOOST_CHECK_EQUAL( v32_1.to(), 0x40c );
         BOOST_CHECK_EQUAL( v32_2.to(), 0xffffffff );
      }

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(varint_test) {
   try {
      {
         std::vector<uint8_t> tv0 = {0x0};
         std::vector<uint8_t> tv1 = {0x1};
         std::vector<uint8_t> tv2 = {0x7f};

         guarded_ptr<uint8_t> gp0(tv0.data(), 5);
         guarded_ptr<uint8_t> gp1(tv1.data(), 5);
         guarded_ptr<uint8_t> gp2(tv2.data(), 5);

         varint<7> v0(gp0);
         varint<7> v1(gp1);
         varint<7> v2(gp2);

         BOOST_CHECK_EQUAL( v0.to(), 0 );
         BOOST_CHECK_EQUAL( v1.to(), 1 );
         BOOST_CHECK_EQUAL( (int32_t)v2.to(), -1 );
      }

      {
         std::vector<uint8_t> tv0 = {0x0};
         std::vector<uint8_t> tv1 = {0x1};
         std::vector<uint8_t> tv2 = {0x7f};
         std::vector<uint8_t> tv3 = {0x80, 0x7f};

         guarded_ptr<uint8_t> gp0(tv0.data(), 5);
         guarded_ptr<uint8_t> gp1(tv1.data(), 5);
         guarded_ptr<uint8_t> gp2(tv2.data(), 5);
         guarded_ptr<uint8_t> gp3(tv3.data(), 5);

         varint<32> v0(gp0);
         varint<32> v1(gp1);
         varint<32> v2(gp2);

         BOOST_CHECK_EQUAL( v0.to(), 0 );
         BOOST_CHECK_EQUAL( v1.to(), 1 );
         BOOST_CHECK_EQUAL( v2.to(), -1 );
      }

      {
         std::vector<uint8_t> tv0 = {0x0};
         std::vector<uint8_t> tv1 = {0x1};
         std::vector<uint8_t> tv2 = {0x7f};

         guarded_ptr<uint8_t> gp0(tv0.data(), 5);
         guarded_ptr<uint8_t> gp1(tv1.data(), 5);
         guarded_ptr<uint8_t> gp2(tv2.data(), 5);

         varint<64> v0(gp0);
         varint<64> v1(gp1);
         varint<64> v2(gp2);

         BOOST_CHECK_EQUAL( v0.to(), 0 );
         BOOST_CHECK_EQUAL( v1.to(), 1 );
         BOOST_CHECK_EQUAL( v2.to(), -1 );
      }
   } FC_LOG_AND_RETHROW() 
}

BOOST_AUTO_TEST_SUITE_END()

