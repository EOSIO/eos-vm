#pragma once

#if !defined(LIKELY) && !defined(UNLIKELY)
#if __has_builtin(__builtin_expect)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x) !!(x)
#define UNLIKELY(x) !!(x)
#endif
#endif

namespace eosio { namespace vm {
   // helpers for std::visit
   template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
   template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}} // namespace eosio::vm
