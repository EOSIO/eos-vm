#pragma once

#include <vector>

#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/vector.hpp>
#include <eosio/wasm_backend/allocator.hpp>

namespace eosio { namespace wasm_backend {
   class binary_parser {
      public:
         template <typename T>
         using binary_parser_vec = vector<T, binary_parser>;

         binary_parser(wasm_allocator& alloc) : _allocator(alloc) {}
         binary_parser( std::vector<uint8_t> code, wasm_allocator& alloc ) : _code(code), _allocator(alloc) {
         }

         void parse( std::vector<uint8_t> code );
         void parse() { parse(_code); }
         size_t parse_magic( const wasm_code& code );
         size_t parse_version( const wasm_code& code, size_t index, uint32_t& version );
         size_t parse_section_id ( const wasm_code& code, size_t index, uint8_t& id );
         size_t parse_section_payload_len( const wasm_code& code, size_t index, varuint<32>& len );
         uint32_t parse_section_payload_len( wasm_code_ptr& code );
         size_t parse_section_payload_data( const wasm_code& code, size_t index, size_t len, wasm_bytes& bytes );
         size_t parse_custom_section( const wasm_code& code, size_t index );
         //size_t parse_type_section( const wasm_code& code, size_t index, binary_parser_vec<func_type>& types, size_t length );
         size_t parse_type_section( wasm_code_ptr& code, binary_parser_vec<func_type>& types );
         size_t parse_import_entry( const wasm_code& code, size_t index, import_entry& entry );
         size_t parse_import_section( const wasm_code& code, size_t index, std::vector<import_entry>& imports );
         size_t parse_table_type( const wasm_code& code, size_t index, table_type& type );
         size_t parse_global_type( const wasm_code& code, size_t index, global_type& type );
         size_t parse_memory_type( const wasm_code& code, size_t index, memory_type& type );
         
         size_t parse_function_section( const wasm_code& code, size_t index, std::vector<uint32_t>& indices );
         size_t parse_table_section( const wasm_code& code, size_t index, std::vector<table_type>& types );
         size_t parse_memory_section( const wasm_code& code, size_t index, binary_parser_vec<memory_type>& types );
         size_t parse_global_section( const wasm_code& code, size_t index, std::vector<global_type>& types );
         template <size_t N>
         varint<N> parse_varint( const wasm_code& code, size_t index ) {
            varint<N> result(0);
            result.set( code, index );
            return result;
         }

         template <size_t N>
         varuint<N> parse_varuint( wasm_code_ptr& code ) {
            varuint<N> result(0);
            result.set( code );
            return result;
         }

         template <size_t N>
         varuint<N> parse_varuint( const wasm_code& code, size_t index ) {
            varuint<N> result(0);
            result.set( code, index );
            return result;
         }
/*
         template <size_t N>
         varuint<N> parse_varuint( const wasm_code& code, size_t index ) {
            varuint<N> result(0);
            result.set( code, index );
            return result;
         }
*/
         wasm_allocator& get_allocator() {
            return _allocator;
         }


      private:
         std::vector<uint8_t> _code;
         std::vector<uint8_t> _decoded;
         wasm_allocator& _allocator;
   };
}} // namespace eosio::wasm_backend
