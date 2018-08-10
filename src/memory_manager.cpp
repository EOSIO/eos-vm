#include <eosio/wasm_backend/memory_manager.hpp>

namespace eosio { namespace wasm_backend {
   native_allocator* memory_manager::_nalloc_ptr = nullptr;
   simple_allocator* memory_manager::_lmalloc_ptr = nullptr;
}} // namespace eosio::wasm_backend
