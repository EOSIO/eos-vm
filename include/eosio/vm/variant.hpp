#pragma once

#include <algorithm>
#include <eosio/vm/utils.hpp>
#include <iostream>
#include <tuple>
#include <type_traits>

namespace eosio { namespace vm {

// implementation details
namespace detail {

template <typename T, typename... Ts>
struct max_layout_size {
   static constexpr auto value = std::max( sizeof(T), max_layout_size<Ts...>::value );
};

template <typename T>
struct max_layout_size<T> {
   static constexpr auto value = sizeof(T);
};

template <typename T, typename... Ts>
struct max_alignof {
   static constexpr auto value = std::max(alignof(T), max_layout_size<Ts...>::value);
};

template <typename T>
struct max_alignof<T> {
   static constexpr auto value = alignof(T);
};

template <typename T, typename Alternative, typename... Alternatives>
struct is_valid_alternative {
   static constexpr auto value =
         std::is_same<T, Alternative>::value ? true : is_valid_alternative<T, Alternatives...>::value;
};

template <typename T, typename Alternative>
struct is_valid_alternative<T, Alternative> {
   static constexpr auto value = std::is_same<T, Alternative>::value;
};

template <size_t N, typename T, typename Alternative, typename... Alternatives>
struct get_alternatives_index {
   static constexpr auto value =
         std::is_same<T, Alternative>::value ? N : get_alternatives_index<N + 1, T, Alternatives...>::value;
};

template <size_t N, typename T, typename Alternative>
struct get_alternatives_index<N, T, Alternative> {
   static constexpr auto value = N;
};

} // namespace detail

template <typename... Alternatives>
class variant {
   static_assert(sizeof...(Alternatives) < std::numeric_limits<uint8_t>::max(),
                 "eosio::vm::variant can only accept 255 alternatives");

 public:
   template <typename T>
   inline constexpr variant(T&& alt) {
      static_assert(detail::is_valid_alternative<T, Alternatives...>::value, "type not a valid alternative");
      new(&_storage) T(std::forward<T>(alt));
      _which = detail::get_alternatives_index<0, T, Alternatives...>::value;
   }

   uint8_t index() const { return _which; }

   template <size_t Index>
   inline constexpr auto&& get_check() {
      return ;
   }

   template <size_t Index>
   inline constexpr auto&& get() {
      return *reinterpret_cast<typename std::tuple_element<Index, alternatives_tuple>::type*>(&_storage);
   }

   template <typename Alt>
   inline constexpr auto&& get() {
      return *reinterpret_cast<Alt*>(&_storage);
   }

 private:
   using alternatives_tuple = std::tuple<Alternatives...>;
   static constexpr size_t _sizeof  = detail::max_layout_size<Alternatives...>::value;
   static constexpr size_t _alignof = detail::max_alignof<Alternatives...>::value;
   std::aligned_storage<_sizeof, _alignof> _storage;
   uint8_t                                 _which = 0;
};

}} // namespace eosio::vm
