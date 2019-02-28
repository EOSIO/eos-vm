#pragma once

#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/wasm_stack.hpp>
#include <eosio/wasm_backend/host_function.hpp>

#define __BACKEND_GET_ARG(ARG, X, EXPECTED)  \
  std::visit( overloaded {                   \
    [&](const EXPECTED& v) {                 \
      ARG = v.data;                          \
    }, [&](auto) {                                                    \
      throw wasm_interpreter_exception{"invalid host function arg"};  \
    }                                                                 \
  }, X )

namespace eosio { namespace wasm_backend {

      template <typename Visitor>
      class execution_context {
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
               _linear_memory = _alloc.alloc<uint8_t>(1); // allocate an initial wasm page
            }
            template <typename... Registered_Funcs>
            inline void set_host_functions() {
               _host_functions = {(void*)Registered_Funcs::function...}; 
               _host_functions_index = sizeof...(Registered_Funcs);
            }
            template <typename Registered_Func>
            inline void add_host_function() {
            }
            inline void _call(uint32_t index, const func_type& ftype) { host_call(index, ftype); }
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
                     return i32_const_t{*(uint32_t*)&gl.init.value.i32}; 
                  case types::i64: 
                     return i64_const_t{*(uint64_t*)&gl.init.value.i64}; 
                  case types::f32: 
                     return f32_const_t{gl.init.value.f32}; 
                  case types::f64: 
                     return f64_const_t{gl.init.value.f64}; 
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

            void host_call(uint32_t index, const func_type& ftype) {
               static constexpr size_t calling_conv_arg_cnt = 6;
               static_assert(__x86_64__, "currently only supports x86_64");
               const void* func_ptr = _host_functions[index];
               uint64_t args[calling_conv_arg_cnt] = {0};

               for (int i=0; i < ftype.param_count && i < calling_conv_arg_cnt; i++) {
                  const auto& op = pop_operand();
                  switch (ftype.param_types[i]) {
                      case types::i32: 
                        {
                          __BACKEND_GET_ARG(args[i], op, i32_const_t);
                          break;
                        }
                      case types::i64: 
                        {
                          __BACKEND_GET_ARG(args[i], op, i64_const_t);
                          break;
                        }
                      case types::f32: 
                        {
                          __BACKEND_GET_ARG(args[i], op, f32_const_t);
                          break;
                        }
                      case types::f64: 
                        {
                          __BACKEND_GET_ARG(args[i], op, f64_const_t);
                          break;
                        }
                  }
               }

               if (ftype.param_count > calling_conv_arg_cnt) {
                   
               }

               switch (ftype.param_count) {
                  case 0:
                    asm( "callq *%0\n\t"
                        :
                        : "a"(func_ptr));
                    break;
                  case 1:
                    asm("movq %1, %%rdi\n\t"
                        "callq *%0\n\t"
                        :
                        : "a"(func_ptr), "g"(args[0]));
                    break;
                  case 2:
                    asm("movq %1, %%rdi\n\t"
                        "movq %2, %%rsi\n\t"
                        "callq *%0\n\t"
                        :
                        : "a"(func_ptr), "g"(args[0]), "g"(args[1]));
                    break;
                  case 3:
                    asm("movq %1, %%rdi\n\t"
                        "movq %2, %%rsi\n\t"
                        "movq %3, %%rdx\n\t"
                        "callq *%0\n\t"
                        :
                        : "a"(func_ptr), "g"(args[0]), "g"(args[1]), "g"(args[2]));
                    break;
                  case 4:
                    asm("movq %1, %%rdi\n\t"
                        "movq %2, %%rsi\n\t"
                        "movq %3, %%rdx\n\t"
                        "movq %4, %%rcx\n\t"
                        "callq *%0\n\t"
                        :
                        : "a"(func_ptr), "g"(args[0]), "g"(args[1]), "g"(args[2]), "g"(args[3]));
                    break;
                  case 5:
                    asm("movq %1, %%rdi\n\t"
                        "movq %2, %%rsi\n\t"
                        "movq %3, %%rdx\n\t"
                        "movq %4, %%rcx\n\t"
                        "movq %5, %%r8\n\t"
                        "callq *%0\n\t"
                        :
                        : "a"(func_ptr), "g"(args[0]), "g"(args[1]), "g"(args[2]), "g"(args[3]), "g"(args[4]));
                    break;
                  case 6:
                    asm("movq %1, %%rdi\n\t"
                        "movq %2, %%rsi\n\t"
                        "movq %3, %%rdx\n\t"
                        "movq %4, %%rcx\n\t"
                        "movq %5, %%r8\n\t"
                        "movq %6, %%r9\n\t"
                        "callq *%0\n\t"
                        :
                        : "a"(func_ptr), "g"(args[0]), "g"(args[1]), "g"(args[2]), "g"(args[3]), "g"(args[4]), "g"(args[5]));
                    break;
                  default:
                    asm("movq %1, %%rdi\n\t"
                        "movq %2, %%rsi\n\t"
                        "movq %3, %%rdx\n\t"
                        "movq %4, %%rcx\n\t"
                        "movq %5, %%r8\n\t"
                        "movq %6, %%r9\n\t"
                        "callq *%0\n\t"
                        :
                        : "a"(func_ptr), "g"(args[0]), "g"(args[1]), "g"(args[2]), "g"(args[3]), "g"(args[4]), "g"(args[5]));
               }
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
            size_t                 _host_functions_index = 0;
            wasm_allocator& _alloc;
            control_stack _cs;
            operand_stack _os;
            call_stack    _as;
            Visitor       _visitor{*this};
      };
}} // ns eosio::wasm_backend
