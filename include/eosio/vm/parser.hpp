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
         EOS_WB_ASSERT(parse_magic(code_ptr) == constants::magic, wasm_parse_exception, "magic number did not match");
         EOS_WB_ASSERT(parse_version(code_ptr) == constants::version, wasm_parse_exception,
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
               default: EOS_WB_ASSERT(false, wasm_parse_exception, "error invalid section id");
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
            default: EOS_WB_ASSERT(false, wasm_unsupported_import_exception, "only function imports are supported");
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
         EOS_WB_ASSERT(ct == types::i32 || ct == types::i64 || ct == types::f32 || ct == types::f64,
                       wasm_parse_exception, "invalid global content type");

         gv.type.mutability = *code++;
         parse_init_expr(code, gv.init);
         gv.current = gv.init;
      }

      void parse_memory_type(wasm_code_ptr& code, memory_type& mt) {
         mt.limits.flags   = *code++;
         mt.limits.initial = parse_varuint32(code);
         if (mt.limits.flags) {
            mt.limits.maximum = parse_varuint32(code);
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
            EOS_WB_ASSERT(pt == types::i32 || pt == types::i64 || pt == types::f32 || pt == types::f64,
                          wasm_parse_exception, "invalid function param type");
         }
         ft.param_types  = std::move(param_types);
         ft.return_count = *code++;
         EOS_WB_ASSERT(ft.return_count < 2, wasm_parse_exception, "invalid function return count");
         if (ft.return_count > 0)
            ft.return_type = *code++;
      }

      void parse_elem_segment(wasm_code_ptr& code, elem_segment& es) {
         table_type* tt = nullptr;
         for (int i = 0; i < _mod->tables.size(); i++) {
            if (_mod->tables[i].element_type == types::anyfunc)
               tt = &(_mod->tables[i]);
         }
         EOS_WB_ASSERT(tt != nullptr, wasm_parse_exception, "table not declared");
         es.index = parse_varuint32(code);
         EOS_WB_ASSERT(es.index == 0, wasm_parse_exception, "only table index of 0 is supported");
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
               EOS_WB_ASSERT(false, wasm_parse_exception,
                             "initializer expression can only acception i32.const, i64.const, f32.const and f64.const");
         }
         EOS_WB_ASSERT((*code++) == opcodes::end, wasm_parse_exception, "no end op found");
      }

      void parse_function_body(wasm_code_ptr& code, function_body& fb, std::size_t idx, Writer& code_writer) {
         const auto&         fn_type   = _mod->types.at(_mod->functions.at(idx));

         const auto&         body_size = parse_varuint32(code);
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

         // -1 is 'end' 0xb byte and one extra slot for an exiting instruction to be held during execution, this is used to drive the pc past normal execution
         size_t            bytes = (body_size - (code.offset() - before));
         wasm_code_ptr     fb_code(code.raw(), bytes);
         code_writer.emit_prologue(fn_type, locals, idx);
         parse_function_body_code(fb_code, bytes, code_writer, fn_type);
         code_writer.emit_epilogue(fn_type, locals, idx);
         code += bytes - 1;
         EOS_WB_ASSERT(*code++ == 0x0B, wasm_parse_exception, "failed parsing function body, expected 'end'");
         code_writer.finalize(fb);
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
         bool is_unreachable;
         std::variant<label_t, std::vector<branch_t>> relocations;
      };

      void parse_function_body_code(wasm_code_ptr& code, size_t bounds, Writer& code_writer, const func_type& ft) {
         // Initialize the control stack with the current function as the sole element
         uint32_t operand_depth = 0;
         std::vector<pc_element_t> pc_stack{{
               operand_depth,
               ft.return_count?ft.return_type:static_cast<uint32_t>(types::pseudo),
               ft.return_count?ft.return_type:static_cast<uint32_t>(types::pseudo),
               false,
               std::vector<branch_t>{}}};

         // writes the continuation of a label to address.  If the continuation
         // is not yet available, address will be recorded in the relocations
         // list for label.
         auto handle_branch_target = [&](uint32_t label, branch_t address) {
            EOS_WB_ASSERT(label < pc_stack.size(), wasm_parse_exception, "invalid label");
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
            EOS_WB_ASSERT(label < pc_stack.size(), wasm_parse_exception, "invalid label");
            pc_element_t& branch_target = pc_stack[pc_stack.size() - label - 1];
            uint32_t result = operand_depth - branch_target.operand_depth;
            if(branch_target.label_result != types::pseudo) {
               // FIXME: Reusing the high bit imposes an additional constraint
               // on the maximum depth of the operand stack.  This isn't an
               // actual problem right now, because the stack is hard-coded
               // to 8192 elements, but it would be better to avoid spreading
               // this assumption around the code.
               result |= 0x80000000;
            }
            return result;
         };

         // Unconditional branches effectively make the state of the
         // stack unconstrained.  FIXME: Note that the unreachable instructions
         // still need to be validated, for consistency, which this impementation
         // fails to do.  Also, note that this variable is strictly for validation
         // purposes, and nested unreachable control structures have this reset
         // to "reachable," since their bodies get validated normally.
         bool is_in_unreachable = false;
         auto start_unreachable = [&]() {
            // We need enough room to push/pop any number of operands.
            operand_depth = 0x80000000;
            is_in_unreachable = true;
         };
         auto is_unreachable = [&]() -> bool {
            return is_in_unreachable;
         };
         auto start_reachable = [&]() { is_in_unreachable = false; };

         // Handles branches to the end of the scope and pops the pc_stack
         auto exit_scope = [&]() {
            if (pc_stack.size()) { // There must be at least one element
               auto end_pos = code_writer.emit_end();
               if(auto* relocations = std::get_if<std::vector<branch_t>>(&pc_stack.back().relocations)) {
                  for(auto branch_op : *relocations) {
                     code_writer.fix_branch(branch_op, end_pos);
                  }
               }
               unsigned expected_operand_depth = pc_stack.back().operand_depth;
               if (pc_stack.back().expected_result != types::pseudo) {
                  ++expected_operand_depth;
               }
               if (!is_unreachable())
                  EOS_WB_ASSERT(operand_depth == expected_operand_depth, wasm_parse_exception, "incorrect stack depth at end");
               operand_depth = expected_operand_depth;
               is_in_unreachable = pc_stack.back().is_unreachable;
               pc_stack.pop_back();
            } else {
               throw wasm_invalid_element{ "unexpected end instruction" };
            }
         };

         // Tracks the operand stack
         auto push_operand = [&](/* uint8_t type */) {
             EOS_WB_ASSERT(operand_depth < 0xFFFFFFFF, wasm_parse_exception, "Wasm stack overflow.");
             ++operand_depth;
         };
         auto pop_operand = [&]() {
            EOS_WB_ASSERT(operand_depth > 0, wasm_parse_exception, "Not enough items on the stack.");
            --operand_depth;
         };
         auto pop_operands = [&](uint32_t num_to_pop) {
            EOS_WB_ASSERT(operand_depth >= num_to_pop, wasm_parse_exception, "Not enough items on the stack.");
            operand_depth -= num_to_pop;
         };

         while (code.offset() < bounds) {
            EOS_WB_ASSERT(pc_stack.size() <= constants::max_nested_structures, wasm_parse_exception,
                          "nested structures validation failure");

            switch (*code++) {
               case opcodes::unreachable: code_writer.emit_unreachable(); start_unreachable(); break;
               case opcodes::nop: code_writer.emit_nop(); break;
               case opcodes::end: {
                  exit_scope();
                  break;
               }
               case opcodes::return_: {
                  uint32_t label = pc_stack.size() - 1;
                  auto branch = code_writer.emit_return(compute_depth_change(label));
                  handle_branch_target(label, branch);
                  start_unreachable();
               } break;
               case opcodes::block: {
                  uint32_t expected_result = *code++;
                  pc_stack.push_back({operand_depth, expected_result, expected_result, is_in_unreachable, std::vector<branch_t>{}});
                  code_writer.emit_block();
                  start_reachable();
               } break;
               case opcodes::loop: {
                  uint32_t expected_result = *code++;
                  auto pos = code_writer.emit_loop();
                  pc_stack.push_back({operand_depth, expected_result, types::pseudo, is_in_unreachable, pos});
                  start_reachable();
               } break;
               case opcodes::if_: {
                  uint32_t expected_result = *code++;
                  auto branch = code_writer.emit_if();
                  pop_operand();
                  pc_stack.push_back({operand_depth, expected_result, expected_result, is_in_unreachable, std::vector{branch}});
                  start_reachable();
               } break;
               case opcodes::else_: {
                  auto& old_index = pc_stack.back();
                  auto& relocations = std::get<std::vector<branch_t>>(old_index.relocations);
                  // reset the operand stack to the same state as the if
                  if (!is_unreachable()) {
                     EOS_WB_ASSERT((old_index.expected_result != types::pseudo) + old_index.operand_depth == operand_depth,
                                   wasm_parse_exception, "Malformed if body");
                  }
                  operand_depth = old_index.operand_depth;
                  start_reachable();
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
                  start_unreachable();
               } break;
               case opcodes::br_if: {
                  uint32_t label = parse_varuint32(code);
                  pop_operand();
                  auto branch = code_writer.emit_br_if(compute_depth_change(label));
                  handle_branch_target(label, branch);
               } break;
               case opcodes::br_table: {
                  size_t table_size = parse_varuint32(code);
                  pop_operand();
                  auto handler = code_writer.emit_br_table(table_size);
                  for (size_t i = 0; i < table_size; i++) {
                     uint32_t label = parse_varuint32(code);
                     auto branch = handler.emit_case(compute_depth_change(label));
                     handle_branch_target(label, branch);
                  }
                  uint32_t label = parse_varuint32(code);
                  auto branch = handler.emit_default(compute_depth_change(label));
                  handle_branch_target(label, branch);
                  start_unreachable();
               } break;
               case opcodes::call: {
                  uint32_t funcnum = parse_varuint32(code);
                  const func_type& ft = _mod->get_function_type(funcnum);
                  pop_operands(ft.param_types.size());
                  EOS_WB_ASSERT(ft.return_count <= 1, wasm_parse_exception, "unsupported");
                  if(ft.return_count == 1)
                     push_operand();
                  code_writer.emit_call(ft, funcnum);
               } break;
               case opcodes::call_indirect: {
                  uint32_t functypeidx = parse_varuint32(code);
                  const func_type& ft = _mod->types.at(functypeidx);
                  pop_operand();
                  pop_operands(ft.param_types.size());
                  EOS_WB_ASSERT(ft.return_count <= 1, wasm_parse_exception, "unsupported");
                  if(ft.return_count == 1)
                     push_operand();
                  code_writer.emit_call_indirect(ft, functypeidx);
                  code++; // 0x00
                  break;
               }
               case opcodes::drop: code_writer.emit_drop(); pop_operand(); break;
               case opcodes::select: code_writer.emit_select(); pop_operands(3); push_operand(); break;
               case opcodes::get_local: code_writer.emit_get_local( parse_varuint32(code) ); push_operand(); break;
               case opcodes::set_local: code_writer.emit_set_local( parse_varuint32(code) ); pop_operand(); break;
               case opcodes::tee_local: code_writer.emit_tee_local( parse_varuint32(code) ); break;
               case opcodes::get_global: code_writer.emit_get_global( parse_varuint32(code) ); push_operand(); break;
               case opcodes::set_global: code_writer.emit_set_global( parse_varuint32(code) ); pop_operand(); break;
#define LOAD_OP(op_name)                                             \
               case opcodes::op_name: {                              \
                  uint32_t alignment = parse_varuint32(code);        \
                  uint32_t offset = parse_varuint32(code);           \
                  code_writer.emit_ ## op_name( alignment, offset ); \
                  pop_operand();                                     \
                  push_operand();                                    \
               } break;

               LOAD_OP(i32_load)
               LOAD_OP(i64_load)
               LOAD_OP(f32_load)
               LOAD_OP(f64_load)
               LOAD_OP(i32_load8_s)
               LOAD_OP(i32_load16_s)
               LOAD_OP(i32_load8_u)
               LOAD_OP(i32_load16_u)
               LOAD_OP(i64_load8_s)
               LOAD_OP(i64_load16_s)
               LOAD_OP(i64_load32_s)
               LOAD_OP(i64_load8_u)
               LOAD_OP(i64_load16_u)
               LOAD_OP(i64_load32_u)

#undef LOAD_OP
                     
#define STORE_OP(op_name)                                            \
               case opcodes::op_name: {                              \
                  uint32_t alignment = parse_varuint32(code);        \
                  uint32_t offset = parse_varuint32(code);           \
                  code_writer.emit_ ## op_name( alignment, offset ); \
                  pop_operands(2);                                   \
               } break;

               STORE_OP(i32_store)
               STORE_OP(i64_store)
               STORE_OP(f32_store)
               STORE_OP(f64_store)
               STORE_OP(i32_store8)
               STORE_OP(i32_store16)
               STORE_OP(i64_store8)
               STORE_OP(i64_store16)
               STORE_OP(i64_store32)

#undef STORE_OP

               case opcodes::current_memory:
                  code_writer.emit_current_memory();
                  push_operand();
                  code++;
                  break;
               case opcodes::grow_memory:
                  code_writer.emit_grow_memory();
                  pop_operand();
                  push_operand();
                  code++;
                  break;
               case opcodes::i32_const: code_writer.emit_i32_const( parse_varint32(code) ); push_operand(); break;
               case opcodes::i64_const: code_writer.emit_i64_const( parse_varint64(code) ); push_operand(); break;
               case opcodes::f32_const: {
                  float value;
                  memcpy(&value, code.raw(), 4);
                  code_writer.emit_f32_const( value );
                  code += 4;
                  push_operand();
               } break;
               case opcodes::f64_const: {
                  double value;
                  memcpy(&value, code.raw(), 8);
                  code_writer.emit_f64_const( value );
                  code += 8;
                  push_operand();
               } break;

#define UNOP(opname) \
               case opcodes::opname: code_writer.emit_ ## opname(); pop_operand(); push_operand(); break;
#define BINOP(opname) \
               case opcodes::opname: code_writer.emit_ ## opname(); pop_operands(2); push_operand(); break;

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
               BINOP(f32_eq)
               BINOP(f32_ne)
               BINOP(f32_lt)
               BINOP(f32_gt)
               BINOP(f32_le)
               BINOP(f32_ge)
               BINOP(f64_eq)
               BINOP(f64_ne)
               BINOP(f64_lt)
               BINOP(f64_gt)
               BINOP(f64_le)
               BINOP(f64_ge)

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

               UNOP(i32_wrap_i64)
               UNOP(i32_trunc_s_f32)
               UNOP(i32_trunc_u_f32)
               UNOP(i32_trunc_s_f64)
               UNOP(i32_trunc_u_f64)
               UNOP(i64_extend_s_i32)
               UNOP(i64_extend_u_i32)
               UNOP(i64_trunc_s_f32)
               UNOP(i64_trunc_u_f32)
               UNOP(i64_trunc_s_f64)
               UNOP(i64_trunc_u_f64)
               UNOP(f32_convert_s_i32)
               UNOP(f32_convert_u_i32)
               UNOP(f32_convert_s_i64)
               UNOP(f32_convert_u_i64)
               UNOP(f32_demote_f64)
               UNOP(f64_convert_s_i32)
               UNOP(f64_convert_u_i32)
               UNOP(f64_convert_s_i64)
               UNOP(f64_convert_u_i64)
               UNOP(f64_promote_f32)
               UNOP(i32_reinterpret_f32)
               UNOP(i64_reinterpret_f64)
               UNOP(f32_reinterpret_i32)
               UNOP(f64_reinterpret_i64)

#undef UNOP
#undef BINOP
               case opcodes::error: code_writer.emit_error(); break;
            }
         }
      }

      void parse_data_segment(wasm_code_ptr& code, data_segment& ds) {
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
         EOS_WB_ASSERT(_id == id, wasm_parse_exception, "Section id does not match");
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
         parse_section_impl(code, elems, [&](wasm_code_ptr& code, memory_type& mt, std::size_t /*idx*/) { parse_memory_type(code, mt); });
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
         Writer code_writer(_allocator, code.bounds() - code.offset(), *_mod);
         parse_section_impl(code, elems,
                            [&](wasm_code_ptr& code, function_body& fb, std::size_t idx) { parse_function_body(code, fb, idx, code_writer); });
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
   };
}} // namespace eosio::vm
