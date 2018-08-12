#pragma once

#include <utility>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/memory_manager.hpp>

namespace eosio { namespace wasm_backend {
   
   template <typename T, size_t Type> 
   class managed_vector {
      public:
         managed_vector(size_t size=0) : _size(size) {
            _data = memory_manager::get_allocator<Type>().template alloc<T>( _size );
         }
         inline void resize( size_t size ) {
            if (size > _size) {
               T* old_data = _data;
               _data = memory_manager::get_allocator<Type>().template alloc<T>( size );
               memcpy(_data, old_data, _size);
            }
            _size = size;
         }

         inline void push_back( const T& val ) {
            EOS_WB_ASSERT( _index  < _size, wasm_vector_oob_exception, "vector write out of bounds" );
            _data[_index++] = val;
         }

         inline void emplace_back( T&& val ) {
            EOS_WB_ASSERT( _index < _size, wasm_vector_oob_exception, "vector write out of bounds" );
            _data[_index++] = std::move(val);
         }
         inline T& at( size_t i ) const {
            EOS_WB_ASSERT( i < _size, wasm_vector_oob_exception, "vector read out of bounds" );
            return _data[i];
         }
         inline T& operator[] (size_t i) const { return at(i); }
         inline T* raw() const { return _data; }
         inline size_t size() const { return _size; }
      private:
         size_t _size = 0;
         T*     _data;
         size_t _index = 0;
   };
}} // namespace eosio::wasm_backend
