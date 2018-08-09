#pragma once

#include <utility>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/memory_owner.hpp>

namespace eosio { namespace wasm_backend {
   
   template <typename T> 
   class managed_vector {
      public:
         managed_vector( memory_owner* owner, size_t size=0 ) : _owner(owner), _size(size) {
            _data = _owner->get_allocator().template alloc<T>( _size );
         }
         inline void resize( size_t size ) {
            _size = size;
            _data = _owner->get_allocator().template alloc<T>( _size );
         }

         inline void push_back( const T& val ) {
            EOS_WB_ASSERT( _index + 1  <= _size, wasm_vector_oob_exception, "vector write out of bounds" );
            _data[_index++] = val;
         }

         inline void emplace_back( T&& val ) {
            EOS_WB_ASSERT( _index + 1 <= _size, wasm_vector_oob_exception, "vector write out of bounds" );
            _data[_index++] = std::move(val);
         }
         inline T& at( size_t i ) const {
            EOS_WB_ASSERT( i < _size, wasm_vector_oob_exception, "vector read out of bounds" );
            return _data[i];
         }
         inline T& operator[] (size_t i) const { return at(i); }
         inline T* raw() const { return _data; }
         inline size_t size() const { return _size; }
         inline void set_owner( memory_owner* owner ) { _owner = owner; }
         inline memory_owner& get_owner() const { return *_owner; }
         inline wasm_allocator& get_allocator() const { return _owner->get_allocator(); }
      private:
         memory_owner* _owner;
         size_t _size;
         T*     _data;
         size_t _index;
   };
}} // namespace eosio::wasm_backend
