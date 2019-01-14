#pragma once

#include <vector>
//#include <variant>

#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/vector.hpp>
#include <eosio/wasm_backend/constants.hpp>
#include <eosio/wasm_backend/sections.hpp>

namespace eosio { namespace wasm_backend {
   class binary_parser {
      public:
         template <typename T>
         using vec = native_vector<T>;

         template <size_t N>
         static inline uint64_t parse_varuint( wasm_code_ptr& code ) {
            varuint<N> result(0);
            result.set( code );
            return result.get();
         }

         template <size_t N>
         static inline uint64_t parse_varint( wasm_code_ptr& code ) {
            varint<N> result(0);
            result.set( code );
            return result.get();
         }
        
         void parse_module( wasm_code& code, module& mod );

         inline uint32_t parse_magic( wasm_code_ptr& code ) {
            code.add_bounds( constants::magic_size );
            const auto magic = *((uint32_t*)code.raw()); 
            code += sizeof(uint32_t);
            return magic;
         }
         inline uint32_t parse_version( wasm_code_ptr& code ) {
            code.add_bounds( constants::version_size );
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
         void parse_init_expr( wasm_code_ptr& code, init_expr& ie );
         void parse_function_body( wasm_code_ptr& code, function_body& fb );
         void parse_function_body_code( wasm_code_ptr& code, size_t bounds, native_vector<opcode>& code_bytes );
         void parse_data_segment( wasm_code_ptr& code, data_segment& ds );

         template <typename Elem, typename ParseFunc>
         inline void parse_section_impl( wasm_code_ptr& code, vec<Elem>& elems, ParseFunc&& elem_parse ) {
            auto count = parse_varuint<32>( code );
            elems.resize(count);
            for (int i=0; i < count; i++ )
               elem_parse(code, elems.at(i));
         }
         
         template <uint8_t id> 
         inline void parse_section_header( wasm_code_ptr& code ) {
            code.add_bounds( constants::id_size );
            auto _id = parse_section_id( code );
            // ignore custom sections
            if ( _id == section_id::custom_section ) {
               code.add_bounds( constants::varuint32_size );
               code += parse_section_payload_len( code );
               code.fit_bounds( constants::id_size );
               _id = parse_section_id( code );
            } 
            EOS_WB_ASSERT( _id == id, wasm_parse_exception, "Section id does not match" );
            code.add_bounds( constants::varuint32_size );
            code.fit_bounds(parse_section_payload_len( code ));
         }

         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code, 
               vec<typename std::enable_if_t<id == section_id::type_section, func_type>>& elems ) {
            parse_section_impl( code, elems, [&](wasm_code_ptr& code, func_type& ft) { parse_func_type( code, ft ); } );
         }
         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code,
               vec<typename std::enable_if_t<id == section_id::import_section, import_entry>>& elems ) {
            parse_section_impl( code, elems, [&](wasm_code_ptr& code, import_entry& ie) { parse_import_entry(code, ie); } );
         }
         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code,
               vec<typename std::enable_if_t<id == section_id::function_section, uint32_t>>& elems ) {
            parse_section_impl( code, elems, [&](wasm_code_ptr& code, uint32_t& elem) { elem = parse_varuint<32>( code ); } );
         }
         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code,
               vec<typename std::enable_if_t<id == section_id::table_section, table_type>>& elems ) {
            parse_section_impl( code, elems, [&](wasm_code_ptr& code, table_type& tt) { parse_table_type( code, tt ); } );
         }
         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code,
               vec<typename std::enable_if_t<id == section_id::memory_section, memory_type>>& elems ) {
            parse_section_impl( code, elems, [&](wasm_code_ptr& code, memory_type& mt) { parse_memory_type( code, mt ); } );
         }
         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code,
               vec<typename std::enable_if_t<id == section_id::global_section, global_variable>>& elems ) {
            parse_section_impl( code, elems, [&](wasm_code_ptr& code, global_variable& gv) { parse_global_variable( code, gv ); } );
         }
         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code,
               vec<typename std::enable_if_t<id == section_id::export_section, export_entry>>& elems ) {
            parse_section_impl( code, elems, [&](wasm_code_ptr& code, export_entry& ee) { parse_export_entry( code, ee ); } );
         }
         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code, 
               typename std::enable_if_t<id == section_id::start_section, uint32_t>& start ) {
            start = parse_varuint<32>( code );
         }
         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code,
               vec<typename std::enable_if_t<id == section_id::element_section, elem_segment>>& elems ) {
            parse_section_impl( code, elems, [&](wasm_code_ptr& code, elem_segment& es) { parse_elem_segment( code, es ); } );
         }
         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code,
               vec<typename std::enable_if_t<id == section_id::code_section, function_body>>& elems ) {
            parse_section_impl( code, elems, [&](wasm_code_ptr& code, function_body& fb) { parse_function_body( code, fb ); } );
         }
         template <uint8_t id> 
         inline void parse_section( wasm_code_ptr& code,
               vec<typename std::enable_if_t<id == section_id::data_section, data_segment>>& elems ) {
            parse_section_impl( code, elems, [&](wasm_code_ptr& code, data_segment& ds) { parse_data_segment( code, ds ); } );
         }
         
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
   };
}} // namespace eosio::wasm_backend
