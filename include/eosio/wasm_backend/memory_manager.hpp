#pragma once

#include <memory>
#include <eosio/wasm_backend/allocator.hpp>

namespace eosio { namespace wasm_backend {
   class memory_manager { 
      public:
         enum types {
            native,
            linear_memory,
            stack64
         };
         
         static void set_memory_limits( uint64_t native, uint64_t linmem ) {
            instance.reset(new memory_manager(native, linmem));
         }

         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == native, native_allocator&> { 
            EOS_WB_ASSERT( instance != nullptr, wasm_memory_exception, "must set memory limits first" );
            return instance->_nalloc; 
         }
         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == linear_memory, simple_allocator&> { 
            EOS_WB_ASSERT( instance != nullptr, wasm_memory_exception, "must set memory limits first" );
            return instance->_lmalloc; 
         }
         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == stack64, stack64_allocator&> { 
            EOS_WB_ASSERT( instance != nullptr, wasm_memory_exception, "must set memory limits first" );
            return instance->_s64alloc; 
         }
        
      private:
         static constexpr size_t stack64_size = 8000;
         memory_manager( uint64_t native_size, 
                         uint64_t linmem_size ) : 
            _nalloc(native_size+linmem_size+stack64_size),
            _lmalloc(_nalloc.alloc<uint8_t>(linmem_size), linmem_size),
            _s64alloc(_nalloc.alloc<uint8_t>(stack64_size), stack64_size) {
         }
         static std::unique_ptr<memory_manager> instance;
         native_allocator _nalloc;
         simple_allocator _lmalloc;
         stack64_allocator _s64alloc;
   };
}} // namespace eosio::wasm_backend
