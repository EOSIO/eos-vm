#pragma once

#include <algorithm>
#include <eosio/vm/utils.hpp>
#include <outcome.hpp>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <variant>

namespace eosio { namespace vm {

// helpers for visit
//template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
//template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

//forward declaration
template <typename... Alternatives>
class variant;

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

template <typename Alternative, typename... Alternatives>
struct get_first_alternative {
   using type = Alternative;
};

template <size_t I, size_t N, typename... Alternatives>
struct get_alternative {
   using type = typename get_alternative<I+1, N, Alternatives...>::type;
};

template <size_t I, typename... Alternatives>
struct get_alternative<I, I, Alternatives...> {
   using type = typename get_first_alternative<Alternatives...>::type;
};

template <size_t I, class Visitor, template <typename...> class Variant, typename... Alts>
inline std::result_of_t<Visitor(typename get_alternative<0, I, Alts...>::type)> __visit(Visitor&&               vis,
                                                                                        const Variant<Alts...>& var) {
   if constexpr ( I < sizeof...(Alts) ) {
      if ( I == var.index() ) {
         return vis( var.template get<I>() );
      } else {
         return __visit<I+1>( std::forward<Visitor>(vis), var );
      }
   } else {
      //TODO add outcome stuff
   }
}
} // namespace detail

template <class Visitor, template<typename...> class Variant, typename... Alts>
constexpr decltype(auto) visit( Visitor&& vis, const Variant<Alts...>& var ) {
   return detail::__visit<0>( std::forward<Visitor>(vis), var );
}

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

   inline constexpr uint8_t index() const { return _which; }

   template <size_t Index>
   inline constexpr auto&& get_check() {
      // TODO add outcome stuff
      return 3;
   }

   template <size_t Index>
   inline constexpr const auto& get()const {
      return reinterpret_cast<const typename std::tuple_element<Index, alternatives_tuple>::type&>(_storage);
   }

   template <typename Alt>
   inline constexpr const auto& get()const {
      return reinterpret_cast<const Alt&>(_storage);
   }

   template <size_t Index>
   inline constexpr auto&& get() {
      return reinterpret_cast<typename std::tuple_element<Index, alternatives_tuple>::type&>(_storage);
   }

   template <typename Alt>
   inline constexpr auto&& get() {
      return reinterpret_cast<Alt&>(_storage);
   }

   inline constexpr void set_which(uint8_t which) { _which = which; }

   static constexpr size_t size() { return std::tuple_size_v<alternatives_tuple>; }

 private:
   using alternatives_tuple = std::tuple<Alternatives...>;
   static constexpr size_t _sizeof  = detail::max_layout_size<Alternatives...>::value;
   static constexpr size_t _alignof = detail::max_alignof<Alternatives...>::value;
   uint8_t                                 _which = 0;
   std::aligned_storage<_sizeof, _alignof> _storage;
};

}} // namespace eosio::vm
