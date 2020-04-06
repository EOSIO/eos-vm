#pragma once

#include <eosio/vm/types.hpp>
#include <eosio/vm/span.hpp>

#include <memory>
namespace eosio { namespace vm {

   template <typename T, bool LegacyAlign=false>
   struct reference_proxy {
      using internal_type = std::conditional_t<std::is_pointer_v<T>, std::remove_pointer_t<T>, std::remove_reference_t<T>>;
      inline constexpr reference_proxy(internal_type& val) : original_ptr(std::addressof(val)) {
         if (reinterpret_cast<std::uintptr_t>(original_ptr) % alignof(internal_type) != 0 || !LegacyAlign) {
            copy.reset(new std::remove_cv_t<internal_type>());
            memcpy( copy.get(), original_ptr, sizeof(internal_type) );
         }
      }
      inline constexpr reference_proxy(internal_type* ptr) : original_ptr(ptr) {
         if (reinterpret_cast<std::uintptr_t>(original_ptr) % alignof(internal_type) != 0 || !LegacyAlign) {
            copy.reset(new std::remove_cv_t<internal_type>());
            memcpy( copy.get(), original_ptr, sizeof(internal_type) );
         }
      }
      inline constexpr reference_proxy(const reference_proxy&) = delete;
      inline constexpr reference_proxy(reference_proxy&&) = default;
      inline ~reference_proxy() {
         if constexpr (!std::is_const_v<internal_type>)
            if (copy || !LegacyAlign)
               memcpy( original_ptr, copy.get(), sizeof(internal_type) );
      }
      constexpr operator internal_type*() const {
         if (copy || !LegacyAlign)
            return copy.get();
         else
            return original_ptr;
      }

      constexpr operator internal_type&() const {
         if (copy || !LegacyAlign)
            return *copy.get();
         else
            return *original_ptr;
      }

      constexpr internal_type* get() { return (internal_type*)*this; }
      constexpr internal_type& ref() {
         return (internal_type&)*this;
      }
      constexpr internal_type* operator->() { return (internal_type*)*this; }

      static constexpr bool is_legacy() { return LegacyAlign; }
      using dependent_type = T;

      internal_type* original_ptr;
      std::unique_ptr<std::remove_cv_t<internal_type>> copy = nullptr;
   };

   template <typename T>
   struct reference_proxy<span<T>> {
      inline constexpr bool is_aligned(T* ptr) { return reinterpret_cast<std::uintptr_t>(original_ptr) % alignof(T) == 0; }
      inline constexpr reference_proxy(T* ptr, uint32_t size)
         : original_ptr(ptr),
           copy( is_aligned(ptr) ? nullptr : new std::remove_cv_t<T>[size] ),
           _span( copy ? copy.get() : ptr, size ) {
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
      constexpr span<T>& operator->() { return _span; }

      static constexpr bool is_legacy() { return false; }

      T* original_ptr;
      std::unique_ptr<std::remove_cv_t<T>[]> copy = nullptr;
      span<T> _span;
      using dependent_type = T;
   };

   namespace detail {
      template <typename T>
      constexpr inline std::true_type is_reference_proxy_type(reference_proxy<T>);
      template <typename T, bool LA>
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
      template <typename U>
      auto get_dependent_type(reference_proxy<U, true>) -> U;
      template <typename U>
      auto get_dependent_type(reference_proxy<U, false>) -> U;
      template <typename T>
      constexpr inline std::true_type is_legacy(reference_proxy<T, true>);
      template <typename T>
      constexpr inline std::false_type is_legacy(T);
   } // ns eosio::vm::detail

   template <typename T>
   using reference_proxy_dependent_type_t = decltype(detail::get_dependent_type<T>());

   template <typename T>
   constexpr inline bool is_reference_proxy_legacy_v = std::is_same_v<decltype(detail::is_legacy<T>(std::declval<T>())), std::true_type>;

}} // ns eosio::vm
