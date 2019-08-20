#pragma once

#include <eosio/vm/host_function.hpp>
#include <eosio/vm/signals.hpp>
#include <eosio/vm/types.hpp>
#include <eosio/vm/wasm_stack.hpp>
#include <eosio/vm/watchdog.hpp>
#include <eosio/vm/x86_64.hpp>
#include <eosio/vm/memory_dump.hpp>

#include <fstream>
#include <optional>
#include <string>
#include <utility>

namespace eosio { namespace vm {

   template<typename Derived, typename Host>
   class execution_context_base {
    public:
      Derived& derived() { return static_cast<Derived&>(*this); }
      execution_context_base(module& m) : _mod(m) {}

      inline int32_t grow_linear_memory(int32_t pages) {
         const int32_t sz = _wasm_alloc->get_current_page();
         if (pages < 0 || !_mod.memories.size() || max_pages < sz + pages ||
             (_mod.memories[0].limits.flags && (_mod.memories[0].limits.maximum < sz + pages)))
            return -1;
         _wasm_alloc->alloc<char>(pages);
         return sz;
      }

      inline int32_t current_linear_memory() const { return _wasm_alloc->get_current_page(); }
      inline void     exit(std::error_code err = std::error_code()) {
         // FIXME: system_error?
         _error_code = err;
         throw wasm_exit_exception{"Exiting"};
      }

      inline module&        get_module() { return _mod; }
      inline void           set_wasm_allocator(wasm_allocator* alloc) { _wasm_alloc = alloc; }
      inline auto           get_wasm_allocator() { return _wasm_alloc; }
      inline char*          linear_memory() { return _linear_memory; }

      inline std::error_code get_error_code() const { return _error_code; }

      inline void reset() {
         _linear_memory = _wasm_alloc->get_base_ptr<char>();
         if (_mod.memories.size()) {
            grow_linear_memory(_mod.memories[0].limits.initial - _wasm_alloc->get_current_page());
         }

         for (int i = 0; i < _mod.data.size(); i++) {
            const auto& data_seg = _mod.data[i];
            // TODO validate only use memory idx 0 in parse
            auto addr = _linear_memory + data_seg.offset.value.i32;
            memcpy((char*)(addr), data_seg.data.raw(), data_seg.data.size());
         }

         // reset the mutable globals
         for (int i = 0; i < _mod.globals.size(); i++) {
            if (_mod.globals[i].type.mutability)
               _mod.globals[i].current = _mod.globals[i].init;
         }
      }

      template <typename Visitor, typename... Args>
      inline std::optional<operand_stack_elem> execute(Host* host, Visitor&& visitor, const std::string_view func,
                                               Args... args) {
         uint32_t func_index = _mod.get_exported_function(func);
         return derived().execute(host, std::forward<Visitor>(visitor), func_index, std::forward<Args>(args)...);
      }

      template <typename Visitor, typename... Args>
      inline void execute_start(Host* host, Visitor&& visitor) {
         if (_mod.start != std::numeric_limits<uint32_t>::max())
            derived().execute(host, std::forward<Visitor>(visitor), _mod.start);
      }

    protected:

      static void handle_signal(int sig) {
         switch(sig) {
          case SIGSEGV:
          case SIGBUS:
          case SIGFPE:
            break;
          default:
            /* TODO fix this */
            assert(!"??????");
         }
         throw wasm_memory_exception{ "wasm memory out-of-bounds" };
      }

      char*                           _linear_memory    = nullptr;
      module&                         _mod;
      wasm_allocator*                 _wasm_alloc;
      registered_host_functions<Host> _rhf;
      std::error_code                 _error_code;
   };

   struct jit_visitor { template<typename T> jit_visitor(T&&) {} };

   template<typename Host>
   class jit_execution_context : public execution_context_base<jit_execution_context<Host>, Host> {
      using base_type = execution_context_base<jit_execution_context<Host>, Host>;
   public:
      using base_type::execute;
      using base_type::_linear_memory;
      using base_type::base_type;
      using base_type::_mod;
      using base_type::_rhf;
      using base_type::_error_code;
      using base_type::handle_signal;

      inline operand_stack& get_operand_stack() { return _os; }

      inline native_value call_host_function(native_value* stack, uint32_t index) {
         const auto& ft = _mod.get_function_type(index);
         uint32_t num_params = ft.param_types.size();
#ifndef NDEBUG
         uint32_t original_operands = _os.size();
#endif
         for(uint32_t i = 0; i < ft.param_types.size(); ++i) {
            switch(ft.param_types[i]) {
             case i32: _os.push(i32_const_t{stack[num_params - i - 1].i32}); break;
             case i64: _os.push(i64_const_t{stack[num_params - i - 1].i64}); break;
             case f32: _os.push(f32_const_t{stack[num_params - i - 1].f32}); break;
             case f64: _os.push(f64_const_t{stack[num_params - i - 1].f64}); break;
             default: assert(!"Unexpected type in param_types.");
            }
         }
         _rhf(_host, *this, _mod.import_functions[index]);
         native_value result{uint32_t{0}};
         // guarantee that the junk bits are zero, to avoid problems.
         auto set_result = [&result](auto val) { std::memcpy(&result, &val, sizeof(val)); };
         if(ft.return_count) {
            operand_stack_elem el = _os.pop();
            switch(ft.return_type) {
             case i32: set_result(el.to_ui32()); break;
             case i64: set_result(el.to_ui64()); break;
             case f32: set_result(el.to_f32()); break;
             case f64: set_result(el.to_f64()); break;
             default: assert(!"Unexpected function return type.");
            }
         }
         assert(_os.size() == original_operands);
         return result;
      }

      template <typename... Args>
      inline std::optional<operand_stack_elem> execute(Host* host, jit_visitor, uint32_t func_index, Args... args) {
         auto saved_host = _host;
         auto g = scope_guard([&](){ _host = saved_host; });

         _host = host;

         auto fn = _mod.code[func_index - _mod.get_imported_functions_size()].jit_code;
         const func_type& ft = _mod.get_function_type(func_index);
         native_value result;

         try {
            vm::invoke_with_signal_handler([&]() {
               // FIXME: Move invoke out of machine_code_writer.
               result = machine_code_writer<jit_execution_context>::invoke(fn, this, _linear_memory, args...);
            }, &handle_signal);
         } catch(wasm_exit_exception&) {
            return {};
         }

         if(!ft.return_count)
            return {};
         else switch (ft.return_type) {
            case i32: return {i32_const_t{result.i32}};
            case i64: return {i64_const_t{result.i64}};
            case f32: return {f32_const_t{result.f32}};
            case f64: return {f64_const_t{result.f64}};
            default: assert(!"Unexpected function return type");
         }
         __builtin_unreachable();
      }
   protected:
      Host * _host = nullptr;
      // This is only needed because the host function api uses operand stack
      bounded_allocator _base_allocator = {
         constants::max_stack_size * sizeof(operand_stack_elem)
      };
      operand_stack                   _os = { _base_allocator };
   };

   template <typename Host>
   class execution_context : public execution_context_base<execution_context<Host>, Host> {
      using base_type = execution_context_base<execution_context<Host>, Host>;
    public:
      using base_type::_mod;
      using base_type::_rhf;
      using base_type::_linear_memory;
      using base_type::_error_code;
      using base_type::handle_signal;
      execution_context(module& m) : base_type(m), _halt(exit_t{}) {
         for (int i = 0; i < _mod.exports.size(); i++) _mod.import_functions.resize(_mod.get_imported_functions_size());
         _mod.function_sizes.resize(_mod.get_functions_total());
         const size_t import_size  = _mod.get_imported_functions_size();
         uint32_t     total_so_far = 0;
         for (int i = _mod.get_imported_functions_size(); i < _mod.function_sizes.size(); i++) {
            _mod.function_sizes[i] = total_so_far;
            total_so_far += _mod.code[i - import_size].size;
         }
      }


      inline void call(uint32_t index) {
         // TODO validate index is valid
         if (index < _mod.get_imported_functions_size()) {
            // TODO validate only importing functions
            const auto& ft = _mod.types[_mod.imports[index].type.func_t];
            type_check(ft);
            inc_pc();
            _rhf(_state.host, *this, _mod.import_functions[index]);
         } else {
            // const auto& ft = _mod.types[_mod.functions[index - _mod.get_imported_functions_size()]];
            // type_check(ft);
            push_call(index);
            setup_locals(index);
            set_pc( _mod.get_function_pc(index) );
         }
      }

      void print_stack() {
         std::cout << "STACK { ";
         for (int i = 0; i < _os.size(); i++) {
            std::cout << "(" << i << ")";
            visit(overloaded { [&](i32_const_t el) { std::cout << "i32:" << el.data.ui << ", "; },
                               [&](i64_const_t el) { std::cout << "i64:" << el.data.ui << ", "; },
                               [&](f32_const_t el) { std::cout << "f32:" << el.data.f << ", "; },
                               [&](f64_const_t el) { std::cout << "f64:" << el.data.f << ", "; },
                               [&](auto el) { std::cout << "(INDEX " << el.index() << "), "; } }, _os.get(i));
         }
         std::cout << " }\n";
      }

      inline operand_stack& get_operand_stack() { return _os; }
      inline uint32_t       table_elem(uint32_t i) { return _mod.tables[0].table[i]; }
      inline void           push_operand(const operand_stack_elem& el) { _os.push(el); }
      inline operand_stack_elem     get_operand(uint16_t index) const { return _os.get(_last_op_index + index); }
      inline void           eat_operands(uint16_t index) { _os.eat(index); }
      inline void           compact_operand(uint16_t index) { _os.compact(index); }
      inline void           set_operand(uint16_t index, const operand_stack_elem& el) { _os.set(_last_op_index + index, el); }
      inline uint16_t       current_operands_index() const { return _os.current_index(); }
      inline void           push_call(const activation_frame& el) { _as.push(el); }
      inline activation_frame     pop_call() { return _as.pop(); }
      inline uint32_t       call_depth()const { return _as.size(); }
      template <bool Should_Exit=false>
      inline void           push_call(uint32_t index) {
         opcode* return_pc = static_cast<opcode*>(&_halt);
         if constexpr (!Should_Exit)
            return_pc = _state.pc + 1;

         _as.push( activation_frame{ return_pc, _last_op_index } );
         _last_op_index = _os.size() - _mod.get_function_type(index).param_types.size();
      }

      inline void apply_pop_call(uint32_t num_locals, uint16_t return_count) {
         const auto& af = _as.pop();
         _state.pc = af.pc;
         _last_op_index = af.last_op_index;
         if (return_count)
            compact_operand(_os.size() - num_locals - 1);
         else
            eat_operands(_os.size() - num_locals);
      }
      inline operand_stack_elem  pop_operand() { return _os.pop(); }
      inline operand_stack_elem& peek_operand(size_t i = 0) { return _os.peek(i); }
      inline operand_stack_elem  get_global(uint32_t index) {
         EOS_VM_ASSERT(index < _mod.globals.size(), wasm_interpreter_exception, "global index out of range");
         const auto& gl = _mod.globals[index];
         switch (gl.type.content_type) {
            case types::i32: return i32_const_t{ *(uint32_t*)&gl.current.value.i32 };
            case types::i64: return i64_const_t{ *(uint64_t*)&gl.current.value.i64 };
            case types::f32: return f32_const_t{ gl.current.value.f32 };
            case types::f64: return f64_const_t{ gl.current.value.f64 };
            default: throw wasm_interpreter_exception{ "invalid global type" };
         }
      }

      inline void set_global(uint32_t index, const operand_stack_elem& el) {
         EOS_VM_ASSERT(index < _mod.globals.size(), wasm_interpreter_exception, "global index out of range");
         auto& gl = _mod.globals[index];
         EOS_VM_ASSERT(gl.type.mutability, wasm_interpreter_exception, "global is not mutable");
         visit(overloaded{ [&](const i32_const_t& i) {
                                  EOS_VM_ASSERT(gl.type.content_type == types::i32, wasm_interpreter_exception,
                                                "expected i32 global type");
                                  gl.current.value.i32 = i.data.ui;
                               },
                                [&](const i64_const_t& i) {
                                   EOS_VM_ASSERT(gl.type.content_type == types::i64, wasm_interpreter_exception,
                                                 "expected i64 global type");
                                   gl.current.value.i64 = i.data.ui;
                                },
                                [&](const f32_const_t& f) {
                                   EOS_VM_ASSERT(gl.type.content_type == types::f32, wasm_interpreter_exception,
                                                 "expected f32 global type");
                                   gl.current.value.f32 = f.data.ui;
                                },
                                [&](const f64_const_t& f) {
                                   EOS_VM_ASSERT(gl.type.content_type == types::f64, wasm_interpreter_exception,
                                                 "expected f64 global type");
                                   gl.current.value.f64 = f.data.ui;
                                },
                                [](auto) { throw wasm_interpreter_exception{ "invalid global type" }; } },
                    el);
      }

      inline bool is_true(const operand_stack_elem& el) {
         bool ret_val = false;
         visit(overloaded{ [&](const i32_const_t& i32) { ret_val = i32.data.ui; },
                           [&](auto) { throw wasm_invalid_element{ "should be an i32 type" }; } },
                    el);
         return ret_val;
      }

      inline void type_check(const func_type& ft) {
         for (int i = 0; i < ft.param_types.size(); i++) {
            const auto& op = peek_operand((ft.param_types.size() - 1) - i);
            visit(overloaded{ [&](const i32_const_t&) {
                                     EOS_VM_ASSERT(ft.param_types[i] == types::i32, wasm_interpreter_exception,
                                                   "function param type mismatch");
                                  },
                                   [&](const f32_const_t&) {
                                      EOS_VM_ASSERT(ft.param_types[i] == types::f32, wasm_interpreter_exception,
                                                    "function param type mismatch");
                                   },
                                   [&](const i64_const_t&) {
                                      EOS_VM_ASSERT(ft.param_types[i] == types::i64, wasm_interpreter_exception,
                                                    "function param type mismatch");
                                   },
                                   [&](const f64_const_t&) {
                                      EOS_VM_ASSERT(ft.param_types[i] == types::f64, wasm_interpreter_exception,
                                                    "function param type mismatch");
                                   },
                                   [&](auto) { throw wasm_interpreter_exception{ "function param invalid type" }; } },
                       op);
         }
      }

      inline opcode*  get_pc() const { return _state.pc; }
      inline void     set_relative_pc(uint32_t pc_offset) { 
         _state.pc = _mod.code[0].code + pc_offset;
      }
      inline void     set_pc(opcode* pc) { _state.pc = pc; }
      inline void     inc_pc(uint32_t offset=1) { _state.pc += offset; }
      inline void     exit(std::error_code err = std::error_code()) {
         _error_code = err;
         _state.pc = &_halt;
      }

      inline void reset() {
         base_type::reset();
         _state = execution_state{};
         _os.eat(_state.os_index);
         _as.eat(_state.as_index);
      }

      template <typename Visitor, typename... Args>
      inline std::optional<operand_stack_elem> execute_func_table(Host* host, Visitor&& visitor, uint32_t table_index,
                                                          Args... args) {
         return execute(host, std::forward<Visitor>(visitor), table_elem(table_index), std::forward<Args>(args)...);
      }

      template <typename Visitor, typename... Args>
      inline std::optional<operand_stack_elem> execute(Host* host, Visitor&& visitor, const std::string_view func,
                                               Args... args) {
         uint32_t func_index = _mod.get_exported_function(func);
         return execute(host, std::forward<Visitor>(visitor), func_index, std::forward<Args>(args)...);
      }

      template <typename Visitor, typename... Args>
      inline void execute_start(Host* host, Visitor&& visitor) {
         if (_mod.start != std::numeric_limits<uint32_t>::max())
            execute(host, std::forward<Visitor>(visitor), _mod.start);
      }

      template <typename Visitor, typename... Args>
      inline std::optional<operand_stack_elem> execute(Host* host, Visitor&& visitor, uint32_t func_index, Args... args) {
         EOS_VM_ASSERT(func_index < std::numeric_limits<uint32_t>::max(), wasm_interpreter_exception,
                       "cannot execute function, function not found");

         auto last_last_op_index = _last_op_index;

         // save the state of the original calling context
         execution_state saved_state = _state;

         _state.host             = host;
         _state.pc               = _mod.get_function_pc(func_index);
         _state.as_index         = _as.size();
         _state.os_index         = _os.size();

         auto cleanup = scope_guard([&]() {
            _os.eat(_state.os_index);
            _as.eat(_state.as_index);
            _state = saved_state;

            _last_op_index = last_last_op_index;
         });

         push_args(args...);
         push_call<true>(func_index);
         type_check(_mod.types[_mod.functions[func_index - _mod.import_functions.size()]]);
         setup_locals(func_index);

         try {
            vm::invoke_with_signal_handler([&]() {
               execute(visitor);
            }, &handle_signal);
         } catch(wasm_exit_exception&) {
            return {};
         }

         if (_mod.get_function_type(func_index).return_count) {
            return pop_operand();
         } else {
            return {};
         }
      }

      inline void jump(uint32_t pop_info, uint32_t new_pc) {
         set_relative_pc(new_pc);
         if ((pop_info & 0x80000000u)) {
            const auto& op = pop_operand();
            eat_operands(_os.size() - ((pop_info & 0x7FFFFFFFu) - 1));
            push_operand(op);
         } else {
            eat_operands(_os.size() - pop_info);
         }
      }

    private:
      template <typename Arg, typename... Args>
      void _push_args(Arg&& arg, Args&&... args) {
         if constexpr (to_wasm_type_v<std::decay_t<Arg>> == types::i32)
            push_operand({ i32_const_t{ static_cast<uint32_t>(arg) } });
         else if constexpr (to_wasm_type_v<std::decay_t<Arg>> == types::f32)
            push_operand(f32_const_t{ static_cast<float>(arg) });
         else if constexpr (to_wasm_type_v<std::decay_t<Arg>> == types::i64)
            push_operand(i64_const_t{ static_cast<uint64_t>(arg) });
         else
            push_operand(f64_const_t{ static_cast<double>(arg) });
         if constexpr (sizeof...(Args) > 0)
            _push_args(args...);
      }

      template <typename... Args>
      void push_args(Args&&... args) {
         if constexpr (sizeof...(Args) > 0)
            _push_args(args...);
      }

      inline void setup_locals(uint32_t index) {
         const auto& fn = _mod.code[index - _mod.get_imported_functions_size()];
         for (int i = 0; i < fn.locals.size(); i++) {
            for (int j = 0; j < fn.locals[i].count; j++)
               switch (fn.locals[i].type) {
                  case types::i32: push_operand(i32_const_t{ (uint32_t)0 }); break;
                  case types::i64: push_operand(i64_const_t{ (uint64_t)0 }); break;
                  case types::f32: push_operand(f32_const_t{ (uint32_t)0 }); break;
                  case types::f64: push_operand(f64_const_t{ (uint64_t)0 }); break;
                  default: throw wasm_interpreter_exception{ "invalid function param type" };
               }
         }
      }

#define CREATE_TABLE_ENTRY(NAME, CODE) &&ev_label_##NAME,
#define CREATE_LABEL(NAME, CODE)                                                                                  \
      ev_label_##NAME : visitor(ev_variant->template get<eosio::vm::EOS_VM_OPCODE_T(NAME)>());                    \
      ev_variant = _state.pc; \
      goto* dispatch_table[ev_variant->index()];
#define CREATE_EXIT_LABEL(NAME, CODE) ev_label_##NAME : \
      return;
#define CREATE_EMPTY_LABEL(NAME, CODE) ev_label_##NAME :  \
      throw wasm_interpreter_exception{"empty operand"};

      template <typename Visitor>
      void execute(Visitor&& visitor) {
         static void* dispatch_table[] = {
            EOS_VM_CONTROL_FLOW_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_BR_TABLE_OP(CREATE_TABLE_ENTRY)
            EOS_VM_RETURN_OP(CREATE_TABLE_ENTRY)
            EOS_VM_CALL_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_CALL_IMM_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_PARAMETRIC_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_VARIABLE_ACCESS_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_MEMORY_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_I32_CONSTANT_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_I64_CONSTANT_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_F32_CONSTANT_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_F64_CONSTANT_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_COMPARISON_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_NUMERIC_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_CONVERSION_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_EXIT_OP(CREATE_TABLE_ENTRY)
            EOS_VM_EMPTY_OPS(CREATE_TABLE_ENTRY)
            EOS_VM_ERROR_OPS(CREATE_TABLE_ENTRY)
            &&__ev_last
         };
         auto* ev_variant = _state.pc;
         goto *dispatch_table[ev_variant->index()];
         while (1) {
             EOS_VM_CONTROL_FLOW_OPS(CREATE_LABEL);
             EOS_VM_BR_TABLE_OP(CREATE_LABEL);
             EOS_VM_RETURN_OP(CREATE_LABEL);
             EOS_VM_CALL_OPS(CREATE_LABEL);
             EOS_VM_CALL_IMM_OPS(CREATE_LABEL);
             EOS_VM_PARAMETRIC_OPS(CREATE_LABEL);
             EOS_VM_VARIABLE_ACCESS_OPS(CREATE_LABEL);
             EOS_VM_MEMORY_OPS(CREATE_LABEL);
             EOS_VM_I32_CONSTANT_OPS(CREATE_LABEL);
             EOS_VM_I64_CONSTANT_OPS(CREATE_LABEL);
             EOS_VM_F32_CONSTANT_OPS(CREATE_LABEL);
             EOS_VM_F64_CONSTANT_OPS(CREATE_LABEL);
             EOS_VM_COMPARISON_OPS(CREATE_LABEL);
             EOS_VM_NUMERIC_OPS(CREATE_LABEL);
             EOS_VM_CONVERSION_OPS(CREATE_LABEL);
             EOS_VM_EXIT_OP(CREATE_EXIT_LABEL);
             EOS_VM_EMPTY_OPS(CREATE_EMPTY_LABEL);
             EOS_VM_ERROR_OPS(CREATE_LABEL);
             __ev_last:
                throw wasm_interpreter_exception{"should never reach here"};
         }
      }

#undef CREATE_EMPTY_LABEL
#undef CREATE_LABEL
#undef CREATE_TABLE_ENTRY

      struct execution_state {
         Host* host                = nullptr;
         uint32_t as_index         = 0;
         uint32_t os_index         = 0;
         opcode*  pc               = nullptr;
      };

      bounded_allocator _base_allocator = {
         (constants::max_stack_size + constants::max_call_depth) * (std::max(sizeof(operand_stack_elem), sizeof(activation_frame)))
      };
      execution_state _state;
      uint16_t                        _last_op_index    = 0;
      operand_stack                   _os = { _base_allocator };
      call_stack                      _as = { _base_allocator };
      opcode                          _halt;
   };
}} // namespace eosio::vm
