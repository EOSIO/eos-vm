#pragma once

#include <memory>
#include <eosio/wasm_backend/allocator.hpp>

namespace eosio { namespace wasm_backend {
   class memory_manager { 
      public:
         enum types {
            bounded,
            growable,
            wasm
         };

         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == growable, growable_allocator&> { 
            EOS_WB_ASSERT( instance.get() != nullptr, wasm_memory_exception, "must set memory limits first" );
            return instance->_balloc; 
         }

         template <size_t Type>
         static auto get_allocator() -> std::enable_if_t<Type == wasm, wasm_allocator&> { 
            EOS_WB_ASSERT( instance.get() != nullptr, wasm_memory_exception, "must set memory limits first" );
            return instance->_walloc; 
         }

         void reset() {
            _balloc.reset();
            _walloc.reset();
         }

      private:
         memory_manager( uint64_t native_size) :
            _balloc(native_size),
            _walloc() {
         }
         static std::unique_ptr<memory_manager> instance;
         growable_allocator _balloc;
         wasm_allocator    _walloc;
   };
}} // namespace eosio::wasm_backend
