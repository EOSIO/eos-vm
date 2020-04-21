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
         using reverse_iterator = std::reverse_iterator<iterator>;
         static constexpr std::size_t extent = Extent;

         template<std::size_t E = Extent, typename Enable = std::enable_if_t<E == dynamic_extent || E == 0>>
         constexpr span() noexcept : first_elem(nullptr), last_elem(nullptr) {}

         // Non-conforming: Only allows pointers, not any contiguous iterator.
         // Implementing a conforming version of this constructor requires C++20.
         inline constexpr span(pointer first, std::size_t len) : first_elem(first), last_elem(first + len) {
            EOS_VM_ASSERT(Extent == dynamic_extent || Extent == size(), span_exception, "Wrong size for span with static extent");
         }
         inline constexpr span(pointer first, pointer last) : first_elem(first), last_elem(last) {
            EOS_VM_ASSERT(last >= first, span_exception, "last iterator < first iterator");
            EOS_VM_ASSERT(Extent == dynamic_extent || Extent == size(), span_exception, "Wrong size for span with static extent");
         }

         template <std::size_t N, typename Enable = std::enable_if_t<(Extent == dynamic_extent || N == Extent)>>
         inline constexpr span(T (&arr)[N]) noexcept : first_elem(&arr[0]), last_elem(&arr[0] + N) {}

         template <typename U, std::size_t N,
                   typename Enable = std::enable_if_t<(Extent == dynamic_extent || N == Extent) && std::is_convertible_v<U(*)[], T(*)[]>>>
         inline constexpr span(std::array<U, N>& arr) noexcept : first_elem(arr.data()), last_elem(arr.data() + N) {}

         template <typename U, std::size_t N,
                   typename Enable = std::enable_if_t<(Extent == dynamic_extent || N == Extent) && std::is_convertible_v<const U(*)[], T(*)[]>>>
         inline constexpr span(const std::array<U, N>& arr) noexcept : first_elem(arr.data()), last_elem(arr.data() + N) {}

         // Not implemented:
         // template<typename R> span(R&&);
         // template<typename U, typename E> span(const span<U, E>&);

         inline constexpr span(const span&) = default;
         inline constexpr span(span&&) = default;
         inline constexpr span& operator=(const span&) = default;
         inline constexpr span& operator=(span&&) = default;

         inline constexpr iterator begin() const noexcept { return first_elem; }
         inline constexpr iterator end()   const noexcept { return last_elem; }
         inline constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator{last_elem}; }
         inline constexpr reverse_iterator rend()   const noexcept { return reverse_iterator{first_elem}; }

         inline constexpr T& front() const { return *first_elem; }
         inline constexpr T& back() const  { return *(last_elem - 1); }
         inline constexpr T& operator[](std::size_t n) const { return first_elem[n]; }

         inline constexpr T* data() const noexcept { return first_elem; }
         inline constexpr std::size_t size() const noexcept { return last_elem - first_elem; }
         inline constexpr std::size_t size_bytes() const noexcept { return size() * sizeof(T); }
         inline constexpr bool empty() const noexcept { return size() == 0; }

         inline constexpr span<T, dynamic_extent> first(std::size_t len) const {
            EOS_VM_ASSERT(len <= size(), span_exception, "length overflows span");
            return {first_elem, first_elem + len};
         }

         inline constexpr span<T, dynamic_extent> last(std::size_t len) const {
            EOS_VM_ASSERT(len <= size(), span_exception, "length underflows span");
            return {last_elem - len, last_elem};
         }

         inline constexpr span<T, dynamic_extent> subspan(std::size_t offset, std::size_t len = dynamic_extent) const {
            if(len == dynamic_extent) len = size() - offset;
            EOS_VM_ASSERT(first_elem + offset + len <= last_elem, span_exception, "length overflows span");
            return {first_elem + offset, len};
         }

      private:
         iterator first_elem;
         iterator last_elem;
   };

   // No deduction guides

   namespace detail {
      template <typename T>
      constexpr std::true_type is_span_type(span<T>) { return {}; }
      template <typename T>
      constexpr std::false_type is_span_type(T) { return {}; }
   } // ns eosio::vm::detail

   template <typename T>
   constexpr inline static bool is_span_type_v = std::is_same_v<decltype(detail::is_span_type(std::declval<T>())), std::true_type>;

}} // ns eosio::vm
