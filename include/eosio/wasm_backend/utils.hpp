#pragma once

#include <eosio/wasm_backend/exceptions.hpp>

namespace eosio { namespace wasm_backend {
   template <typename T>
   struct guarded_ptr {
      T* raw_ptr;
      T* orig_ptr;
      size_t bounds;
      guarded_ptr( T* rp, size_t bnds ) : raw_ptr(rp), orig_ptr(rp), bounds(bnds) {}
      void set(T* rp) {
         raw_ptr = rp;
         orig_ptr = rp;
      }

      inline guarded_ptr& operator++() {
         raw_ptr++;
         return *this;
      }

      inline guarded_ptr operator++(int) {
         guarded_ptr tmp = *this;
         ++*this;
         return tmp;
      }

      inline T& operator* () {
         return *raw_ptr;
      }
      
      inline T* operator-> () {
         return raw_ptr;
      }
   };
}} // namespace eosio::wasm_backend
