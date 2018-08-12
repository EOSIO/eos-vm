#pragma once

#include <vector>

#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/vector.hpp>

namespace eosio { namespace wasm_backend {
   class binary_parser {
      public:
         template <typename T>
         using vec = native_vector<T>;

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

         void parse_import_entry( wasm_code_ptr& code, import_entry& ie );
         void parse_table_type( wasm_code_ptr& code, table_type& tt );
         void parse_global_variable( wasm_code_ptr& code, global_variable& gv );
         void parse_memory_type( wasm_code_ptr& code, memory_type& mt );
         void parse_export_entry( wasm_code_ptr& code, export_entry& ee );
         void parse_func_type( wasm_code_ptr& code, func_type& ft );
         void parse_elem_segment( wasm_code_ptr& code, elem_segment& es );



         template <typename Elem, typename ParseFunc>
         inline void parse_section( wasm_code_ptr& code, vec<Elem>& elems, ParseFunc&& elem_parse ) {
            auto count = parse_varuint<32>( code );
            elems.resize(count);
            for (int i=0; i < count; i++ )
               elem_parse(code, elems.at(i));
         }

         inline void parse_type_section( wasm_code_ptr& code, vec<func_type>& elems ) {
            parse_section( code, elems, [&](wasm_code_ptr& code, func_type& ft) { parse_func_type( code, ft ); } );
         }
         inline void parse_import_section( wasm_code_ptr& code, vec<import_entry>& elems ) {
            parse_section( code, elems, [&](wasm_code_ptr& code, import_entry& ie) { parse_import_entry(code, ie); } );
         }
         inline void parse_function_section( wasm_code_ptr& code, vec<uint32_t>& elems ) {
            parse_section( code, elems, [&](wasm_code_ptr& code, uint32_t& elem) { elem = parse_varuint<32>( code ); } );
         }
         inline void parse_table_section( wasm_code_ptr& code, vec<table_type>& elems ) {
            parse_section( code, elems, [&](wasm_code_ptr& code, table_type& tt) { parse_table_type( code, tt ); } );
         }
         inline void parse_memory_section( wasm_code_ptr& code, vec<memory_type>& elems ) {
            parse_section( code, elems, [&](wasm_code_ptr& code, memory_type& mt) { parse_memory_type( code, mt ); } );
         }
         inline void parse_global_section( wasm_code_ptr& code, vec<global_variable>& elems ) {
            parse_section( code, elems, [&](wasm_code_ptr& code, global_variable& gv) { parse_global_variable( code, gv ); } );
         }
         inline void parse_export_section( wasm_code_ptr& code, vec<export_entry>& elems ) {
            parse_section( code, elems, [&](wasm_code_ptr& code, export_entry& ee) { parse_export_entry( code, ee ); } );
         }
         inline void parse_start_section( wasm_code_ptr& code, uint32_t& start ) {
            start = parse_varuint<32>( code );
         }
         inline void parse_element_section( wasm_code_ptr& code, vec<elem_segment>& elems ) {
            parse_section( code, elems, [&](wasm_code_ptr& code, elem_segment& es) { parse_elem_segment( code, es ); } );
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

      private:
         //std::vector<uint8_t> _code;
         //std::vector<uint8_t> _decoded;
   };
}} // namespace eosio::wasm_backend
