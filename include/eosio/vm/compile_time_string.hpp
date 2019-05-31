#pragma once

namespace eosio { namespace vm {
   template <char... Str>
   struct compile_time_string {
      static constexpr const char value[] = { Str... };
      using t = compile_time_string<Str...>;
   };
}} // namespace eosio::vm

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
template <typename T, T... Str>
inline constexpr auto operator""_c() {
   return eosio::vm::compile_time_string<Str...>{};
}
#pragma clang diagnostic pop

#define id(X) #X ## _c
#define id_t(X) decltype(#X ## _c)
