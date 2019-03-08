#pragma once

//#include <fc/exception/exception.hpp>
#include <exception>
#include <string>

//#define EOS_WB_ASSERT( expr, exc_type, FORMAT, ... )                  \
//   FC_MULTILINE_MACRO_BEGIN                                           \
//   if( !(expr) )                                                      \
//      FC_THROW_EXCEPTION( exc_type, FORMAT, __VA_ARGS__ );            \
//   FC_MULTILINE_MACRO_END
//
#define FC_LOG_AND_RETHROW() \
   catch (...) {             \
      throw;                 \
   }

#if __has_builtin(__builtin_expect)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x) !!(x) 
#define UNLIKELY(x) !!(x)
#endif

#define EOS_WB_ASSERT( expr, exc_type, msg ) \
   if (!UNLIKELY(expr)) {                    \
      throw exc_type{msg};                   \
   }

namespace eosio { namespace wasm_backend {
   struct exception : public std::exception {
      virtual const char* what()const throw()=0;
      virtual const char* detail()const throw()=0;
   };
}}

#define DECLARE_EXCEPTION(name, _code, _what)                                     \
   struct name : public eosio::wasm_backend::exception {                          \
      name(const char* msg) : msg(msg) {}                                         \
      virtual const char* what()const throw() { return _what; }                   \
      virtual const char* detail()const throw() { return msg; }                   \
      uint32_t code()const { return _code; }                                      \
      const char* msg;                                                            \
   };

namespace eosio { namespace wasm_backend {
   DECLARE_EXCEPTION( wasm_interpreter_exception,        4000000, "wasm interpreter exception" )
   DECLARE_EXCEPTION( wasm_section_length_exception,     4000001, "wasm section length exception" )
   DECLARE_EXCEPTION( wasm_bad_alloc,                    4000002, "wasm allocation failed" )
   DECLARE_EXCEPTION( wasm_double_free,                  4000003, "wasm free failed" )
   DECLARE_EXCEPTION( wasm_vector_oob_exception,         4000004, "wasm vector out of bounds" )
   DECLARE_EXCEPTION( wasm_unsupported_import_exception, 4000005, "wasm interpreter only accepts function imports" )
   DECLARE_EXCEPTION( wasm_parse_exception,              4000006, "wasm parse exception" )
   DECLARE_EXCEPTION( wasm_memory_exception,             4000007, "wasm memory exception" )
   DECLARE_EXCEPTION( wasm_invalid_element,              4000008, "wasm invalid_element" )
   DECLARE_EXCEPTION( guarded_ptr_exception,             4010000, "pointer out of bounds" )
}} // eosio::wasm_backend
