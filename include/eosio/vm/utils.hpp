#pragma once

#include <eosio/vm/debug.hpp>

#include <cstdlib>
#include <functional>
#include <string>
#include <memory>
#include <cxxabi.h>

#if !defined(LIKELY) && !defined(UNLIKELY)
#   if defined(__GNUC__)
#      if (__GNUC__ > 5) || defined(__clang__)
#         define LIKELY(x) __builtin_expect(!!(x), 1)
#         define UNLIKELY(x) __builtin_expect(!!(x), 0)
#      else
#         define LIKELY(x) !!(x)
#         define UNLIKELY(x) !!(x)
#      endif
#   else
#      define LIKELY(x) !!(x)
#      define UNLIKELY(x) !!(x)
#   endif
#endif

namespace eosio { namespace vm {
   // helpers for std::visit
   template <class... Ts>
   struct overloaded : Ts... {
      using Ts::operator()...;
   };
   template <class... Ts>
   overloaded(Ts...)->overloaded<Ts...>;

   // helpers for handling void returns
   struct maybe_void_t {
      template <typename T>
      inline constexpr friend T&& operator, (T&& val, maybe_void_t) {return std::forward<T>(val);}
   };

   inline maybe_void_t maybe_void;

   // simple utility function to demangle C++ type names
   static inline std::string demangle(const char* mangled_name) {
      size_t                                          len    = 0;
      int                                             status = 0;
      ::std::unique_ptr<char, decltype(&::std::free)> ptr(
            __cxxabiv1::__cxa_demangle(mangled_name, nullptr, &len, &status), &::std::free);
      return ptr.get();
   }

   template<typename F>
   struct scope_guard {
      scope_guard(F&& f) : _f(static_cast<F&&>(f)) {}
      ~scope_guard() { _f(); }
      F _f;
   };

   template<typename... T>
   void ignore_unused_variable_warning(T&...) {}

   template <char...Str>
   struct compile_time_string {
      static constexpr const char value[] = {Str..., '\0'};
   };

}} // namespace eosio::vm

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
   template <typename T, T... Str>
   inline constexpr auto operator""_cts() {
      constexpr auto x = eosio::vm::compile_time_string<Str...>{};
      return x;
   }
#pragma clang diagnostic pop
