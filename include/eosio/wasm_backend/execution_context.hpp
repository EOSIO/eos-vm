#pragma once

#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/wasm_stack.hpp>

namespace eosio { namespace wasm_backend {
      class execution_context {
         public:
            execution_context(module& m) : _mod(m) {}
            inline module& get_module() { return _mod; }
            inline void push_label( const stack_elem& el ) { _cs.push(el); }
            inline void push_operand( const stack_elem& el ) { _os.push(el); }
            inline void push_call( const stack_elem& el ) { _as.push(el); }
            inline stack_elem pop_label() { return _cs.pop(); }
            inline stack_elem pop_operand() { return _os.pop(); }
            inline bool is_true( const stack_elem& el )  {
               bool ret_val = false;
               std::visit(overloaded {
                     [&](const i32_const_t& i32) {
                        ret_val = i32.data;
                     }, [&](auto) {
                           //throw wasm_invalid_element{"should be an i32 type"};
                     }
                  }, el);
               return ret_val;
            }
            inline stack_elem pop_call() { return _as.pop(); }
            inline uint32_t get_pc()const { return _pc; }
            inline void set_pc( uint32_t pc ) { _pc = pc; }
            inline void inc_pc() { _pc++; }
            inline void jump( uint32_t label ) { 
               std::cout << "JUMP " << label << '\n';
               std::cout << "control_stack " << (int)_cs.size() << std::endl;
               stack_elem el;
               for (int i=0; i < label; i++) {
                  // el = _cs.pop();
               }
               std::visit(overloaded {
                     [&](const block_t& bt) {
                        _pc = bt.pc;
                     }, [&](const loop_t& lt) {
                        _pc = lt.pc;
                     }, [&](const if__t& it) {
                        _pc = it.pc;
                     }, [&](auto) {
                           //throw wasm_invalid_element{"invalid element when popping control stack"};
                     }
                  }, el);
            }
         private:
            uint32_t      _pc = 0;
            module&       _mod;
            control_stack _cs;
            operand_stack _os;
            call_stack    _as;
      };
}} // ns eosio::wasm_backend
