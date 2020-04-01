#pragma once

#include <cstdlib>
#include <functional>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <memory>
#include <vector>
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
   // helper to read a wasm file into a vector of bytes
   inline std::vector<uint8_t> read_wasm(const std::string& fname) {
      std::ifstream wasm_file(fname, std::ios::binary);
      if (!wasm_file.is_open())
         throw std::runtime_error("wasm file not found");
      wasm_file.seekg(0, std::ios::end);
      std::vector<uint8_t> wasm;
      int                  len = wasm_file.tellg();
      if (len < 0)
         throw std::runtime_error("wasm file length is -1");
      wasm.resize(len);
      wasm_file.seekg(0, std::ios::beg);
      wasm_file.read((char*)wasm.data(), wasm.size());
      wasm_file.close();
      return wasm;
   }

   // forward declarations
   struct i32_const_t;
   struct i64_const_t;
   struct f32_const_t;
   struct f64_const_t;

   template <typename StackElem>
   inline void print_result(const std::optional<StackElem>& result) {
      if(result) {
         std::cout << "result: ";
         if (result->template is_a<i32_const_t>())
            std::cout << "i32:" << result->to_ui32();
         else if (result->template is_a<i64_const_t>())
            std::cout << "i64:" << result->to_ui64();
         else if (result->template is_a<f32_const_t>())
            std::cout << "f32:" << result->to_f32();
         else if (result->template is_a<f64_const_t>())
           std::cout << "f64:" << result->to_f64();
         std::cout << std::endl;
     }
   }

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

   // helpers for creating subtuples
   namespace detail {
      template <size_t N, size_t I, typename T, typename... Ts>
      struct subtuple_impl;

      template <size_t N, size_t I, typename T, typename... Ts>
      struct subtuple_impl <N, I, std::tuple<T, Ts...>> {
         using type = decltype( std::tuple_cat( std::declval<std::tuple<T>>(),
                  std::declval<typename subtuple_impl<N, I+1, std::tuple<Ts...>>::type>() ) );
      };

      template <size_t N, typename T, typename... Ts>
      struct subtuple_impl <N, N, std::tuple<T, Ts...>> {
         using type = std::tuple<T>;
      };

      template <size_t N, typename T>
      using subtuple_t = typename subtuple_impl<N, 0, T>::type;

      template <typename>
      struct generate_subtuples_impl;

      template <typename T, typename... Ts>
      struct generate_subtuples_impl<std::tuple<T, Ts...>> {
         template <size_t... Is>
         static constexpr auto value( std::index_sequence<Is...> ) {
            return std::make_tuple(std::declval<subtuple_t<Is, std::tuple<T, Ts...>>>()...);
         }
      };

      template <typename T>
      using generate_subtuples_t = decltype(generate_subtuples_impl<T>::value( std::make_index_sequence<std::tuple_size_v<T>>{} ));

      template <typename T, typename... Ts>
      struct generate_all_subtuples_impl;

      template <typename T, typename... Ts>
      struct generate_all_subtuples_impl<std::tuple<T, Ts...>> {
         using type = decltype( std::tuple_cat( std::declval<typename generate_all_subtuples_impl<std::tuple<Ts...>>::type>(),
                                                std::declval<generate_subtuples_t<std::tuple<T, Ts...>>>() ) );
      };

      template <>
      struct generate_all_subtuples_impl<std::tuple<>> {
         using type = std::tuple<>;
      };

      template <typename Tuple>
      constexpr auto generate_all_subtuples( Tuple&& tup) {
         return std::declval<typename generate_all_subtuples_impl<Tuple>::type>();
      }

      template <size_t N>
      struct tuple_index {
         static constexpr size_t value = N;
      };

      template <size_t N, size_t I, typename Insert, typename Tuple>
      struct insert_type {
         using type = std::tuple<Insert>;
      };

      template <size_t N, size_t I, typename Insert, template<typename...> class Tuple, typename T, typename... Ts>
      struct insert_type<N, I, Insert, Tuple<T, Ts...>> {
         using type = decltype(std::tuple_cat(std::tuple<T>{}, typename insert_type<N, I+1, Insert, Tuple<Ts...>>::type{}));
      };

      template <size_t N, typename Insert, template<typename...> class Tuple, typename T, typename... Ts>
      struct insert_type<N, N, Insert, Tuple<T, Ts...>> {
         using type = std::tuple<Insert, T, Ts...>;
      };

      template <size_t I, typename Tuple, typename Indices>
      constexpr auto get_tuple_size_from_index() {
         if constexpr (I >= std::tuple_size_v<Tuple>)
            return -1;
         else
            return std::tuple_size_v<std::tuple_element_t<std::tuple_element_t<I, Indices>::value, Tuple>>;
      }

      template <size_t I, typename Tuple>
      constexpr auto get_tuple_size() {
         if constexpr (I >= std::tuple_size_v<Tuple>)
            return -1;
         else
            return std::tuple_size_v<std::tuple_element_t<I, Tuple>>;
      }

      template <size_t N, typename Insert, typename Tuple>
      using insert_type_t = typename insert_type<N, 0, Insert, Tuple>::type;

      template <size_t N, size_t I, typename Tuple, typename Indices>
      struct index_inserter {
         static constexpr int32_t size_of_element = get_tuple_size<N, Tuple>();

         template <size_t M = N, size_t J = I>
         static constexpr auto value(std::enable_if_t<M!=J, int> = 0) {
            if constexpr (size_of_element > get_tuple_size_from_index<I, Tuple, Indices>())
               return insert_type_t<I, tuple_index<N>, Indices>();
            else
               return index_inserter<N, I+1, Tuple, Indices>::value();
         }

         template <size_t M = N, size_t J = I>
         static constexpr auto value(std::enable_if_t<M==J, int> = 0) {
            return std::tuple_cat(Indices{}, std::tuple<tuple_index<N>>());
         }
      };

      template <size_t N, typename Tuple, typename Indices>
      using index_insert_t = decltype(index_inserter<N, 0, Tuple, Indices>::value());

      template <typename Tuple, typename Indices, size_t... Is>
      constexpr auto reorder_tuple(std::index_sequence<Is...>) {
         static_assert(std::tuple_size_v<Tuple> == std::tuple_size_v<Indices>);
         return std::tuple<std::tuple_element_t<std::tuple_element_t<Is, Indices>::value, Tuple>...>();
      }

      template <typename Tuple, typename Indices>
      using reorder_tuple_t = decltype(reorder_tuple<Tuple, Indices>(std::make_index_sequence<std::tuple_size_v<Indices>-1>{}));

      // sort the tuple by largest subtuple to smallest subtuple
      template <size_t N, size_t I, typename Tuple, typename Indices>
      constexpr auto tuple_sort() {
         if constexpr (N == I)
            return reorder_tuple_t<Tuple, index_insert_t<N, Tuple, Indices>>();
         else
            return tuple_sort<N, I+1, Tuple, index_insert_t<I, Tuple, Indices>>();
      }

      template <size_t N, size_t I, typename Tuple>
      struct tuple_trim;

      template <size_t N, size_t I, template <typename...> class Tuple, typename T, typename... Ts>
      struct tuple_trim<N, I, Tuple<T, Ts...>> {
         using type = typename tuple_trim<N, I+1, Tuple<Ts...>>::type;
      };

      template <size_t N, template <typename...> class Tuple, typename T, typename... Ts>
      struct tuple_trim<N, N, Tuple<T, Ts...>> {
         using type = Tuple<T, Ts...>;
      };
   } // namespace detail

   template <typename Tuple>
   using generate_all_subtuples_t = decltype(detail::generate_all_subtuples(std::declval<Tuple>()));

   // sort the tuple of subtuples by largest subtuple to smallest subtuple
   template <typename Tuple>
   using tuple_sort_t = decltype(detail::tuple_sort<std::tuple_size_v<Tuple>-1, 0, Tuple, std::tuple<>>());

   template <size_t N, typename Tuple>
   using tuple_trim_t = typename detail::tuple_trim<N, 0, Tuple>::type;
}} // namespace eosio::vm
