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
      const uint8_t* raw = code.data()+index; 
      varuint<32> len = parse_varuint<32>( code, index );
      size_t orig_index = index;
      index += len.size;

      varuint<32> type_cnt = parse_varuint<32>( code, index );
      const uint32_t funcs = *((uint32_t*)raw+sizeof(uint32_t));
      
      index += type_cnt.size; 

      for ( int i=0; i < funcs; i++ ) {
         func_type ft;
         ft.form = parse_varint<7>( code, index );
         std::cout << "FORM " << ft.form.get() << "\n";
         index += ft.form.size;
         ft.param_count = parse_varuint<32>( code, index );
         index += ft.param_count.size;
         for ( int i=0; i < ft.param_count.get(); i++ ) {
            auto tmp = parse_varint<7>( code, index );
            ft.param_types.push_back( tmp );
            index += tmp.size; 
         }
         ft.return_count = parse_varuint<1>( code, index );
         index += 1;
         if ( ft.return_count.get() > 0 ) {
            ft.return_type = parse_varint<7>( code, index );
            index += ft.return_type.size;
         }
         types.push_back( ft );
      } 
      std::cout << "TYPES " << /*std::hex <<*/ " FUNCTIONS " << type_cnt.get() << "\n";

      return 2*sizeof(uint32_t) + len.size;
   }
}} // namespace eosio::wasm_backend
