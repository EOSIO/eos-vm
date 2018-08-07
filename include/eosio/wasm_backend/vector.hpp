#pragma once

#include <utility>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/allocator.hpp>

namespace eosio { namespace wasm_backend {
   
   template <typename T, typename Owner> 
   class vector {
      public:
         vector( Owner& owner, size_t size=0 ) : _owner(owner), _size(size) {
            _data = _owner.get_allocator().template alloc<T>( _size );
         }
         void resize( size_t size ) {
            _size = size;
            _data = _owner.get_allocator().template alloc<T>( _size );
         }
         void push_back( const T& val ) {
            EOS_WB_ASSERT( _index + 1  <= _size, wasm_vector_oob_exception, "vector write out of bounds" );
            _data[_index++] = val;
         }
         void emplace_back( T&& val ) {
            EOS_WB_ASSERT( _index + 1 <= _size, wasm_vector_oob_exception, "vector write out of bounds" );
            _data[_index++] = std::move(val);
         }
         T& at( size_t i ) {
            EOS_WB_ASSERT( i < _size, wasm_vector_oob_exception, "vector read out of bounds" );
            return _data[i];
         }
         T* raw() {
            return _data;
         }
         size_t size() {
            return _size;
         }
      private:
         Owner& _owner;
         size_t _size;
         T*     _data;
         size_t _index;
   };
}} // namespace eosio::wasm_backend
