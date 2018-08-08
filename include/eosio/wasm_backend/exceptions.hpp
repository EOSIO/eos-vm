#pragma once

#include <fc/exception/exception.hpp>

#define EOS_WB_ASSERT( expr, exc_type, FORMAT, ... )                \
   FC_MULTILINE_MACRO_BEGIN                                           \
   if( !(expr) )                                                      \
      FC_THROW_EXCEPTION( exc_type, FORMAT, __VA_ARGS__ );            \
   FC_MULTILINE_MACRO_END

namespace eosio { namespace wasm_backend {
   FC_DECLARE_EXCEPTION( wasm_interpreter_exception,
                         4000000, "wasm_interpreter exception" )
   FC_DECLARE_EXCEPTION( wasm_unreachable_exception,
                         4000001, "wasm unreachable exception" )
   FC_DECLARE_EXCEPTION( wasm_illegal_opcode_exception,
                         4000002, "wasm illegal opcode exception" )
   FC_DECLARE_EXCEPTION( wasm_section_length_exception,
                         4000002, "wasm section length exception" )
   FC_DECLARE_EXCEPTION( wasm_bad_alloc,
                         4000003, "wasm allocation failed" )
   FC_DECLARE_EXCEPTION( wasm_vector_oob_exception,
                         4000004, "wasm vector out of bounds" )
   FC_DECLARE_EXCEPTION( wasm_unsupported_import_exception,
                         4000005, "wasm_interpreter only accepts function imports" )

   FC_DECLARE_EXCEPTION( guarded_ptr_exception,
                         4010000, "pointer out of bounds" )

}} // eosio::wasm_backend
