#pragma once

#include <utility>
#include <eosio/wasm_backend/exceptions.hpp>

namespace eosio { namespace wasm_backend {
   
   class memory_manager; 
   template <typename T, typename MemoryManager=memory_manager> 
   class managed_vector {
      public:
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
         inline MemoryManager& get_manager() const { return *_owner; }
         friend class managed_memory;
      protected:
         managed_vector( MemoryManager* owner, size_t size=0 ) : _owner(owner), _size(size) {
            _data = _owner->get_allocator().template alloc<T>( _size );
         }
      private:
         MemoryManager* _owner;
         size_t _size;
         T*     _data;
         size_t _index;
   };
}} // namespace eosio::wasm_backend
