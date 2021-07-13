#pragma once

#include <eosio/vm/utils.hpp>

#include <cstdint>
#include <exception>

#define EOS_VM_ASSERT( expr, exc_type, msg ) \
   if (!UNLIKELY(expr)) {                    \
      throw exc_type{msg};                   \
   }

namespace eosio { namespace vm {
   struct exception : public std::exception {
      virtual const char* what()const throw()=0;
      virtual const char* detail()const throw()=0;
   };
}}

#define DECLARE_EXCEPTION(name, _code, _what)                   \
   struct name : public eosio::vm::exception {                  \
      name(std::string msg) : msg(msg) {}                       \
      virtual const char* what()const throw() { return _what; } \
      virtual const char* detail()const throw() { return msg.c_str(); } \
      uint32_t code()const { return _code; }                    \
      std::string msg;                                          \
   };

namespace eosio { namespace vm {
   DECLARE_EXCEPTION( wasm_interpreter_exception,        4000000, "wasm interpreter exception" )
   DECLARE_EXCEPTION( wasm_section_length_exception,     4000001, "wasm section length exception" )
   DECLARE_EXCEPTION( wasm_bad_alloc,                    4000002, "wasm allocation failed" )
   DECLARE_EXCEPTION( wasm_double_free,                  4000003, "wasm free failed" )
   DECLARE_EXCEPTION( wasm_vector_oob_exception,         4000004, "wasm vector out of bounds" )
   DECLARE_EXCEPTION( wasm_unsupported_import_exception, 4000005, "wasm interpreter only accepts function imports" )
   DECLARE_EXCEPTION( wasm_parse_exception,              4000006, "wasm parse exception" )
   DECLARE_EXCEPTION( wasm_memory_exception,             4000007, "wasm memory exception" )
   DECLARE_EXCEPTION( stack_memory_exception,            4000008, "stack memory exception" )
   DECLARE_EXCEPTION( wasm_invalid_element,              4000009, "wasm invalid_element" )
   DECLARE_EXCEPTION( wasm_link_exception,               4000010, "wasm linked function failure" )
   DECLARE_EXCEPTION( guarded_ptr_exception,             4010000, "pointer out of bounds" )
   DECLARE_EXCEPTION( timeout_exception,                 4010001, "timeout" )
   DECLARE_EXCEPTION( wasm_exit_exception,               4010002, "exit" )
   DECLARE_EXCEPTION( span_exception,                    4020000, "span exception" )
   DECLARE_EXCEPTION( profile_exception,                 4030000, "profile exception" )
}} // eosio::vm

#undef DECLARE_EXCEPTION
