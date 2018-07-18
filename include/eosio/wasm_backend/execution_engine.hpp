#pragma once

#include <vector>

#include <eosio/wasm_backend/types.hpp>

namespace eosio { namespace wasm_backend {
   class execution_engine {
      public:
         execution_engine() {
            populate_dispatch_table();
         }
         void execute( wasm_code& code, wasm_code_iterator& at );
         void call( size_t index );
      private:
         struct params {
            wasm_code* code;
            const wasm_code_iterator* at;
         };

         typedef void (*op_cb)(params&& p);
         std::vector<op_cb>      dispatch_table;
         std::vector<function>   function_table; 
         void populate_dispatch_table();
   };
}} // namespace eosio::wasm_backend
