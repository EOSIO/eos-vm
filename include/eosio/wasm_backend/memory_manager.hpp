#pragma once

#include <memory>
#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/vector.hpp>

namespace eosio { namespace wasm_backend {
   class memory_manager { 
      public:
         enum types {
            native,
            linear_memory
         };

         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == native, native_allocator&> { return *_nalloc_ptr; }
         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == linear_memory, simple_allocator&> { return *_lmalloc_ptr; }
      private:
         memory_manager( uint64_t native_size, uint64_t linmem_size ) : 
            _nalloc(native_size+linmem_size),
            _lmalloc(_nalloc.alloc<uint8_t>(linmem_size), linmem_size) { 
            _nalloc_ptr = &_nalloc;
            _lmalloc_ptr = &_lmalloc;
         }

         native_allocator _nalloc;
         static native_allocator* _nalloc_ptr; 
         simple_allocator _lmalloc;
         static simple_allocator* _lmalloc_ptr; 
   };
}} // namespace eosio::wasm_backend
