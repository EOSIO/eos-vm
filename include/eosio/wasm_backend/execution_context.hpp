#pragma once

#include <functional>
#include <eosio/wasm_backend/host_function.hpp>
#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/wasm_stack.hpp>

namespace eosio { namespace wasm_backend {
      template <char... Str>
      struct host_function_name {
         static constexpr const char value[] = {Str...};
         static constexpr size_t len = sizeof...(Str); 
         static constexpr bool is_same(const char* nm, size_t l) {
            if (len == l) {
               bool is_not_same = false;
               for (int i=0; i < len; i++) {
                  is_not_same |= nm[i] != value[i];
               } 
               return !is_not_same;
            }
            return false;
         }
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

         template <size_t Index, typename RF, typename... RFs>
         static void _resolve( module& mod ) {
            size_t name_size = RF::name_t::len;
            if (Index >= mod.import_functions.size())
               return;
            else {
               bool found_import = false;
               for (int i=0; i < mod.imports.size(); i++) {
                  if (mod.imports[i].kind != external_kind::Function)
                     continue;
                  if (RF::name_t::is_same((const char*)mod.imports[i].field_str.raw(), mod.imports[i].field_len)) {
                     mod.import_functions[i] = Index;
                     found_import = true;
                     std::cout << "Found it " << RF::name_t::value << "\n";
                  }
               }
               //EOS_WB_ASSERT(found_import, wasm_interpreter_exception, "unresolved import");
               if constexpr (sizeof...(RFs) >= 1)
                  _resolve<Index+1, RFs...>(mod);
            }
         }

         static void resolve( module& mod ) {
            mod.import_functions.resize(mod.get_imported_functions_size());
            _resolve<0, Registered_Funcs...>(mod);
         }

         template <size_t N>
         static constexpr void _call(uint32_t index) {
            if constexpr(index == N)
               std::invoke(std::get<N>(registered));
            else
               _call<N+1>(index);
         }
         static constexpr const std::tuple<Registered_Funcs...> registered;
      };

      template <typename Visitor>
      class execution_context {
         //using registered_hosts = registered_host_functions<Registered_Funcs...>;
         public:
            execution_context(module& m) :
              _mod(m), _alloc(memory_manager::get_allocator<memory_manager::types::wasm>()) {
               _mod.import_functions.resize(_mod.get_imported_functions_size());
               _mod.function_sizes.resize(_mod.get_functions_total());
               const size_t import_size = _mod.get_imported_functions_size();
               uint32_t total_so_far = 0;
               for (int i=_mod.get_imported_functions_size(); i < _mod.function_sizes.size(); i++) {
                  _mod.function_sizes[i] = total_so_far;
                  total_so_far += _mod.code[i-import_size].code.size();
               }
               //registered_hosts::resolve(mod);
               _linear_memory = _alloc.alloc<uint8_t>(1); // allocate an initial wasm page
            }
            template <auto... Funcs>
            inline void set_host_functions() {
               _host_functions = {(void*)Funcs...}; 
            }
            inline void _call() { call(); }
            inline module& get_module() { return _mod; }
            inline void push_label( const stack_elem& el ) { _cs.push(el); }
            inline void push_operand( const stack_elem& el ) { _os.push(el); }
            inline stack_elem get_operand( uint32_t index )const { return _os.get(index); }
            inline void set_operand( uint32_t index, const stack_elem& el ) { _os.set(index, el); }
            inline void push_call( const stack_elem& el ) { _as.push(el); }
            inline stack_elem pop_call() { return _as.pop(); }
            inline stack_elem pop_label() { return _cs.pop(); }
            inline stack_elem pop_operand() { return _os.pop(); }
            inline stack_elem& peek_operand() { return _os.peek(); }
            inline stack_elem get_global(uint32_t index) {
               EOS_WB_ASSERT( index < _mod.globals.size(), wasm_interpreter_exception, "global index out of range" );
               const auto& gl = _mod.globals[index];
               switch (gl.type.content_type) {
                  case types::i32: 
                     {
                        return i32_const_t{*(uint32_t*)&gl.init.value.i32}; 
                     }
                  case types::i64: 
                     {
                        return i64_const_t{*(uint64_t*)&gl.init.value.i64}; 
                     }
                  case types::f32: 
                     {
                        return f32_const_t{gl.init.value.f32}; 
                     }
                  case types::f64: 
                     {
                        return f64_const_t{gl.init.value.f64}; 
                     }
                  default:
                     throw wasm_interpreter_exception{"invalid global type"};
               }
            }
            inline void set_global(uint32_t index, const stack_elem& el) {
               EOS_WB_ASSERT( index < _mod.globals.size(), wasm_interpreter_exception, "global index out of range" );
               auto& gl = _mod.globals[index];
               EOS_WB_ASSERT( gl.type.mutability, wasm_interpreter_exception, "global is not mutable" );
               std::visit(overloaded {
                     [&](const i32_const_t& i){
                        EOS_WB_ASSERT( gl.type.content_type == types::i32, wasm_interpreter_exception, "expected i32 global type");
                        gl.init.value.i32 = i.data; 
                     }, [&](const i64_const_t& i){
                        EOS_WB_ASSERT( gl.type.content_type == types::i64, wasm_interpreter_exception, "expected i64 global type");
                        gl.init.value.i64 = i.data; 
                     }, [&](const f32_const_t& f){
                        EOS_WB_ASSERT( gl.type.content_type == types::f32, wasm_interpreter_exception, "expected f32 global type");
                        gl.init.value.f32 = f.data; 
                     }, [&](const f64_const_t& f){
                        EOS_WB_ASSERT( gl.type.content_type == types::f64, wasm_interpreter_exception, "expected f64 global type");
                        gl.init.value.f64 = f.data; 
                     }, [](auto) {
                        throw wasm_interpreter_exception{"invalid global type"};
                     }
                  }, el);
            }

            inline stack_elem invoke( uint32_t index, const func_type& ftype ) {
               //return invoke<0>( index, ftype );
            }
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

            inline uint32_t get_pc()const { return _pc; }
            inline void set_pc( uint32_t pc ) { _pc = pc; }
            inline void inc_pc() { _pc++; }
            inline void exit()const { _executing = false; }
            inline bool executing()const { _executing; }
            inline void execute(const std::string_view func) {
               uint32_t func_index = _mod.get_exported_function(func);
               EOS_WB_ASSERT(func_index < std::numeric_limits<uint32_t>::max(), wasm_interpreter_exception, "cannot execute function, function not found");
               _current_function = func_index;
               _code_index       = func_index - _mod.import_functions.size();
               _current_offset   = _mod.function_sizes[_current_function];
               _pc               = _current_offset;
               _exit_pc          = _current_offset + _mod.code[_current_function-_mod.import_functions.size()].code.size();
               _executing        = true;
               setup_call(func_index);
               execute();
            }

            inline void jump( uint32_t label ) { 
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
            template <size_t N>
            stack_elem invoke( uint32_t index, const func_type& ftype ) {
            }
            inline void setup_call(uint32_t index) {
               const auto& fn_ty = _mod.get_function_type(index);
               const auto& fn = _mod.code[index-_mod.get_imported_functions_size()];
               for (int i=0; i < fn_ty.param_count; i++) {
                  switch (fn_ty.param_types[i]) {
                     case types::i32:
                        {
                           push_operand(i32_const_t{0});
                           break;
                        }
                     case types::i64:
                        {
                           push_operand(i64_const_t{0});
                           break;
                        }
                     case types::f32:
                        {
                           push_operand(f32_const_t{0});
                           break;
                        }
                     case types::f64:
                        {
                           push_operand(f64_const_t{0});
                           break;
                        }
                     default:
                        throw wasm_interpreter_exception{"invalid function param type"};
                  }
               }
               for (int i=0; i < fn.local_count; i++) {
                  for (int j=0; j < fn.locals.size(); j++) {
                     switch (fn.locals[i].type) {
                        case types::i32: 
                           {
                              push_operand(i32_const_t{0});
                           }
                        case types::i64: 
                           {
                              push_operand(i64_const_t{0});
                           }
                        case types::f32: 
                           {
                              push_operand(f32_const_t{0});
                           }
                        case types::f64: 
                           {
                              push_operand(f64_const_t{0});
                           }
                        default:
                           throw wasm_interpreter_exception{"invalid function param type"};
                     }
                  }
               }
            }

            void execute() {
               do {
                  uint32_t offset = _pc - _current_offset;
                  std::visit(_visitor, _mod.code[_code_index].code[offset]);
                  std::cout << _visitor.dbg_output.str() << "\n";
                  _visitor.dbg_output.str("");
                  if (_pc == _exit_pc)
                     _executing = false;
               } while (_executing);
            }

            void call() {
               const void* hf = _host_functions[0];
               asm( "mov rax, %P0\n\t"
                    "call rax\n\t"
                    : 
                    : "m" (hf)
                    );
            }
            uint32_t      _pc               = 0;
            uint32_t      _exit_pc          = 0;
            uint32_t      _current_function = 0;
            uint32_t      _code_index       = 0;
            uint32_t      _current_offset   = 0;
            bool          _executing        = false;
            uint8_t*      _linear_memory    = nullptr;
            module&       _mod;
            std::array<void*, 256> _host_functions;
            wasm_allocator& _alloc;
            control_stack _cs;
            operand_stack _os;
            call_stack    _as;
            Visitor       _visitor{*this};
      };
}} // ns eosio::wasm_backend
