#include <eosio/wasm_backend/execution_engine.hpp>
#include <eosio/wasm_backend/opcodes.hpp>

namespace eosio { namespace wasm_backend {
   void execution_engine::execute() {
      //dispatch_table[*at]({&code, &at});
      call( apply_index ); 
   }
   
   void execution_engine::call( size_t index ) { 

      if ( index >= mod->imports.size() ) {
         wasm_code_ptr wcp( mod->code.at(index).code.raw(), mod->code.at(index).code.size() );
         call(wcp, mod->code.at(index).code.size());
      }
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
      opcode expect = opcode::nop;
      switch ( *code++ ) {
         // CONTROL FLOW OPERATORS
         case opcode::unreachable:
            //FC_THROW_EXCEPTION(wasm_unreachable_exception, "unreachable opcode"); 
            break;
         case opcode::nop:
            eval<opcode::nop>( code ); break;
         case opcode::end:
            eval<opcode::end>( code ); break;
         case opcode::return_:
            eval<opcode::return_>( code ); break;
         case opcode::block:
            eval<opcode::block>( code ); break;
         case opcode::loop:
            eval<opcode::loop>( code ); break;
         case opcode::if_:
            eval<opcode::if_>( code ); break;
         case opcode::else_:
            eval<opcode::else_>( code ); break;
         case opcode::br:
            eval<opcode::br>( code ); break;
         case opcode::br_if:
            eval<opcode::br_if>( code ); break;
         case opcode::br_table:
            eval<opcode::br_table>( code ); break;
         case opcode::call:
            eval<opcode::call>( code ); break;
         case opcode::call_indirect:
            eval<opcode::call_indirect>( code ); break;
         case opcode::drop:
            eval<opcode::drop>( code ); break;
         case opcode::select:
            eval<opcode::select>( code ); break;
         case opcode::get_local:
            eval<opcode::get_local>( code ); break;
         case opcode::set_local:
            eval<opcode::set_local>( code ); break;
         case opcode::tee_local:
            eval<opcode::tee_local>( code ); break;
         case opcode::get_global:
            eval<opcode::get_global>( code ); break;
         case opcode::set_global:
            eval<opcode::set_global>( code ); break;

         default:
            break;
      }
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
      dispatch_table[opcode::block] = [](params&& p) {
      }; 
      dispatch_table[opcode::call] = [](params&& p) {
      };
   }
   
}} // namespace eosio::wasm_backend
