#pragma once

#include <eosio/vm/exceptions.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace eosio { namespace vm {
   inline static constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();
   // TODO after C++20 use std::span as fundamental type
   template <typename T, std::size_t Extent = dynamic_extent>
   class span {
      static_assert( Extent == 1 || Extent == dynamic_extent,
            "only a static extent of 1 is currently available" );
      public:
         using element_type = T;
         using value_type = std::remove_cv_t<T>;
         using size_type = std::size_t;
         using difference_type = std::ptrdiff_t;
         using pointer = T*;
         using const_pointer = const T*;
         using reference = T&;
         using const_reference = const T&;
         using iterator = T*;
         template <typename It>
         inline constexpr span(It first, std::size_t len) : first_elem(first), last_elem(first + len - 1) {}
         template <typename It>
         inline constexpr span(It first, It last) : first_elem(first), last_elem(last) {
            EOS_VM_ASSERT(last >= first, span_exception, "last iterator < first iterator");
         }

         template <std::size_t N>
         inline constexpr span(T (&arr)[N]) : first_elem(&arr[0]), last_elem(&arr[N-1]) {}

         template <std::size_t N>
         inline constexpr span(std::array<T, N>& arr) : first_elem(arr.data()), last_elem(arr.data() + (N-1)) {}

         template <std::size_t N>
         inline constexpr span(const std::array<T, N>& arr) : first_elem(arr.data()), last_elem(arr.data() + (N-1)) {}
         inline constexpr span(const span&) = default;
         inline constexpr span(span&&) = default;

         inline constexpr iterator begin()  { return first_elem; }
         inline constexpr iterator end()    { return last_elem + 1; }
         inline constexpr iterator rbegin() { return last_elem; }
         inline constexpr iterator rend()   { return first_elem - 1; }

         inline constexpr T& front()             { return *first_elem; }
         inline constexpr const T& front() const { return *first_elem; }
         inline constexpr T& back()              { return *last_elem; }
         inline constexpr const T& back() const  { return *last_elem; }
         inline constexpr T& operator[](std::size_t n) { return first_elem[n]; }
         inline constexpr const T& operator[](std::size_t n) const { return first_elem[n]; }
         inline constexpr T& at(std::size_t n) {
            EOS_VM_ASSERT(first_elem + n <= last_elem, span_exception, "index overflows span");
            return operator[](n);
         }
         inline constexpr const T& at(std::size_t n) const {
            EOS_VM_ASSERT(first_elem + n <= last_elem, span_exception, "index overflows span");
            return operator[](n);
         }

         inline constexpr T* data() { return first_elem; }
         inline constexpr const T* data() const { return first_elem; }
         inline constexpr std::size_t size() const { return last_elem - first_elem; }
         inline constexpr std::size_t size_bytes() const { return size() * sizeof(T); }
         inline constexpr bool empty() const { return size() == 0; }

         inline constexpr span first(std::size_t len) const {
            EOS_VM_ASSERT(first_elem + len <= last_elem, span_exception, "length overflows span");
            return {first_elem, first_elem + len - 1};
         }

         inline constexpr span last(std::size_t len) const {
            EOS_VM_ASSERT(last_elem - len >= first_elem, span_exception, "length underflows span");
            return {last_elem - len + 1, last_elem};
         }

         inline constexpr span subspan(std::size_t offset, std::size_t len) const {
            EOS_VM_ASSERT(first_elem + offset + len <= last_elem, span_exception, "length overflows span");
            return {first_elem + offset, len};
         }

         bool operator==(const span& other) const {
            return first_elem == other.first_elem && last_elem == other.last_elem;
         }

      private:
         iterator first_elem;
         iterator last_elem;
   };

   namespace detail {
      template <typename T>
      constexpr std::true_type is_span_type(span<T>) { return {}; }
      template <typename T>
      constexpr std::false_type is_span_type(T) { return {}; }
   } // ns eosio::vm::detail

   template <typename T>
   constexpr inline static bool is_span_type_v = std::is_same_v<decltype(detail::is_span_type(std::declval<T>())), std::true_type>;

}} // ns eosio::vm
