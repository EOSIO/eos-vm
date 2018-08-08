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

         void parse( std::vector<uint8_t> code );

         template <size_t N>
         uint32_t parse_varuint( wasm_code_ptr& code ) {
            varuint<N> result(0);
            result.set( code );
            return result.get();
         }

         inline uint32_t parse_magic( wasm_code_ptr& code ) {
            const auto magic = *((uint32_t*)code.raw()); 
            code += sizeof(uint32_t);
            return magic;
         }
         inline uint32_t parse_version( wasm_code_ptr& code ) {
            const auto version = *((uint32_t*)code.raw()); 
            code += sizeof(uint32_t);
            return version;
         }
         inline uint8_t parse_section_id ( wasm_code_ptr& code ) {
            return *code++;
         }
         inline uint32_t parse_section_payload_len( wasm_code_ptr& code ) {
            return parse_varuint<32>( code );
         }

         import_entry parse_import_entry( wasm_code_ptr& code );
         table_type parse_table_type( wasm_code_ptr& code );
         global_variable parse_global_variable( wasm_code_ptr& code );
         memory_type parse_memory_type( wasm_code_ptr& code );

         void parse_type_section( wasm_code_ptr& code, binary_parser_vec<func_type>& types );
         inline void parse_import_section( wasm_code_ptr& code, binary_parser_vec<import_entry>& imports ) {
            auto count = parse_varuint<32>( code );
            imports.resize(count);  
            for ( int i=0; i < count; i++ ) 
               imports.at(i) = parse_import_entry( code );
         }
         inline void parse_function_section( wasm_code_ptr& code, binary_parser_vec<uint32_t>& indices ) {
            auto count = parse_varuint<32>( code );
            indices.resize(count);  
            for ( int i=0; i < count; i++ ) 
               indices.at(i) = parse_varuint<32>( code );
         }
         inline void parse_table_section( wasm_code_ptr& code, binary_parser_vec<table_type>& types ) {
            auto count = parse_varuint<32>( code );
            types.resize( count );
            for ( int i=0; i < count; i++ ) 
               types.at(i) = parse_table_type( code );
         }
         inline void parse_memory_section( wasm_code_ptr& code, binary_parser_vec<memory_type>& types ) {
            auto count = parse_varuint<32>( code );
            types.resize( count );
            for ( int i=0; i < count; i++ ) 
               types.at(i) = parse_memory_type( code );
         }
         inline void parse_global_section( wasm_code_ptr& code, binary_parser_vec<global_variable>& types ) {
            auto count = parse_varuint<32>( code );
            types.resize( count );
            for ( int i=0; i < count; i++ ) 
               types.at(i) = parse_global_variable( code );
         }

         init_expr parse_init_expr( wasm_code_ptr& code );

         size_t parse_custom_section( const wasm_code& code, size_t index );

         template <size_t N>
         varint<N> parse_varint( const wasm_code& code, size_t index ) {
            varint<N> result(0);
            result.set( code, index );
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
         //std::vector<uint8_t> _code;
         //std::vector<uint8_t> _decoded;
         wasm_allocator& _allocator;
   };
}} // namespace eosio::wasm_backend
