#pragma once

#include <optional>
#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/wasm_stack.hpp>
#include <eosio/wasm_backend/host_function.hpp>
#include <string>

namespace eosio { namespace wasm_backend {
      template <typename Backend>
      class execution_context {
         public:
            execution_context(Backend& backend, module<Backend>&& m, wasm_allocator& wa) :
              _mod(m),
              _alloc(wa),
              _cs(backend),
              _os(backend),
              _as(backend) {

              for (int i=0; i < _mod.exports.size(); i++)
              _mod.import_functions.resize(_mod.get_imported_functions_size());
              _mod.function_sizes.resize(_mod.get_functions_total());
              const size_t import_size = _mod.get_imported_functions_size();
              uint32_t total_so_far = 0;
              for (int i=_mod.get_imported_functions_size(); i < _mod.function_sizes.size(); i++) {
                _mod.function_sizes[i] = total_so_far;
                total_so_far += _mod.code[i-import_size].code.size();
              }
              _linear_memory = _alloc.get_base_ptr<uint8_t>(); // allocate an initial wasm page

              for (int i=0; i < _mod.data.size(); i++) {
                const auto& data_seg = _mod.data[i];
                //TODO validate only use memory idx 0 in parse
                memcpy((char*)_linear_memory+data_seg.offset.value.i64, data_seg.data.raw(), data_seg.size);
              }
            }

            template <auto Registered_Func>
            inline void add_host_function(const std::string& name) {
               _rhf.add<Registered_Func>(name);
            }
            inline void call(uint32_t index) {
                // TODO validate index is valid
               if (index < _mod.get_imported_functions_size()) {
                  // TODO validate only importing functions
                  const auto& ft = _mod.types[_mod.imports[index].type.func_t];
                  type_check(ft);
                  _rhf(*this, index);
                  inc_pc();
               } else {
                  const auto& ft = _mod.types[_mod.functions[index - _mod.get_imported_functions_size()]];
                  type_check(ft);
                  setup_locals(index);
                  push_call(index);
                  const uint32_t& pc = _mod.function_sizes[index];
                  set_pc( pc );
                  _current_offset = pc;
                  _code_index = index - _mod.get_imported_functions_size();
               }
            }
            inline void call(const std::string& f) {
               _rhf(*this, f);
            } 
            void print_stack() {
               std::cout << "STACK { ";
               for (int i=0; i < _os.size(); i++) {
                  std::cout << _os.get(i).index() << ", "; 
               }
               std::cout << " }\n";
            }
            inline module<Backend>& get_module() { return _mod; }
            inline uint8_t* linear_memory() { return _linear_memory; }
            inline uint32_t table_elem(uint32_t i) { return _mod.elements[0].offset.value.f64 + _mod.elements[0].elems[i]; }
            inline void push_label( const stack_elem& el ) { _cs.push(el); }
            inline uint16_t current_label_index()const { return _cs.current_index(); }
            inline void eat_labels(uint16_t index) { _cs.eat(index); }
            inline void push_operand( const stack_elem& el ) { _os.push(el); }
            inline stack_elem get_operand( uint32_t index )const { return _os.get(index); }
            inline void eat_operands(uint16_t index) { _os.eat(index); }
            inline void set_operand( uint32_t index, const stack_elem& el ) { _os.set(index, el); }
            inline uint16_t current_operands_index()const { return _os.current_index(); }
            inline size_t operands()const { return _os.size(); }
            inline void push_call( const stack_elem& el ) { _as.push(el); }
            inline stack_elem pop_call() { return _as.pop(); }
            inline void push_call(uint32_t index) {
               const auto& ftype  = _mod.types[_mod.functions[index-_mod.get_imported_functions_size()]];
               const auto& locals = _mod.code[index-_mod.get_imported_functions_size()].local_count;
               _as.push(activation_frame{_pc+1, _current_offset, _code_index, static_cast<uint16_t>(ftype.param_count + locals), ftype.return_type});
            }
            inline void apply_pop_call() {
               const auto& af = std::get<activation_frame>(_as.pop());
               _current_offset = af.offset;
               _pc             = af.pc;
               _code_index     = af.index;
               stack_elem el;
               if (af.ret_type != 0x00) {
                  el = pop_operand();
                  EOS_WB_ASSERT( is_a<i32_const_t>(el) && af.ret_type == types::i32 ||
                                 is_a<i64_const_t>(el) && af.ret_type == types::i64 ||
                                 is_a<f32_const_t>(el) && af.ret_type == types::f32 ||
                                 is_a<f64_const_t>(el) && af.ret_type == types::f64, wasm_interpreter_exception, "wrong return type" );
               }
               for (int i=0; i < af.size; i++)
                  pop_operand();
               if (af.ret_type != 0x00)
                  push_operand(el);
            }
            inline stack_elem pop_label() { return _cs.pop(); }
            inline stack_elem pop_operand() { return _os.pop(); }
            inline stack_elem& peek_operand(size_t i=0) { return _os.peek(i); }
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
                        gl.init.value.i32 = i.data.ui; 
                     }, [&](const i64_const_t& i){
                        EOS_WB_ASSERT( gl.type.content_type == types::i64, wasm_interpreter_exception, "expected i64 global type");
                        gl.init.value.i64 = i.data.ui; 
                     }, [&](const f32_const_t& f){
                        EOS_WB_ASSERT( gl.type.content_type == types::f32, wasm_interpreter_exception, "expected f32 global type");
                        gl.init.value.f32 = f.data.ui; 
                     }, [&](const f64_const_t& f){
                        EOS_WB_ASSERT( gl.type.content_type == types::f64, wasm_interpreter_exception, "expected f64 global type");
                        gl.init.value.f64 = f.data.ui; 
                     }, [](auto) {
                        throw wasm_interpreter_exception{"invalid global type"};
                     }
                  }, el);
            }

            inline bool is_true( const stack_elem& el )  {
               bool ret_val = false;
               std::visit(overloaded {
                     [&](const i32_const_t& i32) {
                        ret_val = i32.data.ui;
                     }, [&](auto) {
                        throw wasm_invalid_element{"should be an i32 type"};
                     }
                  }, el);
               return ret_val;
            }

            inline void type_check( const func_type<Backend>& ft ) {
              // TODO validate param_count is less than 256
              for (int i=0; i < ft.param_count; i++) {
                const auto& op = peek_operand(i);
                std::visit(overloaded {
                    [&](const i32_const_t&) {
                      EOS_WB_ASSERT(ft.param_types[i] == types::i32, wasm_interpreter_exception, "function param type mismatch");
                    }, [&](const f32_const_t&) {
                      EOS_WB_ASSERT(ft.param_types[i] == types::f32, wasm_interpreter_exception, "function param type mismatch");
                    }, [&](const i64_const_t&) {
                      EOS_WB_ASSERT(ft.param_types[i] == types::i64, wasm_interpreter_exception, "function param type mismatch");
                    }, [&](const f64_const_t&) {
                      EOS_WB_ASSERT(ft.param_types[i] == types::f64, wasm_interpreter_exception, "function param type mismatch");
                    }, [&](auto) {
                      throw wasm_interpreter_exception{"function param invalid type"};
                    }
                }, op);
              }
            }

            inline uint32_t get_pc()const { return _pc; }
            inline void set_pc( uint32_t pc ) { _pc = pc; }
            inline void set_relative_pc( uint32_t pc ) { _pc = _current_offset+pc; }
            inline void inc_pc() { _pc++; }
            inline void exit()const { _executing = false; }
            inline bool executing()const { _executing; }

            template <typename Visitor, typename... Args>
            inline std::optional<stack_elem> execute(Visitor&& visitor, const std::string_view func, Args... args) {
               uint32_t func_index = _mod.get_exported_function(func);
               EOS_WB_ASSERT(func_index < std::numeric_limits<uint32_t>::max(), wasm_interpreter_exception, "cannot execute function, function not found");
               _current_function = func_index;
               _code_index       = func_index - _mod.import_functions.size();
               _current_offset   = _mod.function_sizes[_current_function];
               _exit_pc          = _current_offset + _mod.code[_current_function-_mod.import_functions.size()].code.size()-1;
               _executing        = true;
               _os.eat(0);
               _as.eat(0);
               _cs.eat(0);
               push_args(args...);
               type_check(_mod.types[_mod.functions[func_index - _mod.import_functions.size()]]);

               _pc = _exit_pc-1;
               push_call(func_index - _mod.import_functions.size());
               _pc = _current_offset;

               setup_locals(func_index);
               execute(visitor);
               stack_elem ret;
               //TODO clean this up
               try {
                  ret = pop_operand();
               } catch(...) {
                  return {};
               }
               _os.eat(0);
               return ret;
            }

            inline void jump( uint32_t label ) { 
               stack_elem el = _cs.pop();
               for (int i=0; i < label; i++)
                  el = _cs.pop();
               uint16_t op_index = 0;
               std::visit(overloaded {
                  [&](const block_t& bt) {
                     _pc = _current_offset + bt.pc+1;
                     op_index = bt.op_index;
                  }, [&](const loop_t& lt) {
                     _pc = _current_offset + lt.pc+1;
                     op_index = lt.op_index;
                  }, [&](const if__t& it) {
                     _pc = _current_offset + it.pc+1;
                     op_index = it.op_index;
                  }, [&](auto) {
                     throw wasm_invalid_element{"invalid element when popping control stack"};
                  }
               }, el);
               bool has_operands = operands() > 0;
               if (has_operands) {
                  const auto& op = pop_operand();
                  eat_operands(op_index);
                  push_operand(op); 
               }

            }

         private:

            template <typename Arg, typename... Args>
            void _push_args(Arg&& arg, Args&&... args) {
               if constexpr (to_wasm_type_v<std::decay_t<Arg>> == types::i32)
                  push_operand({i32_const_t{static_cast<uint32_t>(arg)}});
               else if constexpr (to_wasm_type_v<std::decay_t<Arg>> == types::f32)
                  push_operand(f32_const_t{static_cast<uint32_t>(arg)});
               else if constexpr (to_wasm_type_v<std::decay_t<Arg>> == types::i64)
                  push_operand(i64_const_t{static_cast<uint64_t>(arg)});
               else
                 push_operand(f64_const_t{static_cast<uint64_t>(arg)});
               if constexpr (sizeof...(Args) > 0)
                  _push_args(args...);
            }

            template <typename... Args>
            void push_args(Args&&... args) {
               if constexpr (sizeof...(Args) > 0)
                 _push_args(args...);
            }

            inline void setup_locals(uint32_t index) {
               const auto& fn = _mod.code[index-_mod.get_imported_functions_size()];
               for (int i=0; i < fn.local_count; i++) {
                  for (int j=0; j < fn.locals[i].count; j++)
                     switch (fn.locals[i].type) {
                        case types::i32: 
                           push_operand(i32_const_t{(uint32_t)0});
                           break;
                        case types::i64: 
                           push_operand(i64_const_t{(uint64_t)0});
                           break;
                        case types::f32: 
                           push_operand(f32_const_t{(uint32_t)0});
                           break;
                        case types::f64: 
                           push_operand(f64_const_t{(uint64_t)0});
                           break;
                        default:
                           throw wasm_interpreter_exception{"invalid function param type"};
                     }
               }
            }

            template <typename Visitor>
            void execute( Visitor&& visitor ) {
               do {
                  uint32_t offset = _pc - _current_offset;
                  std::cout << "EXIT PC " << _exit_pc << " PC " << _pc << " Offset " << offset << "\n";
                  if (_pc == _exit_pc) {
                     _executing = false;
                  }
                  std::visit(visitor, _mod.code[_code_index].code[offset]);
                  std::cout << visitor.dbg_output.str() << "\n";
                  visitor.dbg_output.str("");
               } while (_executing);
            }
            uint32_t      _pc               = 0;
            uint32_t      _exit_pc          = 0;
            uint32_t      _current_function = 0;
            uint32_t      _code_index       = 0;
            uint32_t      _current_offset   = 0;
            bool          _executing        = false;
            uint8_t*      _linear_memory    = nullptr;
            module<Backend>&    _mod;
            wasm_allocator&     _alloc;
            control_stack<Backend> _cs;
            operand_stack<Backend> _os;
            call_stack<Backend>    _as;
            registered_host_functions _rhf;
      };
}} // ns eosio::wasm_backend
