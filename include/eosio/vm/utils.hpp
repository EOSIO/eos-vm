#pragma once

/**
 * Utilities for a local counter.
 */
enum { COUNTER_BASE = __COUNTER__ };
#define LOCAL_COUNTER (__COUNTER__ - COUNTER_BASE)

/**
 * Macros for branch taken affinity.
 */
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
   /**
    * Helpers for visit to use overloaded lambdas.
    */
   template <class... Ts>
   struct overloaded : Ts... {
      using Ts::operator()...;
   };
   template <class... Ts>
   overloaded(Ts...)->overloaded<Ts...>;

}} // namespace eosio::vm
