#pragma once

#include <memory>
#include <eosio/wasm_backend/allocator.hpp>

namespace eosio { namespace wasm_backend {
   class memory_manager { 
      public:
         enum types {
            native,
            linear_memory
         };
         
         static void set_memory_limits( uint64_t native, uint64_t linmem ) {
            instance.reset(new memory_manager(native, linmem));
         }

         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == native, native_allocator&> { 
            if ( instance == nullptr )
               instance.reset(new memory_manager());
            return instance->_nalloc; 
         }
         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == linear_memory, simple_allocator&> { 
            if ( instance == nullptr )
               instance.reset(new memory_manager());
            return instance->_lmalloc; 
         }
         
      private:
         memory_manager( uint64_t native_size=default_native_memory_size, 
                         uint64_t linmem_size=default_linear_memory_size ) : 
            _nalloc(native_size+linmem_size),
            _lmalloc(_nalloc.alloc<uint8_t>(linmem_size), linmem_size) { 
         }
         static std::unique_ptr<memory_manager> instance;
         static constexpr uint64_t default_native_memory_size = 64*1024;
         static constexpr uint64_t default_linear_memory_size = 64*1024; 
         native_allocator _nalloc;
         simple_allocator _lmalloc;
   };
}} // namespace eosio::wasm_backend
