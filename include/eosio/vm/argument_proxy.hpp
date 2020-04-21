#pragma once

#include <eosio/vm/types.hpp>
#include <eosio/vm/span.hpp>

#include <memory>
namespace eosio { namespace vm {

   // Used for pointer arguments to intrinsics.
   // T should be a type that points to external memory, such as a pointer or span.
   // The type that T points to must be trivially copyable.
   // argument_proxy copies unaligned memory into zero or more objects of the pointee type.
   // If LegacyAlign is non-zero it will only copy the memory if it is not aligned to LegacyAlign.
   // If the pointee is mutable, the memory will be written back by the destructor.
   template <typename T, std::size_t LegacyAlign=0>
   struct argument_proxy;

   // specialization of argument_proxy for pointers.
   // copies a single element.
   template <typename T, std::size_t LegacyAlign>
   struct argument_proxy<T*, LegacyAlign> {
      static_assert(LegacyAlign % alignof(T) == 0, "Specified alignment must be at least alignment of T");
      static_assert(std::is_trivially_copyable_v<T>, "argument_proxy requires a trivially copyable type");
      inline constexpr argument_proxy(void* ptr) : original_ptr(ptr) {
         if (!LegacyAlign || reinterpret_cast<std::uintptr_t>(original_ptr) % LegacyAlign != 0) {
            copy.emplace();
            memcpy( std::addressof(*copy), original_ptr, sizeof(T) );
         }
      }
      inline constexpr argument_proxy(const argument_proxy&) = delete;
      inline constexpr argument_proxy(argument_proxy&& other) : original_ptr(other.original_ptr), copy(other.copy) {
         other.copy.reset();
         other.original_ptr = nullptr;
      }
      inline ~argument_proxy() {
         if constexpr (!std::is_const_v<T>)
            if (copy)
               memcpy( original_ptr, std::addressof(*copy), sizeof(T) );
      }
      constexpr operator T*() { return get(); }
      constexpr operator const T*() const { return get(); }

      constexpr T* get() { return const_cast<T*>(const_cast<const argument_proxy*>(this)->get()); }
      constexpr const T* get() const {
         if (copy)
            return std::addressof(*copy);
         else
            return static_cast<const T*>(original_ptr);
      }
      constexpr T* operator->() { return get(); }
      constexpr const T* operator->() const { return get(); }

      static constexpr bool is_legacy() { return LegacyAlign != 0; }
      using dependent_type = T;

      void* original_ptr;
      std::optional<std::remove_cv_t<T>> copy;
   };

   template <typename T, std::size_t LegacyAlign>
   struct argument_proxy<span<T>, LegacyAlign> : span<T> {
      static_assert(LegacyAlign % alignof(T) == 0, "Specified alignment must be at least alignment of T");
      static_assert(std::is_trivially_copyable_v<T>, "argument_proxy requires a trivially copyable type");
      inline constexpr bool is_aligned(void* ptr) { return reinterpret_cast<std::uintptr_t>(original_ptr) % LegacyAlign == 0; }
      inline constexpr argument_proxy(void* ptr, uint32_t size)
         : original_ptr(ptr),
           copy( is_aligned(ptr) ? nullptr : new std::remove_cv_t<T>[size] ) {
         *static_cast<span<T>*>(this) = span<T>( copy ? copy.get() : static_cast<T*>(ptr), size );
         if (copy)
            memcpy( copy.get(), original_ptr, this->size_bytes() );
      }
      inline constexpr argument_proxy(const argument_proxy&) = delete;
      inline constexpr argument_proxy(argument_proxy&&) = default;
      inline ~argument_proxy() {
         if constexpr (!std::is_const_v<T>)
            if (copy)
               memcpy( original_ptr, copy.get(), this->size_bytes() );
      }
      static constexpr bool is_legacy() { return LegacyAlign != 0; }

      void* original_ptr;
      std::unique_ptr<std::remove_cv_t<T>[]> copy = nullptr;
      using dependent_type = T;
   };

   namespace detail {
      template <typename T>
      constexpr inline std::true_type is_argument_proxy_type(argument_proxy<T>);
      template <typename T, std::size_t LA>
      constexpr inline std::true_type is_argument_proxy_type(argument_proxy<T, LA>);
      template <typename T>
      constexpr inline std::false_type is_argument_proxy_type(T);
   }

   template <typename T>
   constexpr inline static bool is_argument_proxy_type_v = std::is_same_v<decltype(detail::is_argument_proxy_type(std::declval<T>())), std::true_type>;

   namespace detail {
      template <typename T>
      constexpr auto get_dependent_type() -> std::enable_if_t<is_argument_proxy_type_v<T>, typename T::dependent_type>;
      template <typename T>
      constexpr auto get_dependent_type() -> std::enable_if_t<!is_argument_proxy_type_v<T>, T>;
      // Help OSX clang
      template <typename U, std::size_t A>
      auto get_dependent_type(argument_proxy<U, A>) -> U;
      template <typename U>
      auto get_dependent_type(argument_proxy<U, 0>) -> U;
      template <typename T, std::size_t LA>
      constexpr inline std::integral_constant<bool, LA != 0> is_legacy(argument_proxy<T, LA>);
      template <typename T>
      constexpr inline std::false_type is_legacy(T);
   } // ns eosio::vm::detail

   template <typename T>
   using argument_proxy_dependent_type_t = decltype(detail::get_dependent_type<T>());

   template <typename T>
   constexpr inline bool is_argument_proxy_legacy_v = std::is_same_v<decltype(detail::is_legacy(std::declval<T>())), std::true_type>;

}} // ns eosio::vm
