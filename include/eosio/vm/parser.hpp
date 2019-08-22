#pragma once

#include <eosio/vm/constants.hpp>
#include <eosio/vm/outcome.hpp>
#include <eosio/vm/sections.hpp>
#include <eosio/vm/types.hpp>
#include <eosio/vm/utils.hpp>
#include <eosio/vm/vector.hpp>

#include <set>
#include <stack>
#include <vector>

namespace eosio { namespace vm {

   template <typename Writer>
   class binary_parser {
    public:
      binary_parser(growable_allocator& alloc) : _allocator(alloc) {}

      template <typename T>
      using vec = guarded_vector<T>;

      static inline uint8_t parse_varuint1(wasm_code_ptr& code) { return varuint<1>(code).to(); }

      static inline uint8_t parse_varuint7(wasm_code_ptr& code) { return varuint<7>(code).to(); }

      static inline uint32_t parse_varuint32(wasm_code_ptr& code) { return varuint<32>(code).to(); }

      static inline int8_t parse_varint7(wasm_code_ptr& code) { return varint<7>(code).to(); }

      static inline int32_t parse_varint32(wasm_code_ptr& code) { return varint<32>(code).to(); }

      static inline int64_t parse_varint64(wasm_code_ptr& code) { return varint<64>(code).to(); }

      inline module& parse_module(wasm_code& code, module& mod) {
         wasm_code_ptr cp(code.data(), 0);
         parse_module(cp, code.size(), mod);
         return mod;
      }

      inline module& parse_module2(wasm_code_ptr& code_ptr, size_t sz, module& mod) {
         parse_module(code_ptr, sz, mod);
         return mod;
      }

      void parse_module(wasm_code_ptr& code_ptr, size_t sz, module& mod) {
         _mod = &mod;
         EOS_VM_ASSERT(parse_magic(code_ptr) == constants::magic, wasm_parse_exception, "magic number did not match");
         EOS_VM_ASSERT(parse_version(code_ptr) == constants::version, wasm_parse_exception,
                       "version number did not match");
         for (int i = 0; i < section_id::num_of_elems; i++) {
            if (code_ptr.offset() == sz)
               break;
            code_ptr.add_bounds(constants::id_size);
            auto id = parse_section_id(code_ptr);
            code_ptr.add_bounds(constants::varuint32_size);
            auto len = parse_section_payload_len(code_ptr);
            code_ptr.fit_bounds(len);

            switch (id) {
               case section_id::custom_section: code_ptr += len; break;
               case section_id::type_section: parse_section<section_id::type_section>(code_ptr, mod.types); break;
               case section_id::import_section: parse_section<section_id::import_section>(code_ptr, mod.imports); break;
               case section_id::function_section:
                  parse_section<section_id::function_section>(code_ptr, mod.functions);
                  mod.normalize_types();
                  break;
               case section_id::table_section: parse_section<section_id::table_section>(code_ptr, mod.tables); break;
               case section_id::memory_section:
                  parse_section<section_id::memory_section>(code_ptr, mod.memories);
                  break;
               case section_id::global_section: parse_section<section_id::global_section>(code_ptr, mod.globals); break;
               case section_id::export_section: parse_section<section_id::export_section>(code_ptr, mod.exports); break;
               case section_id::start_section: parse_section<section_id::start_section>(code_ptr, mod.start); break;
               case section_id::element_section:
                  parse_section<section_id::element_section>(code_ptr, mod.elements);
                  break;
               case section_id::code_section: parse_section<section_id::code_section>(code_ptr, mod.code); break;
               case section_id::data_section: parse_section<section_id::data_section>(code_ptr, mod.data); break;
               default: EOS_VM_ASSERT(false, wasm_parse_exception, "error invalid section id");
            }
         }
      }

      inline uint32_t parse_magic(wasm_code_ptr& code) {
         code.add_bounds(constants::magic_size);
         const auto magic = *((uint32_t*)code.raw());
         code += sizeof(uint32_t);
         return magic;
      }
      inline uint32_t parse_version(wasm_code_ptr& code) {
         code.add_bounds(constants::version_size);
         const auto version = *((uint32_t*)code.raw());
         code += sizeof(uint32_t);
         return version;
      }
      inline uint8_t  parse_section_id(wasm_code_ptr& code) { return *code++; }
      inline uint32_t parse_section_payload_len(wasm_code_ptr& code) { return parse_varuint32(code); }

      void parse_import_entry(wasm_code_ptr& code, import_entry& entry) {
         auto len         = parse_varuint32(code);
         entry.module_str = decltype(entry.module_str){ _allocator, len };
         entry.module_str.copy(code.raw(), len);
         code += len;
         len             = parse_varuint32(code);
         entry.field_str = decltype(entry.field_str){ _allocator, len };
         entry.field_str.copy(code.raw(), len);
         code += len;
         entry.kind = (external_kind)(*code++);
         auto type  = parse_varuint32(code);
         switch ((uint8_t)entry.kind) {
            case external_kind::Function: entry.type.func_t = type; break;
            default: EOS_VM_ASSERT(false, wasm_unsupported_import_exception, "only function imports are supported");
         }
      }

      void parse_table_type(wasm_code_ptr& code, table_type& tt) {
         tt.element_type   = *code++;
         tt.limits.flags   = *code++;
         tt.limits.initial = parse_varuint32(code);
         if (tt.limits.flags) {
            tt.limits.maximum = parse_varuint32(code);
            tt.table          = decltype(tt.table){ _allocator, tt.limits.maximum };
            for (int i = 0; i < tt.limits.maximum; i++) tt.table[i] = std::numeric_limits<uint32_t>::max();
         } else {
            tt.table = decltype(tt.table){ _allocator, tt.limits.initial };
            for (int i = 0; i < tt.limits.initial; i++) tt.table[i] = std::numeric_limits<uint32_t>::max();
         }
      }

      void parse_global_variable(wasm_code_ptr& code, global_variable& gv) {
         uint8_t ct           = *code++;
         gv.type.content_type = ct;
         EOS_VM_ASSERT(ct == types::i32 || ct == types::i64 || ct == types::f32 || ct == types::f64,
                       wasm_parse_exception, "invalid global content type");

         gv.type.mutability = *code++;
         parse_init_expr(code, gv.init);
         gv.current = gv.init;
      }

      void parse_memory_type(wasm_code_ptr& code, memory_type& mt) {
         mt.limits.flags   = *code++;
         mt.limits.initial = parse_varuint32(code);
         EOS_VM_ASSERT(mt.limits.initial <= 65535, wasm_parse_exception, "initial memory out of range");
         if (mt.limits.flags) {
            mt.limits.maximum = parse_varuint32(code);
            EOS_VM_ASSERT(mt.limits.maximum >= mt.limits.initial, wasm_parse_exception, "maximum must be at least minimum");
            EOS_VM_ASSERT(mt.limits.maximum <= 65536u, wasm_parse_exception, "maximum memory out of range");
         }
      }

      void parse_export_entry(wasm_code_ptr& code, export_entry& entry) {
         auto len        = parse_varuint32(code);
         entry.field_str = decltype(entry.field_str){ _allocator, len };
         entry.field_str.copy(code.raw(), len);
         code += len;
         entry.kind  = (external_kind)(*code++);
         entry.index = parse_varuint32(code);
	 if (entry.kind == external_kind::Function) {
             _export_indices.insert(entry.index);
	 }
      }

      void parse_func_type(wasm_code_ptr& code, func_type& ft) {
         ft.form                              = *code++;
         decltype(ft.param_types) param_types = { _allocator, parse_varuint32(code) };
         for (size_t i = 0; i < param_types.size(); i++) {
            uint8_t pt        = *code++;
            param_types.at(i) = pt;
            EOS_VM_ASSERT(pt == types::i32 || pt == types::i64 || pt == types::f32 || pt == types::f64,
                          wasm_parse_exception, "invalid function param type");
         }
         ft.param_types  = std::move(param_types);
         ft.return_count = *code++;
         EOS_VM_ASSERT(ft.return_count < 2, wasm_parse_exception, "invalid function return count");
         if (ft.return_count > 0)
            ft.return_type = *code++;
      }

      void parse_elem_segment(wasm_code_ptr& code, elem_segment& es) {
         table_type* tt = nullptr;
         for (int i = 0; i < _mod->tables.size(); i++) {
            if (_mod->tables[i].element_type == types::anyfunc)
               tt = &(_mod->tables[i]);
         }
         EOS_VM_ASSERT(tt != nullptr, wasm_parse_exception, "table not declared");
         es.index = parse_varuint32(code);
         EOS_VM_ASSERT(es.index == 0, wasm_parse_exception, "only table index of 0 is supported");
         parse_init_expr(code, es.offset);
         uint32_t           size  = parse_varuint32(code);
         decltype(es.elems) elems = { _allocator, size };
         for (uint32_t i = 0; i < size; i++) {
            uint32_t index                     = parse_varuint32(code);
            tt->table[es.offset.value.i32 + i] = index;
            elems.at(i)                        = index;
         }
         es.elems = std::move(elems);
      }

      void parse_init_expr(wasm_code_ptr& code, init_expr& ie) {
         ie.opcode = *code++;
         switch (ie.opcode) {
            case opcodes::i32_const: ie.value.i32 = parse_varint32(code); break;
            case opcodes::i64_const: ie.value.i64 = parse_varint64(code); break;
            case opcodes::f32_const:
               std::memcpy(&ie.value.f32, code.raw(), sizeof(float));
               code += sizeof(float);
               break;
            case opcodes::f64_const:
               std::memcpy(&ie.value.f64, code.raw(), sizeof(double));
               code += sizeof(double);
               break;
            default:
               EOS_VM_ASSERT(false, wasm_parse_exception,
                             "initializer expression can only acception i32.const, i64.const, f32.const and f64.const");
         }
         EOS_VM_ASSERT((*code++) == opcodes::end, wasm_parse_exception, "no end op found");
      }

      void parse_function_body(wasm_code_ptr& code, function_body& fb, std::size_t idx) {
         fb.size   = parse_varuint32(code);
         const auto&         before    = code.offset();
         const auto&         local_cnt = parse_varuint32(code);
         _current_function_index++;
         decltype(fb.locals) locals    = { _allocator, local_cnt };
         // parse the local entries
         for (size_t i = 0; i < local_cnt; i++) {
            locals.at(i).count = parse_varuint32(code);
            locals.at(i).type  = *code++;
         }
         fb.locals = std::move(locals);

         fb.size -= code.offset() - before;
         _function_bodies.emplace_back(code.raw(), fb.size);

         code += fb.size-1;
         EOS_VM_ASSERT(*code++ == 0x0B, wasm_parse_exception, "failed parsing function body, expected 'end'");

      }

      // The control stack holds either address of the target of the
      // label (for backward jumps) or a list of instructions to be
      // updated (for forward jumps).
      //
      // Inside an if: The first element refers to the `if` and should
      // jump to `else`.  The remaining elements should branch to `end`
      using label_t = decltype(std::declval<Writer>().emit_end());
      using branch_t = decltype(std::declval<Writer>().emit_if());
      struct pc_element_t {
         uint32_t operand_depth;
         uint32_t expected_result;
         uint32_t label_result;
         std::variant<label_t, std::vector<branch_t>> relocations;
      };

      static constexpr uint8_t any_type = 0x82;
      struct operand_stack_type_tracker {
        std::vector<uint8_t> state = { scope_tag };
         static constexpr uint8_t unreachable_tag = 0x80;
         static constexpr uint8_t scope_tag = 0x81;
         uint32_t operand_depth = 0;
         void push(uint8_t type) {
            assert(type != unreachable_tag && type != scope_tag);
            assert(type == types::i32 || type == types::i64 || type == types::f32 || type == types::f64 || type == any_type);
            // any_type can only be pushed by select.
            if(type == any_type) {
               assert(!state.empty() && state.back() == unreachable_tag);
               return;
            }
            EOS_VM_ASSERT(operand_depth < std::numeric_limits<uint32_t>::max(), wasm_parse_exception, "integer overflow in operand depth");
            ++operand_depth;
            state.push_back(type);
         }
         void pop(uint8_t expected) {
            assert(expected != unreachable_tag && expected != scope_tag);
            if(expected == types::pseudo) return;
            EOS_VM_ASSERT(!state.empty(), wasm_parse_exception, "unexpected pop");
            if (state.back() != unreachable_tag) {
               EOS_VM_ASSERT(state.back() == expected, wasm_parse_exception, "wrong type");
               --operand_depth;
               state.pop_back();
            }
         }
         uint8_t pop() {
            EOS_VM_ASSERT(!state.empty() && state.back() != scope_tag, wasm_parse_exception, "unexpected pop");
            if (state.back() == unreachable_tag)
               return any_type;
            else {
               uint8_t result = state.back();
               --operand_depth;
               state.pop_back();
               return result;
            }
         }
         void top(uint8_t expected) {
            assert(expected != unreachable_tag && expected != scope_tag);
            EOS_VM_ASSERT(!state.empty(), wasm_parse_exception, "expected a value");
            EOS_VM_ASSERT(state.back() == expected || state.back() == unreachable_tag, wasm_parse_exception, "wrong type");
         }
         void start_unreachable() {
            while(!state.empty() && state.back() != scope_tag) {
               if (state.back() != unreachable_tag)
                  --operand_depth;
               state.pop_back();
            }
            state.push_back(unreachable_tag);
         }
         void push_scope() {
            state.push_back(scope_tag);
         }
         void pop_scope(uint8_t expected_result = types::pseudo) {
            pop(expected_result);
            EOS_VM_ASSERT(!state.empty(), wasm_parse_exception, "unexpected end");
            if (state.back() == unreachable_tag) {
               state.pop_back();
            }
            EOS_VM_ASSERT(state.back() == scope_tag, wasm_parse_exception, "unexpected end");
            state.pop_back();
            if (expected_result != types::pseudo) {
               push(expected_result);
            }
         }
         void finish() {
            if (!state.empty() && state.back() == unreachable_tag) {
               state.pop_back();
            }
            EOS_VM_ASSERT(state.empty(), wasm_parse_exception, "stack not empty at scope end");
         }
         uint32_t depth() const { return operand_depth; }
      };

      struct local_types_t {
         local_types_t(const func_type& ft, const guarded_vector<local_entry>& locals_arg) {
            for (uint32_t i = 0; i < ft.param_types.size(); ++i) {
               locals.push_back(ft.param_types[i]);
            }
            for (uint32_t i = 0; i < locals_arg.size(); ++i) {
               locals.insert(locals.end(), locals_arg[i].count, locals_arg[i].type);
            }
         }
         uint8_t operator[](uint32_t local_idx) const {
            EOS_VM_ASSERT(local_idx < locals.size(), wasm_parse_exception, "undefined local");
            return locals[local_idx];
         }
         std::vector<value_type> locals;
      };

      void parse_function_body_code(wasm_code_ptr& code, size_t bounds, Writer& code_writer, const func_type& ft, const guarded_vector<local_entry>& locals) {
         // Initialize the control stack with the current function as the sole element
         operand_stack_type_tracker op_stack;
         std::vector<pc_element_t> pc_stack{{
               op_stack.depth(),
               ft.return_count ? ft.return_type : static_cast<uint32_t>(types::pseudo),
               ft.return_count ? ft.return_type : static_cast<uint32_t>(types::pseudo),
               std::vector<branch_t>{}}};

         local_types_t local_types(ft, locals);

         // writes the continuation of a label to address.  If the continuation
         // is not yet available, address will be recorded in the relocations
         // list for label.
         auto handle_branch_target = [&](uint32_t label, branch_t address) {
            EOS_VM_ASSERT(label < pc_stack.size(), wasm_parse_exception, "invalid label");
            pc_element_t& branch_target = pc_stack[pc_stack.size() - label - 1];
            std::visit(overloaded{ [&](label_t target) { code_writer.fix_branch(address, target); },
                                   [&](std::vector<branch_t>& relocations) { relocations.push_back(address); } },
               branch_target.relocations);
         };

         // Returns the number of operands that need to be popped when
         // branching to label.  If the label has a return value it will
         // be counted in this, and the high bit will be set to signal
         // its presence.
         auto compute_depth_change = [&](uint32_t label) -> uint32_t {
            EOS_VM_ASSERT(label < pc_stack.size(), wasm_parse_exception, "invalid label");
            pc_element_t& branch_target = pc_stack[pc_stack.size() - label - 1];
            uint32_t result = op_stack.depth() - branch_target.operand_depth;
            if(branch_target.label_result != types::pseudo) {
               // FIXME: Reusing the high bit imposes an additional constraint
               // on the maximum depth of the operand stack.  This isn't an
               // actual problem right now, because the stack is hard-coded
               // to 8192 elements, but it would be better to avoid spreading
               // this assumption around the code.
               result |= 0x80000000;
               op_stack.top(branch_target.label_result);
            }
            return result;
         };

         // Handles branches to the end of the scope and pops the pc_stack
         auto exit_scope = [&]() {
            // There must be at least one element
            EOS_VM_ASSERT(pc_stack.size(), wasm_invalid_element, "unexpected end instruction");
            auto end_pos = code_writer.emit_end();
            if(auto* relocations = std::get_if<std::vector<branch_t>>(&pc_stack.back().relocations)) {
               for(auto branch_op : *relocations) {
                  code_writer.fix_branch(branch_op, end_pos);
               }
            }
            op_stack.pop_scope(pc_stack.back().expected_result);
            pc_stack.pop_back();
         };

         while (code.offset() < bounds) {
            EOS_VM_ASSERT(pc_stack.size() <= constants::max_nested_structures, wasm_parse_exception,
                          "nested structures validation failure");

            switch (*code++) {
               case opcodes::unreachable: code_writer.emit_unreachable(); op_stack.start_unreachable(); break;
               case opcodes::nop: code_writer.emit_nop(); break;
               case opcodes::end: {
                  exit_scope();
                  break;
               }
               case opcodes::return_: {
                  uint32_t label = pc_stack.size() - 1;
                  auto branch = code_writer.emit_return(compute_depth_change(label));
                  handle_branch_target(label, branch);
                  op_stack.start_unreachable();
               } break;
               case opcodes::block: {
                  uint32_t expected_result = *code++;
                  pc_stack.push_back({op_stack.depth(), expected_result, expected_result, std::vector<branch_t>{}});
                  code_writer.emit_block();
                  op_stack.push_scope();
               } break;
               case opcodes::loop: {
                  uint32_t expected_result = *code++;
                  auto pos = code_writer.emit_loop();
                  pc_stack.push_back({op_stack.depth(), expected_result, types::pseudo, pos});
                  op_stack.push_scope();
               } break;
               case opcodes::if_: {
                  uint32_t expected_result = *code++;
                  auto branch = code_writer.emit_if();
                  op_stack.pop(types::i32);
                  pc_stack.push_back({op_stack.depth(), expected_result, expected_result, std::vector{branch}});
                  op_stack.push_scope();
               } break;
               case opcodes::else_: {
                  auto& old_index = pc_stack.back();
                  auto& relocations = std::get<std::vector<branch_t>>(old_index.relocations);
                  // reset the operand stack to the same state as the if
                  op_stack.pop(old_index.expected_result);
                  op_stack.pop_scope();
                  op_stack.push_scope();
                  // Overwrite the branch from the `if` with the `else`.
                  // We're left with a normal relocation list where everything
                  // branches to the corresponding `end`
                  relocations[0] = code_writer.emit_else(relocations[0]);
                  break;
               }
               case opcodes::br: {
                  uint32_t label = parse_varuint32(code);
                  auto branch = code_writer.emit_br(compute_depth_change(label));
                  handle_branch_target(label, branch);
                  op_stack.start_unreachable();
               } break;
               case opcodes::br_if: {
                  uint32_t label = parse_varuint32(code);
                  op_stack.pop(types::i32);
                  auto branch = code_writer.emit_br_if(compute_depth_change(label));
                  handle_branch_target(label, branch);
               } break;
               case opcodes::br_table: {
                  size_t table_size = parse_varuint32(code);
                  op_stack.pop(types::i32);
                  auto handler = code_writer.emit_br_table(table_size);
                  for (size_t i = 0; i < table_size; i++) {
                     uint32_t label = parse_varuint32(code);
                     auto branch = handler.emit_case(compute_depth_change(label));
                     handle_branch_target(label, branch);
                  }
                  uint32_t label = parse_varuint32(code);
                  auto branch = handler.emit_default(compute_depth_change(label));
                  handle_branch_target(label, branch);
                  op_stack.start_unreachable();
               } break;
               case opcodes::call: {
                  uint32_t funcnum = parse_varuint32(code);
                  const func_type& ft = _mod->get_function_type(funcnum);
                  for(uint32_t i = 0; i < ft.param_types.size(); ++i)
                     op_stack.pop(ft.param_types[ft.param_types.size() - i - 1]);
                  EOS_VM_ASSERT(ft.return_count <= 1, wasm_parse_exception, "unsupported");
                  if(ft.return_count)
                     op_stack.push(ft.return_type);
                  code_writer.emit_call(ft, funcnum);
               } break;
               case opcodes::call_indirect: {
                  uint32_t functypeidx = parse_varuint32(code);
                  const func_type& ft = _mod->types.at(functypeidx);
                  op_stack.pop(types::i32);
                  for(uint32_t i = 0; i < ft.param_types.size(); ++i)
                     op_stack.pop(ft.param_types[ft.param_types.size() - i - 1]);
                  EOS_VM_ASSERT(ft.return_count <= 1, wasm_parse_exception, "unsupported");
                  if(ft.return_count)
                     op_stack.push(ft.return_type);
                  code_writer.emit_call_indirect(ft, functypeidx);
                  code++; // 0x00
                  break;
               }
               case opcodes::drop: code_writer.emit_drop(); op_stack.pop(); break;
               case opcodes::select: {
                  code_writer.emit_select();
                  op_stack.pop(types::i32);
                  uint8_t t0 = op_stack.pop();
                  uint8_t t1 = op_stack.pop();
                  EOS_VM_ASSERT(t0 == t1 || t0 == any_type || t1 == any_type, wasm_parse_exception, "incorrect types for select");
                  op_stack.push(t0 != any_type? t0 : t1);
               } break;
               case opcodes::get_local: {
                  uint32_t local_idx = parse_varuint32(code);
                  code_writer.emit_get_local(local_idx);
                  op_stack.push(local_types[local_idx]);
               } break;
               case opcodes::set_local: {
                  uint32_t local_idx = parse_varuint32(code);
                  code_writer.emit_set_local(local_idx);
                  op_stack.pop(local_types[local_idx]);
               } break;
               case opcodes::tee_local: {
                  uint32_t local_idx = parse_varuint32(code);
                  code_writer.emit_tee_local(local_idx);
                  op_stack.top(local_types[local_idx]);
               } break;
               case opcodes::get_global: {
                  uint32_t global_idx = parse_varuint32(code);
                  code_writer.emit_get_global(global_idx);
                  op_stack.push(_mod->globals.at(global_idx).type.content_type);
               } break;
               case opcodes::set_global: {
                  uint32_t global_idx = parse_varuint32(code);
                  code_writer.emit_set_global(global_idx);
                  EOS_VM_ASSERT(_mod->globals.at(global_idx).type.mutability, wasm_parse_exception, "cannot set const global");
                  op_stack.pop(_mod->globals.at(global_idx).type.content_type);
               } break;
#define LOAD_OP(op_name, max_align, type)                            \
               case opcodes::op_name: {                              \
                  EOS_VM_ASSERT(_mod->memories.size() > 0, wasm_parse_exception, "load requires memory"); \
                  uint32_t alignment = parse_varuint32(code);        \
                  uint32_t offset = parse_varuint32(code);           \
                  EOS_VM_ASSERT(alignment <= uint32_t(max_align), wasm_parse_exception, "alignment cannot be greater than size."); \
                  code_writer.emit_ ## op_name( alignment, offset ); \
                  op_stack.pop(types::i32);                          \
                  op_stack.push(types::type);                        \
               } break;

               LOAD_OP(i32_load, 2, i32)
               LOAD_OP(i64_load, 3, i64)
               LOAD_OP(f32_load, 2, f32)
               LOAD_OP(f64_load, 3, f64)
               LOAD_OP(i32_load8_s, 0, i32)
               LOAD_OP(i32_load16_s, 1, i32)
               LOAD_OP(i32_load8_u, 0, i32)
               LOAD_OP(i32_load16_u, 1, i32)
               LOAD_OP(i64_load8_s, 0, i64)
               LOAD_OP(i64_load16_s, 1, i64)
               LOAD_OP(i64_load32_s, 2, i64)
               LOAD_OP(i64_load8_u, 0, i64)
               LOAD_OP(i64_load16_u, 1, i64)
               LOAD_OP(i64_load32_u, 2, i64)

#undef LOAD_OP
                     
#define STORE_OP(op_name, max_align, type)                           \
               case opcodes::op_name: {                              \
                  EOS_VM_ASSERT(_mod->memories.size() > 0, wasm_parse_exception, "store requires memory"); \
                  uint32_t alignment = parse_varuint32(code);        \
                  uint32_t offset = parse_varuint32(code);           \
                  EOS_VM_ASSERT(alignment <= uint32_t(max_align), wasm_parse_exception, "alignment cannot be greater than size."); \
                  code_writer.emit_ ## op_name( alignment, offset ); \
                  op_stack.pop(types::type);                         \
                  op_stack.pop(types::i32);                          \
               } break;

               STORE_OP(i32_store, 2, i32)
               STORE_OP(i64_store, 3, i64)
               STORE_OP(f32_store, 2, f32)
               STORE_OP(f64_store, 3, f64)
               STORE_OP(i32_store8, 0, i32)
               STORE_OP(i32_store16, 1, i32)
               STORE_OP(i64_store8, 0, i64)
               STORE_OP(i64_store16, 1, i64)
               STORE_OP(i64_store32, 2, i64)

#undef STORE_OP

               case opcodes::current_memory:
                  EOS_VM_ASSERT(_mod->memories.size() != 0, wasm_parse_exception, "memory.size requires memory");
                  code_writer.emit_current_memory();
                  op_stack.push(types::i32);
                  code++;
                  break;
               case opcodes::grow_memory:
                  EOS_VM_ASSERT(_mod->memories.size() != 0, wasm_parse_exception, "memory.grow requires memory");
                  code_writer.emit_grow_memory();
                  op_stack.pop(types::i32);
                  op_stack.push(types::i32);
                  code++;
                  break;
               case opcodes::i32_const: code_writer.emit_i32_const( parse_varint32(code) ); op_stack.push(types::i32); break;
               case opcodes::i64_const: code_writer.emit_i64_const( parse_varint64(code) ); op_stack.push(types::i64); break;
               case opcodes::f32_const: {
                  float value;
                  memcpy(&value, code.raw(), 4);
                  code_writer.emit_f32_const( value );
                  code += 4;
                  op_stack.push(types::f32);
               } break;
               case opcodes::f64_const: {
                  double value;
                  memcpy(&value, code.raw(), 8);
                  code_writer.emit_f64_const( value );
                  code += 8;
                  op_stack.push(types::f64);
               } break;

#define UNOP(opname) \
               case opcodes::opname: code_writer.emit_ ## opname(); op_stack.pop(types::A); op_stack.push(types::R); break;
#define BINOP(opname) \
               case opcodes::opname: code_writer.emit_ ## opname(); op_stack.pop(types::A); op_stack.pop(types::A); op_stack.push(types::R); break;
#define CASTOP(dst, opname, src)                                         \
               case opcodes::dst ## _ ## opname ## _ ## src: code_writer.emit_ ## dst ## _ ## opname ## _ ## src(); op_stack.pop(types::src); op_stack.push(types::dst); break;

#define R i32
#define A i32  
               UNOP(i32_eqz)
               BINOP(i32_eq)
               BINOP(i32_ne)
               BINOP(i32_lt_s)
               BINOP(i32_lt_u)
               BINOP(i32_gt_s)
               BINOP(i32_gt_u)
               BINOP(i32_le_s)
               BINOP(i32_le_u)
               BINOP(i32_ge_s)
               BINOP(i32_ge_u)
#undef A
#define A i64                 
               UNOP(i64_eqz)
               BINOP(i64_eq)
               BINOP(i64_ne)
               BINOP(i64_lt_s)
               BINOP(i64_lt_u)
               BINOP(i64_gt_s)
               BINOP(i64_gt_u)
               BINOP(i64_le_s)
               BINOP(i64_le_u)
               BINOP(i64_ge_s)
               BINOP(i64_ge_u)
#undef A
#define A f32
               BINOP(f32_eq)
               BINOP(f32_ne)
               BINOP(f32_lt)
               BINOP(f32_gt)
               BINOP(f32_le)
               BINOP(f32_ge)
#undef A
#define A f64
               BINOP(f64_eq)
               BINOP(f64_ne)
               BINOP(f64_lt)
               BINOP(f64_gt)
               BINOP(f64_le)
               BINOP(f64_ge)
#undef A
#undef R
#define R A
#define A i32
               UNOP(i32_clz)
               UNOP(i32_ctz)
               UNOP(i32_popcnt)
               BINOP(i32_add)
               BINOP(i32_sub)
               BINOP(i32_mul)
               BINOP(i32_div_s)
               BINOP(i32_div_u)
               BINOP(i32_rem_s)
               BINOP(i32_rem_u)
               BINOP(i32_and)
               BINOP(i32_or)
               BINOP(i32_xor)
               BINOP(i32_shl)
               BINOP(i32_shr_s)
               BINOP(i32_shr_u)
               BINOP(i32_rotl)
               BINOP(i32_rotr)
#undef A
#define A i64
               UNOP(i64_clz)
               UNOP(i64_ctz)
               UNOP(i64_popcnt)
               BINOP(i64_add)
               BINOP(i64_sub)
               BINOP(i64_mul)
               BINOP(i64_div_s)
               BINOP(i64_div_u)
               BINOP(i64_rem_s)
               BINOP(i64_rem_u)
               BINOP(i64_and)
               BINOP(i64_or)
               BINOP(i64_xor)
               BINOP(i64_shl)
               BINOP(i64_shr_s)
               BINOP(i64_shr_u)
               BINOP(i64_rotl)
               BINOP(i64_rotr)
#undef A
#define A f32
               UNOP(f32_abs)
               UNOP(f32_neg)
               UNOP(f32_ceil)
               UNOP(f32_floor)
               UNOP(f32_trunc)
               UNOP(f32_nearest)
               UNOP(f32_sqrt)
               BINOP(f32_add)
               BINOP(f32_sub)
               BINOP(f32_mul)
               BINOP(f32_div)
               BINOP(f32_min)
               BINOP(f32_max)
               BINOP(f32_copysign)
#undef A
#define A f64
               UNOP(f64_abs)
               UNOP(f64_neg)
               UNOP(f64_ceil)
               UNOP(f64_floor)
               UNOP(f64_trunc)
               UNOP(f64_nearest)
               UNOP(f64_sqrt)
               BINOP(f64_add)
               BINOP(f64_sub)
               BINOP(f64_mul)
               BINOP(f64_div)
               BINOP(f64_min)
               BINOP(f64_max)
               BINOP(f64_copysign)
#undef A
#undef R

               CASTOP(i32, wrap, i64)
               CASTOP(i32, trunc_s, f32)
               CASTOP(i32, trunc_u, f32)
               CASTOP(i32, trunc_s, f64)
               CASTOP(i32, trunc_u, f64)
               CASTOP(i64, extend_s, i32)
               CASTOP(i64, extend_u, i32)
               CASTOP(i64, trunc_s, f32)
               CASTOP(i64, trunc_u, f32)
               CASTOP(i64, trunc_s, f64)
               CASTOP(i64, trunc_u, f64)
               CASTOP(f32, convert_s, i32)
               CASTOP(f32, convert_u, i32)
               CASTOP(f32, convert_s, i64)
               CASTOP(f32, convert_u, i64)
               CASTOP(f32, demote, f64)
               CASTOP(f64, convert_s, i32)
               CASTOP(f64, convert_u, i32)
               CASTOP(f64, convert_s, i64)
               CASTOP(f64, convert_u, i64)
               CASTOP(f64, promote, f32)
               CASTOP(i32, reinterpret, f32)
               CASTOP(i64, reinterpret, f64)
               CASTOP(f32, reinterpret, i32)
               CASTOP(f64, reinterpret, i64)

#undef CASTOP
#undef UNOP
#undef BINOP
               case opcodes::error: code_writer.emit_error(); break;
            }
         }
      }

      void parse_data_segment(wasm_code_ptr& code, data_segment& ds) {
         EOS_VM_ASSERT(_mod->memories.size() != 0, wasm_parse_exception, "data requires memory");
         ds.index = parse_varuint32(code);
         parse_init_expr(code, ds.offset);
         ds.data = decltype(ds.data){ _allocator, parse_varuint32(code) };
         ds.data.copy(code.raw(), ds.data.size());
         code += ds.data.size();
      }

      template <typename Elem, typename ParseFunc>
      inline void parse_section_impl(wasm_code_ptr& code, vec<Elem>& elems, ParseFunc&& elem_parse) {
         auto count = parse_varuint32(code);
         elems      = vec<Elem>{ _allocator, count };
         for (size_t i = 0; i < count; i++) { elem_parse(code, elems.at(i), i); }
      }

      template <uint8_t id>
      inline void parse_section_header(wasm_code_ptr& code) {
         code.add_bounds(constants::id_size);
         auto _id = parse_section_id(code);
         // ignore custom sections
         if (_id == section_id::custom_section) {
            code.add_bounds(constants::varuint32_size);
            code += parse_section_payload_len(code);
            code.fit_bounds(constants::id_size);
            _id = parse_section_id(code);
         }
         EOS_VM_ASSERT(_id == id, wasm_parse_exception, "Section id does not match");
         code.add_bounds(constants::varuint32_size);
         code.fit_bounds(parse_section_payload_len(code));
      }

      template <uint8_t id>
      inline void parse_section(wasm_code_ptr&                                                             code,
                                vec<typename std::enable_if_t<id == section_id::type_section, func_type>>& elems) {
         parse_section_impl(code, elems, [&](wasm_code_ptr& code, func_type& ft, std::size_t /*idx*/) { parse_func_type(code, ft); });
      }
      template <uint8_t id>
      inline void parse_section(wasm_code_ptr&                                                                  code,
                                vec<typename std::enable_if_t<id == section_id::import_section, import_entry>>& elems) {
         parse_section_impl(code, elems, [&](wasm_code_ptr& code, import_entry& ie, std::size_t /*idx*/) { parse_import_entry(code, ie); });
      }
      template <uint8_t id>
      inline void parse_section(wasm_code_ptr&                                                                code,
                                vec<typename std::enable_if_t<id == section_id::function_section, uint32_t>>& elems) {
         parse_section_impl(code, elems, [&](wasm_code_ptr& code, uint32_t& elem, std::size_t /*idx*/) { elem = parse_varuint32(code); });
      }
      template <uint8_t id>
      inline void parse_section(wasm_code_ptr&                                                               code,
                                vec<typename std::enable_if_t<id == section_id::table_section, table_type>>& elems) {
         parse_section_impl(code, elems, [&](wasm_code_ptr& code, table_type& tt, std::size_t /*idx*/) { parse_table_type(code, tt); });
      }
      template <uint8_t id>
      inline void parse_section(wasm_code_ptr&                                                                 code,
                                vec<typename std::enable_if_t<id == section_id::memory_section, memory_type>>& elems) {
         parse_section_impl(code, elems, [&](wasm_code_ptr& code, memory_type& mt, std::size_t idx) {
            EOS_VM_ASSERT(idx == 0, wasm_parse_exception, "only one memory is permitted");
            parse_memory_type(code, mt);
         });
      }
      template <uint8_t id>
      inline void
      parse_section(wasm_code_ptr&                                                                     code,
                    vec<typename std::enable_if_t<id == section_id::global_section, global_variable>>& elems) {
         parse_section_impl(code, elems,
                            [&](wasm_code_ptr& code, global_variable& gv, std::size_t /*idx*/) { parse_global_variable(code, gv); });
      }
      template <uint8_t id>
      inline void parse_section(wasm_code_ptr&                                                                  code,
                                vec<typename std::enable_if_t<id == section_id::export_section, export_entry>>& elems) {
         parse_section_impl(code, elems, [&](wasm_code_ptr& code, export_entry& ee, std::size_t /*idx*/) { parse_export_entry(code, ee); });
      }
      template <uint8_t id>
      inline void parse_section(wasm_code_ptr&                                                        code,
                                typename std::enable_if_t<id == section_id::start_section, uint32_t>& start) {
         start = parse_varuint32(code);
      }
      template <uint8_t id>
      inline void
      parse_section(wasm_code_ptr&                                                                   code,
                    vec<typename std::enable_if_t<id == section_id::element_section, elem_segment>>& elems) {
         parse_section_impl(code, elems, [&](wasm_code_ptr& code, elem_segment& es, std::size_t /*idx*/) { parse_elem_segment(code, es); });
      }
      template <uint8_t id>
      inline void parse_section(wasm_code_ptr&                                                                 code,
                                vec<typename std::enable_if_t<id == section_id::code_section, function_body>>& elems) {
         parse_section_impl(code, elems,
                            [&](wasm_code_ptr& code, function_body& fb, std::size_t idx) { parse_function_body(code, fb, idx); });
         Writer code_writer(_allocator, code.bounds() - code.offset(), *_mod);
         for (size_t i = 0; i < _function_bodies.size(); i++) {
            function_body& fb = _mod->code[i];
            func_type& ft = _mod->types.at(_mod->functions.at(i));
            code_writer.emit_prologue(ft, fb.locals, i);
            parse_function_body_code(_function_bodies[i], fb.size, code_writer, ft, fb.locals);
            code_writer.emit_epilogue(ft, fb.locals, i);
            code_writer.finalize(fb);
         }
      }
      template <uint8_t id>
      inline void parse_section(wasm_code_ptr&                                                                code,
                                vec<typename std::enable_if_t<id == section_id::data_section, data_segment>>& elems) {
         parse_section_impl(code, elems, [&](wasm_code_ptr& code, data_segment& ds, std::size_t /*idx*/) { parse_data_segment(code, ds); });
      }

      template <size_t N>
      varint<N> parse_varint(const wasm_code& code, size_t index) {
         varint<N> result(0);
         result.set(code, index);
         return result;
      }

      template <size_t N>
      varuint<N> parse_varuint(const wasm_code& code, size_t index) {
         varuint<N> result(0);
         result.set(code, index);
         return result;
      }

    private:
      growable_allocator& _allocator;
      module*             _mod; // non-owning weak pointer
      int64_t             _current_function_index = -1;
      std::set<uint32_t>  _export_indices;
      std::vector<wasm_code_ptr>  _function_bodies;
   };
}} // namespace eosio::vm
