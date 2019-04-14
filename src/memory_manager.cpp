#include <eosio/wasm_backend/memory_manager.hpp>

namespace eosio { namespace wasm_backend {
   std::unique_ptr<memory_manager> memory_manager::instance = nullptr;
}} // namespace eosio::wasm_backend
