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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
         if constexpr (!std::is_const_v<T>)
            if (copy)
               memcpy(original_ptr, std::addressof(*copy), sizeof(T));
#pragma GCC diagnostic pop
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
      using pointee_type = T;
      using proxy_type = T*;
      constexpr const void* get_original_pointer() const { return original_ptr; }

    private:
      void* original_ptr;
      std::optional<std::remove_cv_t<T>> copy;
   };

   template <typename T, std::size_t LegacyAlign>
   struct argument_proxy<span<T>, LegacyAlign> : span<T> {
      static_assert(LegacyAlign % alignof(T) == 0, "Specified alignment must be at least alignment of T");
      static_assert(std::is_trivially_copyable_v<T>, "argument_proxy requires a trivially copyable type");
      using pointee_type = T;
      using proxy_type = span<T>;
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
         if constexpr (!std::is_const_v<T>)
            if (copy)
               memcpy(original_ptr, copy.get(), this->size_bytes());
#pragma GCC diagnostic pop
      }
      static constexpr bool is_legacy() { return LegacyAlign != 0; }
      constexpr const void* get_original_pointer() const { return original_ptr; }

   private:
      inline static constexpr bool is_aligned(void* ptr) { return reinterpret_cast<std::uintptr_t>(ptr) % LegacyAlign == 0; }
      void* original_ptr;
      std::unique_ptr<std::remove_cv_t<T>[]> copy = nullptr;
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

}} // ns eosio::vm
