#pragma once

#include <string>
#include <utility>
#include <initializer_list>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/memory_manager.hpp>

namespace eosio { namespace wasm_backend {
   
   template <typename T, typename Owner> 
   class managed_vector {
      public:
         managed_vector(Owner& owner, size_t size=0) :
            _size(size),
            _owner(&owner),
            _data(owner.get_allocator().template alloc<T>( _size )) {
         }

         inline void resize( size_t size ) {
            if (size > _size) {
               T* old_data = _data;
               _data = _owner->get_allocator().template alloc<T>( size );
               memcpy(_data, old_data, _size);
            }
            _size = size;
         }

         inline void push_back( const T& val ) {
            EOS_WB_ASSERT( _index < _size, wasm_vector_oob_exception, "vector write out of bounds" );
            _data[_index++] = val;
         }
         inline void emplace_back( T&& val ) {
            EOS_WB_ASSERT( _index < _size, wasm_vector_oob_exception, "vector write out of bounds" );
            _data[_index++] = std::move(val);
         }
         inline void back() {
            return _data[_index];
         }
         inline void pop_back() {
            EOS_WB_ASSERT( _index >= 0, wasm_vector_oob_exception, "vector pop out of bounds" );
         }
         inline T& at( size_t i ) const {
            EOS_WB_ASSERT( i < _size, wasm_vector_oob_exception, "vector read out of bounds" );
            return _data[i];
         }
         inline T& begin()const {
            return _data[0];
         }
         inline T& end()const {
            static T _end;
            return _end;
         }
         inline T& operator[] (size_t i) const { return at(i); }
         inline T* raw() const { return _data; }
         inline size_t size() const { return _size; }
         inline void set( T* data, size_t size ) { _size = size; _data = data; _index = size-1; }

      private:
         size_t _size  = 0;
         Owner* _owner = nullptr;
         T*     _data  = nullptr;
         size_t _index = 0;
   };
}} // namespace eosio::wasm_backend
