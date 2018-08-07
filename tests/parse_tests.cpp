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
#include <eosio/wasm_backend/constants.hpp>
#include <eosio/wasm_backend/sections.hpp>

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
         wasm_allocator wa(64*1024);
         binary_parser bp(wa);
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
      static const std::vector<std::pair <std::vector<uint8_t>, std::vector<uint8_t>>> system_contract_types = {
         {{types::i32,types::i32}, {}},
         {{types::i32,types::i32,types::i64},{}},
         {{types::i32},{}},
         {{types::i32,types::i64,types::i64},{}},
         {{types::i32,types::i64,types::i64,types::i32},{}},
         {{types::i32,types::i64,types::i32,types::i32,types::i32},{}},
         {{types::i32,types::i64},{}},
         {{types::i32,types::i64,types::i64,types::i32,types::i32},{}},
         {{types::i32,types::i64,types::i32},{}},
         {{types::i32,types::i64,types::i64,types::i32,types::i32,types::i32},{}},
         {{},{}},
         {{types::i64,types::i64},{}},
         {{types::i64},{}},
         {{},{types::i64}},
         {{types::i64,types::i64,types::i64,types::i64},{types::i32}},
         {{types::i32,types::i64,types::i32,types::i32},{}},
         {{types::i64,types::i64,types::i64,types::i32,types::i64},{types::i32}},
         {{types::i32,types::i32,types::i32},{types::i32}},
         {{types::i32,types::i32},{types::i64}},
         {{types::i64,types::i64,types::i64,types::i64,types::i32,types::i32},{types::i32}},
         {{types::i64,types::i64,types::i64,types::i64},{}},
         {{types::i32,types::i32},{types::i32}},
         {{types::i32},{types::i32}},
         {{types::i64,types::i32},{}},
         {{types::i64},{types::i32}},
         {{types::i64,types::i64,types::i64,types::i64,types::i32},{types::i32}},
         {{},{types::i32}},
         {{types::i64,types::i64,types::i64,types::i32,types::i32},{types::i32}},
         {{types::i32,types::i64,types::i64,types::i64,types::i64},{}},
         {{types::i64,types::i64},{types::i32}},
         {{types::i32,types::f64},{}},
         {{types::i32,types::f32},{}},
         {{types::i64,types::i64},{types::f64}},
         {{types::i64,types::i64},{types::f32}},
         {{types::i32,types::i32,types::i32},{}},
         {{types::i32,types::i64,types::i32},{types::i32}},
         {{types::i64,types::i64,types::i32,types::i32},{}},
         {{types::i32,types::i32,types::i32,types::i64},{}},
         {{types::i32,types::i32,types::i32,types::i32},{}},
         {{types::i32,types::i32,types::i32,types::i32,types::i32},{}},
         {{types::i32,types::i32,types::i64,types::i32},{}},
         {{types::i32,types::i64},{types::i32}},
         {{types::i64},{types::i64}},
         {{types::i64,types::i64,types::i64},{}},
         {{types::i32,types::i32,types::i32,types::i32,types::i32,types::i32},{types::i32}},
         {{types::i32,types::i32,types::i32,types::i32},{types::i32}},
         {{types::i32,types::i32,types::i32,types::i32,types::i32},{types::i32}},
         {{types::i32,types::i32,types::i32,types::i32,types::i32,types::i32,types::i32,types::i32},{}},
         {{types::f64},{types::f64}},
         {{types::f64,types::f64},{types::f64}},
         {{types::f64,types::i32},{types::f64}}
      };

      {
         wasm_allocator wa(64*1024);
         binary_parser bp(wa);
         module<binary_parser> mod(bp);
         wasm_code code = read_wasm( "test.wasm" );
         auto n = bp.parse_magic( code );
         BOOST_CHECK_EQUAL(n, sizeof(uint32_t));

         uint32_t version;
         n += bp.parse_version( code, n, version );
         BOOST_CHECK_EQUAL(version, 1);

         uint8_t id;
         n += bp.parse_section_id( code, n, id );
         BOOST_CHECK_EQUAL(id, section_id::type_section);

         varuint<32> len;
         n += bp.parse_section_payload_len( code, n, len );
         BOOST_CHECK_EQUAL(len.get(), 335);
         wasm_code_ptr tmp(code.data()+n - len.size, len.size); 

         auto len3 = bp.parse_section_payload_len( tmp );
         std::cout << "LEN3 " << len3 << "\n";
         wasm_code_ptr code_ptr(code.data()+n, len.get());
         auto len2 = bp.parse_type_section( code_ptr, mod.types );
         for ( int i=0; i < mod.types.size(); i++ ) {
            auto& ft = mod.types.at(i);
            BOOST_CHECK_EQUAL( ft.form, types::func );
            BOOST_CHECK_EQUAL( ft.param_count, std::get<0>(system_contract_types[i]).size() );
            for ( int j=0; j < ft.param_types.size(); j++ ) {
               auto type = ft.param_types.at(j);
               BOOST_CHECK_EQUAL( std::get<0>(system_contract_types[i])[j++], type );
            }
            if ( ft.return_count )
               BOOST_CHECK_EQUAL ( std::get<1>(system_contract_types[i])[0], ft.return_type );
            i++;
         }

         BOOST_CHECK_EQUAL( len2, len.get() );
         n += len2;

         n += bp.parse_section_id( code, n, id );
         BOOST_CHECK_EQUAL(id, section_id::import_section);
         n += bp.parse_section_payload_len( code, n, len );
         BOOST_CHECK_EQUAL( len.get(), 1179 ); 

         n += bp.parse_import_section( code, n, mod.imports );
         for (int i = 0; i < mod.imports.size(); i++ ) {
            BOOST_CHECK_EQUAL( mod.imports[i].kind, external_kind::Function );
         }
         
         n += bp.parse_section_id( code, n, id ); 
         BOOST_CHECK_EQUAL(id, section_id::function_section);
         n += bp.parse_section_payload_len( code, n, len );
         BOOST_CHECK_EQUAL( len.get(), 210 );

         n += bp.parse_function_section( code, n, mod.functions );
         for ( int i=0; i < mod.functions.size(); i++ ) {
            //std::cout << "FUNC " << mod.imports.size()+i << " : " << mod.functions[i] << "\n";
         }

         n += bp.parse_section_id( code, n, id );
         BOOST_CHECK_EQUAL(id, section_id::table_section);
         n += bp.parse_section_payload_len( code, n, len );
         BOOST_CHECK_EQUAL( len.get(), 5 );

         n += bp.parse_table_section( code, n, mod.tables );
         BOOST_CHECK_EQUAL( mod.tables[0].element_type, 0x70 );
         BOOST_CHECK_EQUAL( mod.tables[0].limits.flags, true );
         BOOST_CHECK_EQUAL( mod.tables[0].limits.initial, 0x1A );
         BOOST_CHECK_EQUAL( mod.tables[0].limits.maximum, 0x1A );

         n += bp.parse_section_id( code, n, id );
         BOOST_CHECK_EQUAL(id, section_id::memory_section);
         n += bp.parse_section_payload_len( code, n, len );
         BOOST_CHECK_EQUAL( len.get(), 3 );

         wasm_env we;
         n += bp.parse_memory_section( code, n, mod.memories );
         BOOST_CHECK_EQUAL( mod.memories.at(0).limits.flags, false );
         BOOST_CHECK_EQUAL( mod.memories.at(0).limits.initial, 0x01 );
         
         n += bp.parse_section_id( code, n, id );
         BOOST_CHECK_EQUAL(id, section_id::global_section);
         n += bp.parse_section_payload_len( code, n, len );
         BOOST_CHECK_EQUAL( len.get(), 0x16 );

      }
   } FC_LOG_AND_RETHROW() 
}
BOOST_AUTO_TEST_SUITE_END()

