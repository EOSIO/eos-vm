#pragma once

#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/wasm_stack.hpp>

namespace eosio { namespace wasm_backend {
      template <char... Str>
      struct host_function_name {
        static constexpr const char value[] = {Str...};
      };

      template <typename T, T... Str>
      static constexpr host_function_name<Str...> operator ""_hfn() {
        constexpr auto hfn = host_function_name<Str...>{};
        return hfn;
      }

      template <typename C, auto C::*MP, typename Name>
      struct registered_member_function {
         static constexpr auto function = MP;
         static constexpr auto name = Name{};
         using name_t = Name;
         static constexpr bool is_member = true; 
      };

      template <auto F, typename Name>
      struct registered_function {
        static constexpr auto function = F;
        static constexpr auto name = Name{};
        using name_t = Name;
        static constexpr bool is_member = false; 
      };
      
      template <typename... Registered_Funcs>
      struct registered_host_functions {

        template <size_t Index, typename RF, typename... _RFs>
        static void _resolve( module& mod ) {
          size_t name_size = sizeof(RF::typename name_t::value);
          if (Index >= mod.imports.size())
            return;
          else {
            bool found_import = false;
            for (int i=0; i < mod.imports.size(); i++) {
              if (mod.imports[i].kind != external_kind::Function)
                continue;
              if (mod.imports[i].field_len == name_size) {
                if (memcmp(mod.imports[i].field_str, RF::typename name_t::value, name_size) == 0) {
                  mod.import_functions[i] = Index;
                }
              }
            }
          }
        }

        template <typename RF, typename... _RFs>
        static void resolve( module& mod ) {
          _resolve<0, RF, RFs...>(mod);
        }

        template <size_t N>
        static constexpr void _call(uint32_t index) {
          if constexpr(index == N)
            std::invoke(std::get<N>(registered));
          else
            _call<N+1>(index);
        }

        static constexpr void call(uint32_t index) {
           call<0>(index);
        }
        static constexpr const std::tuple<Registered_Funcs...> registered;
      };

      class execution_context {
         public:
            execution_context(module& m) : _mod(m) {}
            template <typename F>
            inline void register_host_function( F&& host_func ) {
            }
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
            stack_elem invoke(uint32_t func_index, const func_type& ftype) {
              //if (_mod.
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
