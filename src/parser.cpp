#include <eosio/wasm_backend/parser.hpp>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/opcodes.hpp>
#include <eosio/wasm_backend/constants.hpp>


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

   size_t binary_parser::parse_section_payload_len( const wasm_code& code, size_t index, varuint<32>& id ) {
      id = parse_varuint<32>( code, index );
      return id.size;
   } 
   
   size_t binary_parser::parse_section_payload_data( const wasm_code& code, size_t index, size_t len, wasm_bytes& bytes ) {
      const uint8_t* raw = code.data()+index; 
      bytes.reserve(len);
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
      const uint32_t sec_size = *((uint32_t*)raw);
      const uint32_t funcs = *((uint32_t*)raw+sizeof(uint32_t));

      std::cout << "TYPES " << /*std::hex <<*/ sec_size << " FUNCTIONS " << funcs << "\n";

      return sizeof(uint32_t)+sec_size;
   }
}} // namespace eosio::wasm_backend
