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
         binary_parser bp;
         wasm_code error = { 0x01, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00 };
         wasm_code_ptr error_ptr( error.data(), 4 );
         auto n = bp.parse_magic( error_ptr );
         BOOST_CHECK(n != constants::magic);
         wasm_code correct = { 0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00 };
         wasm_code_ptr correct_ptr( correct.data(), 4 );
         n = bp.parse_magic( correct_ptr );
         BOOST_CHECK_EQUAL(n, constants::magic);
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
         binary_parser bp;
         module mod;
         memory_manager::set_memory_limits( 128*1024, 64*1024 );
         wasm_code code = read_wasm( "test.wasm" );
         wasm_code_ptr code_ptr(code.data(), 0);
         
         code_ptr.add_bounds( constants::magic_size );
         auto magic = bp.parse_magic( code_ptr );
         BOOST_CHECK_EQUAL(constants::magic, magic);

         code_ptr.add_bounds( constants::version_size );
         auto version = bp.parse_version( code_ptr );
         BOOST_CHECK_EQUAL(constants::version, version);

         code_ptr.add_bounds( constants::id_size );
         uint8_t id = bp.parse_section_id( code_ptr );
         BOOST_CHECK_EQUAL(id, section_id::type_section);

         code_ptr.add_bounds( constants::varuint32_size );
         auto len = bp.parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds();
         BOOST_CHECK_EQUAL(len, 335);

         code_ptr.add_bounds( len );
         bp.parse_type_section( code_ptr, mod.types );

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
         }

         code_ptr.add_bounds( constants::id_size );
         id = bp.parse_section_id( code_ptr );
         BOOST_CHECK_EQUAL( id, section_id::import_section );

         code_ptr.add_bounds( constants::varuint32_size );
         len = bp.parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds();
         BOOST_CHECK_EQUAL( len, 1179 );

         code_ptr.add_bounds( len );
         bp.parse_import_section( code_ptr, mod.imports );
         for ( int i=0; i < mod.imports.size(); i++ ) {
            BOOST_CHECK_EQUAL( mod.imports.at(i).kind, external_kind::Function );
         }
         
         code_ptr.add_bounds( constants::id_size );
         id = bp.parse_section_id( code_ptr );
         BOOST_CHECK_EQUAL( id, section_id::function_section );

         code_ptr.add_bounds( constants::varuint32_size );
         len = bp.parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds();
         BOOST_CHECK_EQUAL( len, 210 );

         code_ptr.add_bounds(len); 
         bp.parse_function_section( code_ptr, mod.functions );
         for ( int i=0; i < mod.functions.size(); i++ ) {
            //std::cout << "FUNC " << mod.imports.size()+i << " : " << mod.functions.at(i) << "\n";
         }

         code_ptr.add_bounds( constants::id_size );
         id = bp.parse_section_id( code_ptr );
         BOOST_CHECK_EQUAL( id, section_id::table_section );

         code_ptr.add_bounds( constants::varuint32_size );
         len = bp.parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds();
         BOOST_CHECK_EQUAL( len, 5 );

         code_ptr.add_bounds( len );
         bp.parse_table_section( code_ptr, mod.tables );
         BOOST_CHECK_EQUAL( mod.tables.at(0).element_type, 0x70 );
         BOOST_CHECK_EQUAL( mod.tables.at(0).limits.flags, true );
         BOOST_CHECK_EQUAL( mod.tables.at(0).limits.initial, 0x1A );
         BOOST_CHECK_EQUAL( mod.tables.at(0).limits.maximum, 0x1A );
         
         code_ptr.add_bounds( constants::id_size ); 
         id = bp.parse_section_id( code_ptr );
         BOOST_CHECK_EQUAL( id, section_id::memory_section );

         code_ptr.add_bounds( constants::varuint32_size );
         len = bp.parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds();
         BOOST_CHECK_EQUAL( len, 3 );

         code_ptr.add_bounds( len );
         bp.parse_memory_section( code_ptr, mod.memories );
         BOOST_CHECK_EQUAL( mod.memories.at(0).limits.flags, false );
         BOOST_CHECK_EQUAL( mod.memories.at(0).limits.initial, 0x01 );

         code_ptr.add_bounds( constants::id_size );
         id = bp.parse_section_id( code_ptr );
         BOOST_CHECK_EQUAL( id, section_id::global_section );

         code_ptr.add_bounds( constants::varuint32_size );
         len = bp.parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds();
         BOOST_CHECK_EQUAL( len, 0x16 );

         code_ptr.add_bounds( len );
         bp.parse_global_section( code_ptr, mod.globals );

         BOOST_CHECK_EQUAL( mod.globals.at(0).type.content_type, types::i32 );
         BOOST_CHECK_EQUAL( mod.globals.at(1).type.content_type, types::i32 );
         BOOST_CHECK_EQUAL( mod.globals.at(2).type.content_type, types::i32 );

         BOOST_CHECK_EQUAL( mod.globals.at(0).type.mutability, true );
         BOOST_CHECK_EQUAL( mod.globals.at(1).type.mutability, false );
         BOOST_CHECK_EQUAL( mod.globals.at(2).type.mutability, false );
       
         BOOST_CHECK_EQUAL( mod.globals.at(0).init.opcode, opcode::i32_const );
         BOOST_CHECK_EQUAL( mod.globals.at(1).init.opcode, opcode::i32_const );
         BOOST_CHECK_EQUAL( mod.globals.at(2).init.opcode, opcode::i32_const );

         BOOST_CHECK_EQUAL( mod.globals.at(0).init.value.i32, 8192 );
         BOOST_CHECK_EQUAL( mod.globals.at(1).init.value.i32, 12244 );
         BOOST_CHECK_EQUAL( mod.globals.at(2).init.value.i32, 12244 );

         code_ptr.add_bounds( constants::id_size );
         id = bp.parse_section_id( code_ptr );
         BOOST_CHECK_EQUAL( id, section_id::export_section );

         code_ptr.add_bounds( constants::varuint32_size );
         len = bp.parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds();
         BOOST_CHECK_EQUAL( len, 84 );
         
         code_ptr.add_bounds( len );
         bp.parse_export_section( code_ptr, mod.exports );
         BOOST_CHECK( memcmp((char*)mod.exports.at(0).field_str.raw(), "memory", mod.exports.at(0).field_len) == 0 && 
               mod.exports.at(0).kind == external_kind::Memory );
         BOOST_CHECK( memcmp((char*)mod.exports.at(1).field_str.raw(), "__heap_base", mod.exports.at(1).field_len) == 0 &&
               mod.exports.at(1).kind == external_kind::Global );
         BOOST_CHECK( memcmp((char*)mod.exports.at(2).field_str.raw(), "__data_end", mod.exports.at(2).field_len) == 0 &&
               mod.exports.at(2).kind == external_kind::Global );
         BOOST_CHECK( memcmp((char*)mod.exports.at(3).field_str.raw(), "apply", mod.exports.at(3).field_len) == 0 &&
               mod.exports.at(3).kind == external_kind::Function );
         BOOST_CHECK( memcmp((char*)mod.exports.at(4).field_str.raw(), "_ZdlPv", mod.exports.at(4).field_len) == 0 &&
               mod.exports.at(4).kind == external_kind::Function );
         BOOST_CHECK( memcmp((char*)mod.exports.at(5).field_str.raw(), "_Znwj", mod.exports.at(5).field_len) == 0 &&
               mod.exports.at(5).kind == external_kind::Function );
         BOOST_CHECK( memcmp((char*)mod.exports.at(6).field_str.raw(), "_Znaj", mod.exports.at(6).field_len) == 0 &&
               mod.exports.at(6).kind == external_kind::Function );
         BOOST_CHECK( memcmp((char*)mod.exports.at(7).field_str.raw(), "_ZdaPv", mod.exports.at(7).field_len) == 0 &&
               mod.exports.at(7).kind == external_kind::Function );

#if 0
         code_ptr.add_bounds( constants::id_size);
         id = bp.parse_section_id( code_ptr );
         BOOST_CHECK_EQUAL( id, section_id::start_section );

         code_ptr.add_bounds( constants::varuint32_size );
         len = bp.parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds();

         code_ptr.add_bounds( len );
         bp.parse_start_section( code_ptr, mod.start );
#endif

         code_ptr.add_bounds( constants::id_size );
         id = bp.parse_section_id( code_ptr );  
         BOOST_CHECK_EQUAL( id, section_id::element_section );

         code_ptr.add_bounds( constants::varuint32_size );
         len = bp.parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds();
         
         code_ptr.add_bounds( len );
         bp.parse_element_section( code_ptr, mod.elements );
         
         uint32_t indices[] = {73, 60, 169, 92, 80, 124, 131, 152, 176, 177, 178, 164, 153, 122, 136, 156, 158, 83, 184, 123, 129, 185, 154, 155, 121};

         for (int i=0; i < mod.elements.size(); i++) {
            for (int j=0; j < mod.elements[i].elems.size(); j++)
               BOOST_CHECK_EQUAL(mod.elements[i].elems[j], indices[j]);
         }

         code_ptr.add_bounds( constants::id_size );
         id = bp.parse_section_id( code_ptr );
         BOOST_CHECK_EQUAL( id, section_id::code_section );

         code_ptr.add_bounds( constants::varuint32_size );
         len = bp.parse_section_payload_len( code_ptr );
         code_ptr.fit_bounds();

         code_ptr.add_bounds( len );
         bp.parse_code_section( code_ptr, mod.code );
        /* 
         code_ptr.add_bounds( constants::id_size );
         id = bp.parse_section_id( code_ptr );
         BOOST_CHECK_EQUAL( id, section_id::data_section );
         */
      }
   } FC_LOG_AND_RETHROW() 
}
BOOST_AUTO_TEST_SUITE_END()

