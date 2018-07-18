#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <fstream>
#include <string>

#include <boost/test/unit_test.hpp>
#include <boost/test/framework.hpp>

#include <eosio/wasm_backend/integer_types.hpp>
#include <eosio/wasm_backend/wasm_interpreter.hpp>
#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/execution_engine.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/parser.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;

std::vector<uint8_t> read_wasm( const std::string& fname ) {
   std::ifstream wasm_file(fname, std::ios::binary);
   FC_ASSERT( wasm_file.is_open(), "wasm file cannot be found" );
   wasm_file.seekg(0, std::ios::end);
   std::vector<uint8_t> wasm; 
   int len = wasm_file.tellg();
   FC_ASSERT( len >= 0, "wasm file length is -1" );
   wasm.resize(len);
   wasm_file.seekg(0, std::ios::beg);
   wasm_file.read((char*)wasm.data(), wasm.size());
   wasm_file.close();
   return wasm;
}

BOOST_AUTO_TEST_SUITE(parser_tests)
BOOST_AUTO_TEST_CASE(parse_test) { 
   try {
      {
         binary_parser bp;
         wasm_code error = { 0x01, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00 };
         BOOST_CHECK_THROW(bp.parse_magic( error ), wasm_interpreter_exception);
         wasm_code correct = { 0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00 };
         auto n = bp.parse_magic( correct );
         BOOST_CHECK_EQUAL(n, sizeof(uint32_t));
      }
   } FC_LOG_AND_RETHROW() 
}

BOOST_AUTO_TEST_CASE(actual_wasm_test) { 
   try {
      {
         wasm_code code = read_wasm( "test.wasm" );
         binary_parser bp;
         auto n = bp.parse_magic( code );
         BOOST_CHECK_EQUAL(n, sizeof(uint32_t));
         uint32_t version;
         n += bp.parse_version( code, n, version );
         BOOST_CHECK_EQUAL(version, 1);
         uint8_t id;
         n += bp.parse_section_id( code, n, id );
         BOOST_CHECK_EQUAL(id, 1);
         varuint<32> plen;
         n += bp.parse_section_payload_len( code, n, plen );
         BOOST_CHECK_EQUAL(plen.get(), 335);
         wasm_bytes payload;
         n += bp.parse_section_payload_data( code, n, plen.get(), payload );
         std::vector<func_type> types;
         n += bp.parse_type_section( code, n, types );
      }
   } FC_LOG_AND_RETHROW() 
}
BOOST_AUTO_TEST_SUITE_END()

