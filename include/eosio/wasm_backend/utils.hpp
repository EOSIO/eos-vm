#pragma once

#include <eosio/wasm_backend/exceptions.hpp>

namespace eosio { namespace wasm_backend {
   template <typename T>
   struct guarded_ptr {
      T* raw_ptr;
      T* orig_ptr;
      T* bnds;
      guarded_ptr( T* rp, size_t bnds ) : raw_ptr(rp), orig_ptr(rp), bnds(rp + bnds) {}

      void set(T* rp, size_t bnds) {
         raw_ptr = rp;
         orig_ptr = rp;
         bnds = orig_ptr+bnds;
      }

      inline guarded_ptr& operator+=(size_t i) {
         EOS_WB_ASSERT(raw_ptr + i <= bnds, guarded_ptr_exception, "overbounding pointer");
         raw_ptr += i;
         return *this;
      }

      inline guarded_ptr& operator++() {
         EOS_WB_ASSERT(raw_ptr + 1 <= bnds, guarded_ptr_exception, "overbounding pointer");
         raw_ptr += 1;
         return *this;
      }

      inline guarded_ptr operator++(int) {
         guarded_ptr tmp = *this;
         ++*this;
         return tmp;
      }

      inline guarded_ptr operator+(size_t i) const {
         guarded_ptr tmp = *this;
         tmp += i;
         return tmp; 
      }
      
      inline T& operator* () {
         //std::cout << "* " << (uint64_t)raw_ptr << " OP " << (uint64_t)orig_ptr << " BOUNDS " << (uint64_t)bnds << "\n";
         return *raw_ptr;
      }
      
      inline T* operator-> () {
         return raw_ptr;
      }
      
      inline T& operator= (const guarded_ptr<T>& ptr) {
         raw_ptr = ptr.raw_ptr;
         orig_ptr = ptr.orig_ptr;
         bnds     = ptr.bnds;
         return *this;
      }

      inline T* raw() {
         return raw_ptr;
      }
      
      inline size_t offset() {
         return raw_ptr - orig_ptr;
      }
      
      inline void add_bounds(size_t n) {
         bnds += n;
      } 
      
      inline void fit_bounds() {
         bnds = raw_ptr;
      }

      inline size_t bounds() {
         return bnds - orig_ptr;
      }

      inline T at(size_t index) const {
         //std::cout << "at " << (uint64_t)raw_ptr << " OP " << (uint64_t)orig_ptr << " BOUNDS " << (uint64_t)bnds << " INDEX " << index << "\n";
         EOS_WB_ASSERT(orig_ptr + index <= bnds, guarded_ptr_exception, "accessing out of bounds");
         return raw_ptr[index];
      }
      
      inline T at() const {
         return *raw_ptr;
      }

      inline T operator[](size_t index) const {
         //std::cout << "[] " << (uint64_t)raw_ptr << " OP " << (uint64_t)orig_ptr << " BOUNDS " << (uint64_t)bnds << " INDEX " << index << "\n";
         EOS_WB_ASSERT(orig_ptr + index <= bnds, guarded_ptr_exception, "accessing out of bounds");
         return raw_ptr[index];
      }
   };
}} // namespace eosio::wasm_backend
