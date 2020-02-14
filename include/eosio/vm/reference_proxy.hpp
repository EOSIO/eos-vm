#pragma once

#include <eosio/vm/types.hpp>
#include <eosio/vm/span.hpp>

#include <memory>
namespace eosio { namespace vm {

   template <typename T, bool LegacyAlign=false>
   struct reference_proxy {
      reference_proxy(T& val) : original_ptr(std::addressof(val)) {
         if (reinterpret_cast<std::uintptr_t>(original_ptr) % alignof(T) != 0 || !LegacyAlign) {
            copy.reset(new std::remove_cv_t<T>());
            memcpy( copy.get(), original_ptr, sizeof(T) );
         }
      }
      reference_proxy(T* ptr) : original_ptr(ptr) {
         if (reinterpret_cast<std::uintptr_t>(original_ptr) % alignof(T) != 0 || !LegacyAlign) {
            copy.reset(new std::remove_cv_t<T>());
            memcpy( copy.get(), original_ptr, sizeof(T) );
         }
      }
      ~reference_proxy() {
         if constexpr (!std::is_const_v<T>)
            if (copy || !LegacyAlign)
               memcpy( original_ptr, copy.get(), sizeof(T) );
      }
      constexpr operator T*() const {
         if (copy || !LegacyAlign)
            return copy.get();
         else
            return original_ptr;
      }
      constexpr operator T&() const {
         if (copy || !LegacyAlign)
            return *copy.get();
         else
            return *original_ptr;
      }

      T* original_ptr;
      std::unique_ptr<std::remove_cv_t<T>> copy = nullptr;
   };

   template <typename T>
   struct reference_proxy<span<T>> {
      reference_proxy(T* ptr, uint32_t size) : original_ptr(ptr), size(size) {
         if (reinterpret_cast<std::uintptr_t>(original_ptr) % alignof(T) != 0) {
            copy.reset(new std::remove_cv_t<T>[size]);
            memcpy( copy.get(), original_ptr, sizeof(T) * size );
         }
      }
      ~reference_proxy() {
         if constexpr (!std::is_const_v<T>)
            if (copy)
               memcpy( original_ptr, copy.get(), sizeof(T) * size );
      }
      constexpr operator T*() const {
         if (copy)
            return copy.get();
         else
            return original_ptr;
      }
      constexpr operator span<T>() const {
         if (copy)
            return span<T>{ copy.get(), size };
         else
            return span<T>{ original_ptr, size };
      }

      T* original_ptr;
      std::size_t size;
      std::unique_ptr<std::remove_cv_t<T>[]> copy = nullptr;
   };
}} // ns eosio::vm
