#include <eosio/wasm_backend/execution_engine.hpp>
#include <eosio/wasm_backend/opcodes.hpp>

namespace eosio { namespace wasm_backend {
   void execution_engine::execute() {
      //dispatch_table[*at]({&code, &at});
      call( apply_index ); 
   }
   
   void execution_engine::call( size_t index ) { 
#if 0
      if ( index >= mod->imports.size() ) {
         wasm_code_ptr wcp( mod->code.at(index).code.raw(), mod->code.at(index).code.size() );
         call(wcp, mod->code.at(index).code.size());
      }
#endif
      /*
      else {
         call_import(
      }
      */
   }

   void execution_engine::call( wasm_code_ptr& code, size_t size ) {
      opcode_utils ou;
      for (size_t i=0; i < size; i++) {
         eval( code );
      }
   }
   
   void execution_engine::eval( wasm_code_ptr& code ) {
      switch ( *code++ ) {
         // CONTROL FLOW OPERATORS
         case opcodes::unreachable:
            //FC_THROW_EXCEPTION(wasm_unreachable_exception, "unreachable opcodes"); 
            break;
         case opcodes::nop:
            eval<opcodes::nop>( code ); break;
         case opcodes::end:
            eval<opcodes::end>( code ); break;
         case opcodes::return_:
            eval<opcodes::return_>( code ); break;
         case opcodes::block:
            eval<opcodes::block>( code ); break;
         case opcodes::loop:
            eval<opcodes::loop>( code ); break;
         case opcodes::if_:
            eval<opcodes::if_>( code ); break;
         case opcodes::else_:
            eval<opcodes::else_>( code ); break;
         case opcodes::br:
            eval<opcodes::br>( code ); break;
         case opcodes::br_if:
            eval<opcodes::br_if>( code ); break;
         case opcodes::br_table:
            eval<opcodes::br_table>( code ); break;
         case opcodes::call:
            eval<opcodes::call>( code ); break;
         case opcodes::call_indirect:
            eval<opcodes::call_indirect>( code ); break;
         case opcodes::drop:
            eval<opcodes::drop>( code ); break;
         case opcodes::select:
            eval<opcodes::select>( code ); break;
         case opcodes::get_local:
            eval<opcodes::get_local>( code ); break;
         case opcodes::set_local:
            eval<opcodes::set_local>( code ); break;
         case opcodes::tee_local:
            eval<opcodes::tee_local>( code ); break;
         case opcodes::get_global:
            eval<opcodes::get_global>( code ); break;
         case opcodes::set_global:
            eval<opcodes::set_global>( code ); break;

         default:
            break;
      }
   }

   void execution_engine::populate_dispatch_table() {
      dispatch_table.reserve(257);
      
      auto error_func = [](params&&) {
        FC_THROW_EXCEPTION(wasm_illegal_opcode_exception, "illegal opcodes"); 
      };

      for (int i=0; i < 257; i++)
        dispatch_table[i] = error_func;

      // fill in for each opcodes, sorry quite large function
      dispatch_table[opcodes::unreachable] = [](params&&) { 
         FC_THROW_EXCEPTION(wasm_unreachable_exception, "unreachable opcodes"); 
      };

      dispatch_table[opcodes::nop] = [](params&&) { 
         std::cout << "NOP\n";
      };
      dispatch_table[opcodes::block] = [](params&& p) {
      }; 
      dispatch_table[opcodes::call] = [](params&& p) {
      };
   }
   
}} // namespace eosio::wasm_backend
