#pragma once

#include <iostream>

namespace eosio { namespace vm {
#ifdef EOS_VM_DEBUG
   inline constexpr bool is_debug = true;
#else
   inline constexpr bool is_debug = false;
#endif

   template <typename F, typename... Args>
   constexpr inline void debug_stmt(F&& f, Args&&... args) {
      if constexpr (is_debug)
         F(std::forward<Args>(args)...);
   }

   inline void debug_print_impl(const std::string&, const char* s) {
      std::cerr << s << std::endl;
   }

   template <typename Arg, typename... Args>
   inline void debug_print_impl(const std::string& env, const char* fmt, Arg&& arg, Args&&... args) {
      std::cerr << env << " : ";
      while ( *fmt != '\0' ) {
         if ( *fmt == '%' ) {
            if constexpr (std::is_same_v<Arg, uint8_t>)
               std::cerr << std::hex << (int)arg << std::dec;
            else
               std::cerr << arg;
            debug_print_impl(env, fmt+1, args...);
            return;
         }
         std::cerr << fmt[0];
         fmt++;
      }
      std::cerr << fmt;
   }

#define EOS_VM_DEBUG(...) debug_stmt(__VA_ARGS__)

#define EOS_VM_DEBUG_OUTS(...) \
   debug_print_impl("", __VA_ARGS__);
   //debug_print_impl(std::string(__FILE__)+":"+std::to_string(__LINE__), __VA_ARGS__);

}} // ns eosio::vm
