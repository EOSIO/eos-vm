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

}} // eosio::wasm_backend
