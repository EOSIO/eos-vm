#pragma once

#include <eosio/vm/compile_time_string.hpp>

#include <tuple>
#include <type_traits>

namespace eosio { namespace vm { namespace detail {

template <typename Str, typename T>
struct bound_pair {
  inline constexpr bound_pair(Str str, T ty) {}
  using first = Str;
  using second = T;
};

template <uint64_t N, typename Tst, typename Str, typename... Strs>
struct get_index {
  static constexpr uint64_t value = get_index<N+1, Tst, Strs...>::value;
};

template <uint64_t N, typename Tst>
struct get_index<N, Tst, Tst> {
  static constexpr uint64_t value = N;
};

template <uint64_t N, typename Tst, typename... Strs>
struct get_index<N, Tst, Tst, Strs...> {
  static constexpr uint64_t value = N;
};

template <typename... Pairs>
struct bound_tuple {
  std::tuple<typename Pairs::second...> body;

  inline constexpr bound_tuple(Pairs...) {}

  template <typename Str>
  inline constexpr auto&& get() && {
     return std::get<get_index<0, Str, typename Pairs::first...>::value>(body);
  }

  template <typename Str>
  inline constexpr auto& get() {
     return std::get<get_index<0, Str, typename Pairs::first...>::value>(body);
  }

  template <typename Str>
  inline constexpr const auto& get() const {
     return std::get<get_index<0, Str, typename Pairs::first...>::value>(body);
  }
};

}}}
