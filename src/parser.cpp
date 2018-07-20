#include <eosio/wasm_backend/parser.hpp>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/constants.hpp>
#include <eosio/wasm_backend/sections.hpp>

namespace eosio { namespace wasm_backend {
   void binary_parser::parse( std::vector<uint8_t> code ) {
   }

   size_t binary_parser::parse_magic( const wasm_code& code ) {
      const uint8_t* raw = code.data(); 
      EOS_WB_ASSERT(constant::magic_number == *((uint32_t*)raw), 
            wasm_interpreter_exception,
            "magic number is invalid");
      return sizeof(uint32_t);
   }
   
   size_t binary_parser::parse_version( const wasm_code& code, size_t index, uint32_t& version ) {
      const uint8_t* raw = code.data()+index;
      version = *((uint32_t*)raw);
      EOS_WB_ASSERT(constant::version == version,
            wasm_interpreter_exception,
            "currently only support WA version 1.0");
      return sizeof(uint32_t);
   }
   
   size_t binary_parser::parse_section_id( const wasm_code& code, size_t index, uint8_t& id ) {
      const uint8_t* raw = code.data()+index;
      id = *raw;
      return sizeof(uint8_t);
   } 

   size_t binary_parser::parse_section_payload_len( const wasm_code& code, size_t index, varuint<32>& len ) {
      len = parse_varuint<32>( code, index );
      return len.size;
   } 
   
   size_t binary_parser::parse_section_payload_data( const wasm_code& code, size_t index, size_t len, wasm_bytes& bytes ) {
      const uint8_t* raw = code.data()+index; 
      bytes.resize(len);
      memcpy(bytes.data(), raw, len);
      return len;
   }

   // TODO: figure out if we can omit this. currently just ignore 
   size_t binary_parser::parse_custom_section( const wasm_code& code, size_t index ) {
      const uint8_t* raw = code.data()+index;
      return 1+sizeof(uint32_t)+*((uint32_t*)raw);
   }
   
   size_t binary_parser::parse_type_section( const wasm_code& code, size_t index, std::vector<func_type>& types ) {
      //varuint<32> len = parse_varuint<32>( code, index );
      //index += len.size;
      auto len = 335;

      varuint<32> type_cnt = parse_varuint<32>( code, index );
      
      index += type_cnt.size; 

      for ( int i=0; i < type_cnt.get(); i++ ) {
         func_type ft;
         ft.form = code.at(index);
         index += 1;
         auto pc = parse_varuint<32>( code, index );
         ft.param_count = pc.get();
         index += pc.size;
         ft.param_types.resize(ft.param_count);
         for ( int j=0; j < ft.param_count; j++ ) {
            auto tmp = code.at(index);
            ft.param_types[j] = tmp;
            index += 1; 
         }
         ft.return_count = parse_varuint<1>( code, index ).get();
         index += 1;
         if ( ft.return_count > 0 ) {
            ft.return_type = code.at(index);
            index += 1;
         }
         types.push_back( ft );
      } 
      return len;
   }

   size_t binary_parser::parse_import_section( const wasm_code& code, size_t index, std::vector<import_entry>& imports ) {
      const uint8_t* raw = code.data()+index;
      varuint<32> len = parse_varuint<32>( code, index );
      index += len.size;

      std::cout << "LEN " << len.get() << "\n";
   }
}} // namespace eosio::wasm_backend
