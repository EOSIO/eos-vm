#include <eosio/wasm_backend/execution_engine.hpp>
#include <eosio/wasm_backend/opcodes.hpp>

namespace eosio { namespace wasm_backend {
   void execution_engine::execute( wasm_code& code, wasm_code_iterator& at ) {
      dispatch_table[*at]({&code, &at});
   }
   
   void execution_engine::populate_dispatch_table() {
      dispatch_table.reserve(257);
      
      auto error_func = [](params&&) {
        FC_THROW_EXCEPTION(wasm_illegal_opcode_exception, "illegal opcode"); 
      };

      for (int i=0; i < 257; i++)
        dispatch_table[i] = error_func;

      // fill in for each opcode, sorry quite large function
      dispatch_table[opcode::unreachable] = [](params&&) { 
         FC_THROW_EXCEPTION(wasm_unreachable_exception, "unreachable opcode"); 
      };

      dispatch_table[opcode::nop] = [](params&&) { 
         std::cout << "NOP\n";
      };
      
      dispatch_table[opcode::call] = [](params&& p) {
         //call[
      };
   }
}} // namespace eosio::wasm_backend
