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

   uint32_t binary_parser::parse_section_payload_len( wasm_code_ptr& code ) {
      return parse_varuint<32>( code ).get();
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
   
#define TEST_LENGTH( x, y, z ) \
   x += y;                     \
   EOS_WB_ASSERT( x <= z, wasm_section_length_exception, "section length overflow" );

   size_t binary_parser::parse_type_section( wasm_code_ptr& code, binary_parser_vec<func_type>& types ) {
      varuint<32> type_cnt = parse_varuint<32>( code );
      size_t sum = 0;
      code.add(type_cnt.size); 
      types.resize( type_cnt.get() );
      for ( int i=0; i < type_cnt.get(); i++ ) {
         func_type ft;
         ft.form = *code++;
         auto pc = parse_varuint<32>( code );
         ft.param_count = pc.get();
         std::cout << "PC SIZE " << (uint32_t)pc.size << "\n";
         code.add(pc.size);
         ft.param_types.resize(ft.param_count);
         for ( int j=0; j < ft.param_count; j++ ) {
            ft.param_types[j] = *code++;
         }
         ft.return_count = *code++;
         if ( ft.return_count > 0 ) {
            ft.return_type = *code++;
         }
         types.push_back( ft );
      } 
      return sum;
   }
   
   size_t binary_parser::parse_global_type( const wasm_code& code, size_t index, global_type& type ) {
      type.content_type = code.at(index++);
      type.mutability   = code.at(index);
      return 2;
   } 
   
   size_t binary_parser::parse_memory_type( const wasm_code& code, size_t index, memory_type& type ) {
      const size_t orig_index = index;
      type.limits.flags = code.at(index++);
      auto init = parse_varuint<32>( code, index );
      index += init.size;
      type.limits.initial = init.get();
      if (type.limits.flags) {
         auto max = parse_varuint<32>( code, index );
         index += max.size;
         type.limits.maximum = max.get();
      }
      return index - orig_index;
   }

   size_t binary_parser::parse_table_type( const wasm_code& code, size_t index, table_type& type ) {
      const size_t orig_index = index;
      type.element_type = code.at(index++);
      type.limits.flags = code.at(index++);
      auto init = parse_varuint<32>( code, index );
      index += init.size;
      type.limits.initial = init.get();
      if (type.limits.flags) {
         auto max = parse_varuint<32>( code, index );
         index += max.size;
         type.limits.maximum = max.get();
      }
      return index - orig_index;
   } 

   size_t binary_parser::parse_import_entry( const wasm_code& code, size_t index, import_entry& entry ) {
      varuint<32> module_len = parse_varuint<32>( code, index );
      const size_t orig_index = index;
      entry.module_len = module_len.get();
      entry.module_str.resize(entry.module_len);
      index += module_len.size;
      memcpy( (char*)entry.module_str.data(), code.data()+index, entry.module_len );
      index += entry.module_len;
      varuint<32> field_len = parse_varuint<32>( code, index );
      entry.field_len = field_len.get();
      entry.field_str.resize(entry.field_len);
      index += field_len.size;
      memcpy( (char*)entry.field_str.data(), code.data()+index, entry.field_len );
      index += entry.field_len;
      entry.kind = (external_kind)code.at(index);
      index += 1;
      switch (entry.kind) {
         case external_kind::Function: {
            auto type = parse_varuint<32>( code, index );
            entry.type.func_t = type.get();
            index += type.size;
            break;
         }
         case external_kind::Table : {
            index += parse_table_type( code, index, entry.type.table_t );
            break;
         }
         case external_kind::Global : {
            index += parse_global_type( code, index, entry.type.global_t );
            break;
         }
         case external_kind::Memory : {
            index += parse_memory_type( code, index, entry.type.mem_t );
            break;
         }
      }
      return index - orig_index;
   }

   size_t binary_parser::parse_import_section( const wasm_code& code, size_t index, std::vector<import_entry>& imports ) {
      varuint<32> count = parse_varuint<32>( code, index );
      const size_t orig_index = index;
      index += count.size;
      
      imports.resize(count.get());  
      for ( int i=0; i < count.get(); i++ ) {
         import_entry entry;
         index += parse_import_entry( code, index, entry );
         imports[i] = entry;
      }
      return index - orig_index;
   }

   size_t binary_parser::parse_function_section( const wasm_code& code, size_t index, std::vector<uint32_t>& indices ) {
      const size_t orig_index = index;
      auto count = parse_varuint<32>( code, index );
      index += count.size;
      indices.resize( count.get() );
      for ( int i=0; i < count.get(); i++ ) {
         auto findex = parse_varuint<32>( code, index );
         index += findex.size;
         indices[i] = findex.get();
      }

      return index - orig_index;
   }

   size_t binary_parser::parse_table_section( const wasm_code& code, size_t index, std::vector<table_type>& types ) {
      const size_t orig_index = index;
      auto count = parse_varuint<32>( code, index );
      index += count.size;
      types.resize( count.get() );
      for ( int i=0; i < count.get(); i++ ) {
         table_type tmp;
         index += parse_table_type( code, index, tmp );
         types[i] = tmp;
      }

      return index - orig_index;
   }

   size_t binary_parser::parse_memory_section( const wasm_code& code, size_t index, binary_parser_vec<memory_type>& types ) {
      const size_t orig_index = index;
      auto count = parse_varuint<32>( code, index );
      index += count.size;
      types.resize( count.get() );
      for ( int i=0; i < count.get(); i++ ) {
         memory_type tmp;
         index += parse_memory_type( code, index, tmp );
         types.at(i) = tmp;
         //types[i] = tmp;
      }

      return index - orig_index;
   }

   size_t binary_parser::parse_global_section( const wasm_code& code, size_t index, std::vector<global_type>& globals ) {
      const size_t orig_index = index;
      auto count = parse_varuint<32>( code, index );
      index += count.size;
      globals.resize( count.get() );
      for ( int i=0; i < count.get(); i++ ) {
      }
   }
}} // namespace eosio::wasm_backend
