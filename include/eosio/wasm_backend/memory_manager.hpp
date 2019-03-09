#pragma once

#include <memory>
#include <eosio/wasm_backend/allocator.hpp>

namespace eosio { namespace wasm_backend {
   class memory_manager { 
      public:
         enum types {
            native,
            wasm,
            stack64
         };
         
         static void set_memory_limits( uint64_t native ) {
            instance.reset(new memory_manager(native));
         }

         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == native, native_allocator&> { 
            EOS_WB_ASSERT( instance.get() != nullptr, wasm_memory_exception, "must set memory limits first" );
            return instance->_nalloc; 
         }

         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == wasm, wasm_allocator&> { 
            EOS_WB_ASSERT( instance.get() != nullptr, wasm_memory_exception, "must set memory limits first" );
            return instance->_walloc; 
         }

         void reset() {
            _nalloc.reset();
            _walloc.reset();
         }

      private:
      //static constexpr size_t stack64_size = 8000;
         memory_manager( uint64_t native_size) :
            _nalloc(native_size),
            _walloc() {
         }
         static std::unique_ptr<memory_manager> instance;
         native_allocator _nalloc;
         wasm_allocator   _walloc;
   };
}} // namespace eosio::wasm_backend