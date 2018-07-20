#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>

#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>

#include <eosio/wasm_backend/integer_types.hpp>
#include <eosio/wasm_backend/wasm_interpreter.hpp>
#include <eosio/wasm_backend/types.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;

BOOST_AUTO_TEST_SUITE(varint_tests)
BOOST_AUTO_TEST_CASE(varint_test) { 
   try {
      {
         varuint<1> v(0); 
         BOOST_CHECK_EQUAL( v.get(), 0 );
         v.set(1);
         BOOST_CHECK_EQUAL( v.get(), 1 );
         BOOST_CHECK_THROW( v.set(2), wasm_interpreter_exception );
      }
      {
         varuint<7> v(0);
         BOOST_CHECK_EQUAL( v.get(), 0 );
         v.set(1);
         BOOST_CHECK_EQUAL( v.get(), 1 );
         v.set(2);
         BOOST_CHECK_EQUAL( v.get(), 2 );
         v.set(127); 
         BOOST_CHECK_EQUAL( v.get(), 127 );
         BOOST_CHECK_THROW( v.set(128), wasm_interpreter_exception );
      }
      {
         varuint<32> v(0);
         BOOST_CHECK_EQUAL( v.get(), 0 );
         v.set(1);
         BOOST_CHECK_EQUAL( v.get(), 1 );
         v.set(2);
         BOOST_CHECK_EQUAL( v.get(), 2 );
         v.set(127); 
         BOOST_CHECK_EQUAL( v.get(), 127 );
         v.set(128); 
         BOOST_CHECK_EQUAL( v.get(), 128 );
         v.set((1<<31)-1); 
         BOOST_CHECK_EQUAL( v.get(), (1<<31)-1 );
         BOOST_CHECK_THROW( v.set(1<<31), wasm_interpreter_exception );
      }
      {
         varint<7> v(0);
         BOOST_CHECK_EQUAL( v.get(), 0 );
         v.set(1);
         BOOST_CHECK_EQUAL( v.get(), 1 );
         v.set(2);
         BOOST_CHECK_EQUAL( v.get(), 2 );
         v.set(63); 
         BOOST_CHECK_EQUAL( v.get(), 63 );
         BOOST_CHECK_THROW( v.set(64), wasm_interpreter_exception );
         v.set(-1);
         BOOST_CHECK_EQUAL( v.get(), -1 );
         v.set(-64);
         BOOST_CHECK_EQUAL( v.get(), -64 );
         BOOST_CHECK_THROW( v.set(-65), wasm_interpreter_exception );
      }
      {
         varint<32> v(0);
         BOOST_CHECK_EQUAL( v.get(), 0 );
         v.set(1);
         BOOST_CHECK_EQUAL( v.get(), 1 );
         v.set(2);
         BOOST_CHECK_EQUAL( v.get(), 2 );
         v.set(127); 
         BOOST_CHECK_EQUAL( v.get(), 127 );
         v.set(128); 
         BOOST_CHECK_EQUAL( v.get(), 128 );
         v.set((1<<31)-1);
         BOOST_CHECK_EQUAL( v.get(), (1<<31)-1 );
         BOOST_CHECK_THROW( v.set(((uint64_t)1<<32)), wasm_interpreter_exception );
      }

   } FC_LOG_AND_RETHROW() 
}

BOOST_AUTO_TEST_CASE(varint_raw_test) {
   try {
      {
         varuint<1> v(0); 
         v.set(std::vector<uint8_t>{0x00,0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 0 );
         v.set(std::vector<uint8_t>{0x01, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 1 );
         //BOOST_CHECK_THROW( v.set(std::vector<uint8_t>{0x02, 0x00}, 0), wasm_interpreter_exception );
      }
      {
         varuint<7> v(0);
         v.set(std::vector<uint8_t>{0x00, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 0 );
         v.set(std::vector<uint8_t>{0x01, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 1 );
         v.set(std::vector<uint8_t>{0x02, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 2 );
         v.set(std::vector<uint8_t>{0x7F, 0x00}, 0); 
         BOOST_CHECK_EQUAL( v.get(), 127 );
         //BOOST_CHECK_THROW( v.set(128), wasm_interpreter_exception );
      }
      {
         varuint<32> v(0);
         BOOST_CHECK_EQUAL( v.get(), 0 );
         v.set(std::vector<uint8_t>{0x01, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 1 );
         v.set(std::vector<uint8_t>{0x02, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 2 );
         v.set(std::vector<uint8_t>{0x7F, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 127 );
         v.set(std::vector<uint8_t>{0x80, 0x01}, 0);
         BOOST_CHECK_EQUAL( v.get(), 128 );
         v.set(std::vector<uint8_t>{0xFF, 0xFF, 0xFF, 0xFF, 0x07}, 0); 
         BOOST_CHECK_EQUAL( v.get(), (1<<31)-1 );
         //BOOST_CHECK_THROW( v.set(1<<31), wasm_interpreter_exception );
      }
      {
         varint<7> v(0);
         BOOST_CHECK_EQUAL( v.get(), 0 );
         v.set(std::vector<uint8_t>{0x01, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 1 );
         v.set(std::vector<uint8_t>{0x02, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 2 );
         v.set(std::vector<uint8_t>{0x3F, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 63 );
         //BOOST_CHECK_THROW( v.set(64), wasm_interpreter_exception );
         v.set(std::vector<uint8_t>{0x7F, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), -1 );
         v.set(std::vector<uint8_t>{0x40, 0x01}, 0);
         BOOST_CHECK_EQUAL( v.get(), -64 );
         //BOOST_CHECK_THROW( v.set(-65), wasm_interpreter_exception );
      }
      {
         varint<32> v(0);
         BOOST_CHECK_EQUAL( v.get(), 0 );
         v.set(std::vector<uint8_t>{0x01, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 1 );
         v.set(std::vector<uint8_t>{0x02, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 2 );
         v.set(std::vector<uint8_t>{0xFF, 0x00}, 0);
         BOOST_CHECK_EQUAL( v.get(), 127 );
         v.set(std::vector<uint8_t>{0x80, 0x01}, 0);
         BOOST_CHECK_EQUAL( v.get(), 128 );
         v.set(std::vector<uint8_t>{0xFF, 0xFF, 0xFF, 0xFF, 0x07}, 0);
         BOOST_CHECK_EQUAL( v.get(), (1<<31)-1 );
         //BOOST_CHECK_THROW( v.set(((uint64_t)1<<32)), wasm_interpreter_exception );
      }

   } FC_LOG_AND_RETHROW() 
}

BOOST_AUTO_TEST_SUITE_END()

