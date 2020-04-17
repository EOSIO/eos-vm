#pragma once

#include <eosio/vm/types.hpp>
#include <eosio/vm/span.hpp>

#include <memory>
namespace eosio { namespace vm {

   template <typename T, std::size_t LegacyAlign=0>
   struct reference_proxy {
      static_assert(LegacyAlign % alignof(T) == 0, "Specified alignment must be at least alignment of T");
      static_assert(std::is_trivially_copy_constructible_v<T>, "reference_proxy requires a trivially copyable type");
      using internal_type = std::conditional_t<std::is_pointer_v<T>, std::remove_pointer_t<T>, std::remove_reference_t<T>>;
      inline constexpr reference_proxy(internal_type& val) : reference_proxy(std::addressof(val)) {}
      inline constexpr reference_proxy(void* ptr) : original_ptr(ptr) {
         if (!LegacyAlign || reinterpret_cast<std::uintptr_t>(original_ptr) % LegacyAlign != 0) {
            copy.emplace();
            memcpy( std::addressof(*copy), original_ptr, sizeof(internal_type) );
         }
      }
      inline constexpr reference_proxy(const reference_proxy&) = delete;
      inline constexpr reference_proxy(reference_proxy&& other) : original_ptr(other.original_ptr), copy(other.copy) {
         other.copy.reset();
         other.original_ptr = nullptr;
      }
      inline ~reference_proxy() {
         if constexpr (!std::is_const_v<internal_type>)
            if (copy)
               memcpy( original_ptr, std::addressof(*copy), sizeof(internal_type) );
      }
      constexpr operator internal_type*() const {
         if (copy)
            return std::addressof(*copy);
         else
            return original_ptr;
      }

      constexpr operator internal_type&() const {
         if (copy)
            return *copy;
         else
            return *static_cast<internal_type*>(original_ptr);
      }

      constexpr internal_type* get() { return (internal_type*)*this; }
      constexpr internal_type& ref() {
         return (internal_type&)*this;
      }
      constexpr internal_type* operator->() { return (internal_type*)*this; }

      static constexpr bool is_legacy() { return LegacyAlign != 0; }
      using dependent_type = T;

      void* original_ptr;
      mutable std::optional<std::remove_cv_t<internal_type>> copy;
   };

   template <typename T, std::size_t LegacyAlign>
   struct reference_proxy<span<T>, LegacyAlign> {
      static_assert(LegacyAlign % alignof(T) == 0, "Specified alignment must be at least alignment of T");
      static_assert(std::is_trivially_copy_constructible_v<T>, "reference_proxy requires a trivially copyable type");
      inline constexpr bool is_aligned(void* ptr) { return reinterpret_cast<std::uintptr_t>(original_ptr) % LegacyAlign == 0; }
      inline constexpr reference_proxy(void* ptr, uint32_t size)
         : original_ptr(ptr),
           copy( is_aligned(ptr) ? nullptr : new std::remove_cv_t<T>[size] ),
           _span( copy ? copy.get() : static_cast<T*>(ptr), size ) {
         if (copy)
            memcpy( copy.get(), original_ptr, _span.size_bytes() );
      }
      inline constexpr reference_proxy(const reference_proxy&) = delete;
      inline constexpr reference_proxy(reference_proxy&&) = default;
      inline ~reference_proxy() {
         if constexpr (!std::is_const_v<T>)
            if (copy)
               memcpy( original_ptr, copy.get(), _span.size_bytes() );
      }
      constexpr operator T*() {
         return _span.data();
      }
      constexpr operator const T*() const {
         return _span.data();
      }
      constexpr operator span<T>&() {
         return _span;
      }
      constexpr operator const span<T>&() const {
         return _span;
      }

      constexpr span<T>& ref() { return _span; }
      constexpr span<T>* operator->() { return &_span; }

      static constexpr bool is_legacy() { return false; }

      void* original_ptr;
      std::unique_ptr<std::remove_cv_t<T>[]> copy = nullptr;
      span<T> _span;
      using dependent_type = T;
   };

   namespace detail {
      template <typename T>
      constexpr inline std::true_type is_reference_proxy_type(reference_proxy<T>);
      template <typename T, std::size_t LA>
      constexpr inline std::true_type is_reference_proxy_type(reference_proxy<T, LA>);
      template <typename T>
      constexpr inline std::false_type is_reference_proxy_type(T);
   }

   template <typename T>
   constexpr inline static bool is_reference_proxy_type_v = std::is_same_v<decltype(detail::is_reference_proxy_type(std::declval<T>())), std::true_type>;

   namespace detail {
      template <typename T>
      constexpr auto get_dependent_type() -> std::enable_if_t<is_reference_proxy_type_v<T>, typename T::dependent_type>;
      template <typename T>
      constexpr auto get_dependent_type() -> std::enable_if_t<!is_reference_proxy_type_v<T>, T>;
      // Help OSX clang
      template <typename U, std::size_t A>
      auto get_dependent_type(reference_proxy<U, A>) -> U;
      template <typename U>
      auto get_dependent_type(reference_proxy<U, 0>) -> U;
      template <typename T, std::size_t LA>
      constexpr inline std::integral_constant<bool, LA != 0> is_legacy(reference_proxy<T, LA>);
      template <typename T>
      constexpr inline std::false_type is_legacy(T);
   } // ns eosio::vm::detail

   template <typename T>
   using reference_proxy_dependent_type_t = decltype(detail::get_dependent_type<T>());

   template <typename T>
   constexpr inline bool is_reference_proxy_legacy_v = std::is_same_v<decltype(detail::is_legacy(std::declval<T>())), std::true_type>;

}} // ns eosio::vm
