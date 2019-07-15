#pragma once

#include <eosio/vm/allocator.hpp>
#include <eosio/vm/types.hpp>

#include <softfloat.hpp>

namespace eosio { namespace vm {



   // Random notes:
   // - branch instructions return the address that will need to be updated
   // - label instructions return the address of the target
   // - fix_branch will be called when the branch target is resolved
   // - It would make everything more efficient to make RAX always represent the top of
   //   the stack.
   //
   // - The base of memory is stored in rsi
   //
   // - FIXME: Factor the machine instructions into a separate assembler class.
   // - FIXME: The top level entry point needs to set the floating point state
   class machine_code_writer {
    public:
      machine_code_writer(jit_allocator& alloc, std::size_t source_bytes, uint32_t funcnum, module& mod) :
         _mod(mod),
         _allocator(alloc),
         _ft(mod.get_function_type(funcnum)) {
         code = _allocator.alloc(source_bytes * 32 + 128); // FIXME: Guess at upper bound on function size
         start_function(code, funcnum);
      }

      // FIXME: initialize locals
      void emit_prologue(const func_type& ft, const guarded_vector<local_entry>& locals) {
         // pushq RBP
         emit_bytes(0x55);
         // movq RSP, RBP
         emit_bytes(0x48, 0x89, 0xe5);
         // xor %rax, %rax
         emit_bytes(0x48, 0x31, 0xc0);
         for(std::size_t i = 0; i < locals.size(); ++i) {
            for(uint32_t j = 0; j < locals[i].count; ++j)
               // pushq %rax
               emit_bytes(0x50);
         }
      }
      void emit_epilogue(const func_type& ft, const guarded_vector<local_entry>& locals) {
         if(ft.return_count != 0) {
            // pop RAX
            emit_bytes(0x58);
         }
         uint32_t count = 0;
         for(std::size_t i = 0; i < locals.size(); ++i) {
            count += locals[i].count;
         }
         emit_multipop(count);
         // popq RBP
         emit_bytes(0x5d);
         // retq
         emit_bytes(0xc3);
      }
      
      void emit_unreachable() { /* FIXME: emit a trap */ }
      void emit_nop() {}
      void* emit_end() { return code; }
      void* emit_return(uint32_t depth_change) {
         // Return is defined as equivalent to branching to the outermost label
         return emit_br(depth_change);
      }
      void emit_block() {}
      void* emit_loop() { return code; }
      void* emit_if() {
         // pop RAX
         emit_bytes(0x58);
         // test EAX, EAX
         emit_bytes(0x85, 0xC0);
         // jnz DEST
         emit_bytes(0x0F, 0x85);
         return emit_branch_target32();
      }
      void* emit_else(void* if_loc) {
         void* result = emit_br(0);
         fix_branch(if_loc, code);
         return result;
      }
      void* emit_br(uint32_t depth_change) {
         // add RSP, depth_change * 8
         emit_multipop(depth_change);
         // jmp DEST
         emit_bytes(0xe9);
         return emit_branch_target32();
      }
      void* emit_br_if(uint32_t depth_change) {
         // pop RAX
         emit_bytes(0x58);
         // add RSP, depth_change*8
         if(depth_change != 0) unimplemented();
         emit_multipop(depth_change); // doesn't work
         // test EAX, EAX
         emit_bytes(0x85, 0xC0);
         // jnz DEST
         emit_bytes(0x0F, 0x85);
         return emit_branch_target32();
      }

      // FIXME This just does a linear search.
      struct br_table_generator {
         void* emit_case(uint32_t depth_change) {
            // cmp i, %eax
            _this->emit_bytes(0x3d);
            _this->emit_operand32(_i++);
            // jne NEXT
            _this->emit_bytes(0x0f, 0x85);
            void* internal_branch = _this->code;
            _this->emit_operand32(0);
            _this->emit_multipop(depth_change);
            // jmp TARGET
            _this->emit_bytes(0xe9);
            void* branch = _this->emit_branch_target32();
            _this->fix_branch(internal_branch, _this->code);
            return branch;
         }
         void* emit_default(uint32_t depth_change) {
            return _this->emit_br(depth_change);
         }
         machine_code_writer * _this;
         int _i = 0;
      };
      br_table_generator emit_br_table(uint32_t table_size) {
         // pop %rax
         emit_bytes(0x58);
         return { this };
      }

      // HACK: Don't use global variables
      static auto& call_singleton() {
         static std::vector<std::variant<std::vector<void*>, void*>> functions;
         return functions;
      }
      static void register_call(void* ptr, uint32_t funcnum) {
         auto& vec = call_singleton();
         if(funcnum >= vec.size()) vec.resize(funcnum + 1);
         if(void** addr = std::get_if<void*>(&vec[funcnum])) {
            fix_branch(ptr, *addr);
         } else {
            std::get<std::vector<void*>>(vec[funcnum]).push_back(ptr);
         }
      }
      static void start_function(void* func_start, uint32_t funcnum) {
         auto& vec = call_singleton();
         if(funcnum >= vec.size()) vec.resize(funcnum + 1);
         for(void* branch : std::get<std::vector<void*>>(vec[funcnum])) {
            fix_branch(branch, func_start);
         }
         vec[funcnum] = func_start;
      }

      void emit_call(const func_type& ft, uint32_t funcnum) {
         if(is_host_function(funcnum)) {
            call_host_function(ft, funcnum);
         } else {
            // callq TARGET
            emit_bytes(0xe8);
            void * branch = emit_branch_target32();
#if 0
            // movabsq TARGET, %rax
            emit_bytes(0x48, 0xa1);
            void * branch = code;
            emit_operand64(3735928559u);
            // callq %rax
            emit_bytes(0xff, 0xd0);
#endif
            emit_multipop(ft.param_types.size());
            register_call(branch, funcnum);
            if(ft.return_count != 0)
               // pushq %rax
               emit_bytes(0x50);
         }
      }

      void emit_call_indirect(const func_type& ft, uint32_t functypeidx) {
         unimplemented();
         // check function type
         // insert branch through table 0
      }

      void emit_drop() {
         // pop RAX
         emit_bytes(0x58);
      }

      void emit_select() {
         // popq RAX
         emit_bytes(0x58);
         // popq RCX
         emit_bytes(0x59);
         // test EAX, EAX
         emit_bytes(0x85, 0xc0);
         // cmovnzq RCX, (RSP)
         emit_bytes(0x48, 0x0f, 0x45, 0x0c, 0x24);
         // movq (RSP), RCX
         emit_bytes(0x48, 0x89, 0x0c, 0x24);
      }

      void emit_get_local(uint32_t local_idx) {
         // stack layout:
         //   param0    <----- %rbp + 8*(nparams + 1)
         //   param1
         //   param2
         //   ...
         //   paramN
         //   return address
         //   old %rbp    <------ %rbp
         //   local0    <------ %rbp - 8
         //   local1
         //   ...
         //   localN
         if (local_idx < _ft.param_types.size()) {
            // mov 8*(local_idx)(%RBP), RAX
            emit_bytes(0x48, 0x8b, 0x85);
            emit_operand32(8 * (_ft.param_types.size() - local_idx + 1));
            // push RAX
            emit_bytes(0x50);
         } else {
            // mov -8*(local_idx+1)(%RBP), RAX
            emit_bytes(0x48, 0x8b, 0x85);
            emit_operand32(-8 * (local_idx - _ft.param_types.size() + 1));
            // push RAX
            emit_bytes(0x50);
         }
      }

      void emit_set_local(uint32_t local_idx) {
         if (local_idx < _ft.param_types.size()) {
            // pop RAX
            emit_bytes(0x58);
            // mov RAX, -8*local_idx(EBP)
            emit_bytes(0x48, 0x89, 0x85);
            emit_operand32(8 * (_ft.param_types.size() - local_idx + 1));
         } else {
            // pop RAX
            emit_bytes(0x58);
            // mov RAX, -8*local_idx(EBP)
            emit_bytes(0x48, 0x89, 0x85);
            emit_operand32(-8 * (local_idx - _ft.param_types.size() + 1));
         }
      }

      void emit_tee_local(uint32_t local_idx) {
         if (local_idx < _ft.param_types.size()) {
            // pop RAX
            emit_bytes(0x58);
            // push RAX
            emit_bytes(0x50);
            // mov RAX, -8*local_idx(EBP)
            emit_bytes(0x48, 0x89, 0x85);
            emit_operand32(8 * (_ft.param_types.size() - local_idx + 1));
         } else {
            // pop RAX
            emit_bytes(0x58);
            // push RAX
            emit_bytes(0x50);
            // mov RAX, -8*local_idx(EBP)
            emit_bytes(0x48, 0x89, 0x85);
            emit_operand32(-8 * (local_idx - _ft.param_types.size() + 1));
         }
      }

      void emit_get_global(uint32_t globalidx) {
         auto& gl = _mod.globals[globalidx];
         void *ptr = &gl.init.value;
         switch(gl.type.content_type) {
          case types::i32:
          case types::f32:
            // movabsq $ptr, %rax
            emit_bytes(0x48, 0xb8);
            emit_operand_ptr(ptr);
            // movl (%rax), eax
            emit_bytes(0x8b, 0x00);
            // push %rax
            emit_bytes(0x50);
            break;
          case types::i64:
          case types::f64:
            // movabsq $ptr, %rax
            emit_bytes(0x48, 0xb8);
            emit_operand_ptr(ptr);
            // movl (%rax), %rax
            emit_bytes(0x48, 0x8b, 0x00);
            // push %rax
            emit_bytes(0x50);
            break;
         }
      }
      void emit_set_global(uint32_t globalidx) {
         auto& gl = _mod.globals[globalidx];
         void *ptr = &gl.init.value;
         // popq %rcx
         emit_bytes(0x59);
         // movabsq $ptr, %rax
         emit_bytes(0x48, 0xb8);
         emit_operand_ptr(ptr);
         // movq %rcx, (%rax)
         emit_bytes(0x48, 0x89, 0x08);
      }

      void emit_i32_load(uint32_t alignment, uint32_t offset) {
         // movl (RAX), EAX
         emit_load_impl(offset, 0x8b, 0x00);
      }

      void emit_i64_load(uint32_t alignment, uint32_t offset) {
         // movq (RAX), RAX
         emit_load_impl(offset, 0x48, 0x8b, 0x00);
      }

      void emit_f32_load(uint32_t alignment, uint32_t offset) {
         // movl (RAX), EAX
         emit_load_impl(offset, 0x8b, 0x00);
      }

      void emit_f64_load(uint32_t alignment, uint32_t offset) {
         // movq (RAX), RAX
         emit_load_impl(offset, 0x48, 0x8b, 0x00);
      }

      void emit_i32_load8_s(uint32_t alignment, uint32_t offset) {
         // movsbl (RAX), EAX; 
         emit_load_impl(offset, 0x0F, 0xbe, 0x00);
      }

      void emit_i32_load16_s(uint32_t alignment, uint32_t offset) {
         // movswl (RAX), EAX; 
         emit_load_impl(offset, 0x0F, 0xbf, 0x00);
      }

      void emit_i32_load8_u(uint32_t alignment, uint32_t offset) {
         // movzbl (RAX), EAX; 
         emit_load_impl(offset, 0x0f, 0xb6, 0x00);
      }

      void emit_i32_load16_u(uint32_t alignment, uint32_t offset) {
         // movzwl (RAX), EAX; 
         emit_load_impl(offset, 0x0f, 0xb7, 0x00);
      }

      void emit_i64_load8_s(uint32_t alignment, uint32_t offset) {
         // movsbq (RAX), RAX; 
         emit_load_impl(offset, 0x48, 0x0F, 0xbe, 0x00);
      }

      void emit_i64_load16_s(uint32_t alignment, uint32_t offset) {
         // movswq (RAX), RAX; 
         emit_load_impl(offset, 0x48, 0x0F, 0xbf, 0x00);
      }

      void emit_i64_load32_s(uint32_t alignment, uint32_t offset) {
         // movslq (RAX), RAX
         emit_load_impl(offset, 0x48, 0x63, 0x00);
      }

      void emit_i64_load8_u(uint32_t alignment, uint32_t offset) {
         // movzbl (RAX), EAX; 
         emit_load_impl(offset, 0x0f, 0xb6, 0x00);
      }

      void emit_i64_load16_u(uint32_t alignment, uint32_t offset) {
         // movzwl (RAX), EAX; 
         emit_load_impl(offset, 0x0f, 0xb7, 0x00);
      }

      void emit_i64_load32_u(uint32_t alignment, uint32_t offset) {
         // movl (RAX), EAX
         emit_load_impl(offset, 0x8b, 0x00);
      }

      void emit_i32_store(uint32_t alignment, uint32_t offset) {
         // movl ECX, (RAX)
         emit_store_impl(offset, 0x89, 0x08);
      }

      void emit_i64_store(uint32_t alignment, uint32_t offset) {
         // movl ECX, (RAX)
         emit_store_impl(offset, 0x48, 0x89, 0x08);
      }

      void emit_f32_store(uint32_t alignment, uint32_t offset) {
         // movl ECX, (RAX)
         emit_store_impl(offset, 0x89, 0x08);
      }

      void emit_f64_store(uint32_t alignment, uint32_t offset) {
         // movl ECX, (RAX)
         emit_store_impl(offset, 0x48, 0x89, 0x08);
      }

      void emit_i32_store8(uint32_t alignment, uint32_t offset) {
         // movb CL, (RAX)
         emit_store_impl(offset, 0x88, 0x08);
      }

      void emit_i32_store16(uint32_t alignment, uint32_t offset) {
         // movb CX, (RAX)
         emit_store_impl(offset, 0x66, 0x89, 0x08);
      }

      void emit_i64_store8(uint32_t alignment, uint32_t offset) {
         // movb CL, (RAX)
         emit_store_impl(offset, 0x88, 0x08);
      }

      void emit_i64_store16(uint32_t alignment, uint32_t offset) {
         // movb CX, (RAX)
         emit_store_impl(offset, 0x66, 0x89, 0x08);
      }

      void emit_i64_store32(uint32_t alignment, uint32_t offset) {
         // movl ECX, (RAX)
         emit_store_impl(offset, 0x89, 0x08);
      }

      void emit_current_memory() { unimplemented(); }
      void emit_grow_memory() { unimplemented(); }

      void emit_i32_const(uint32_t value) {
         // mov value, %eax
         emit_bytes(0xb8);
         emit_operand32(value);
         // push %rax
         emit_bytes(0x50);
      }

      void emit_i64_const(uint64_t value) {
         // movabsq value, %rax
         emit_bytes(0x48, 0xa1);
         emit_operand64(value);
         // push %rax
         emit_bytes(0x50);
      }

      void emit_f32_const(float value) { 
         // mov value, %eax
         emit_bytes(0x8b, 0x04, 0x25);
         emit_operandf32(value);
         // push %rax
         emit_bytes(0x50);
      }
      void emit_f64_const(double value) {
         // movabsq value, %rax
         emit_bytes(0x48, 0xa1);
         emit_operandf64(value);
         // push %rax
         emit_bytes(0x50);
      }

      void emit_i32_eqz() {
         // pop %rax
         emit_bytes(0x58);
         // xor %rcx, %rcx
         emit_bytes(0x48, 0x31, 0xc9);
         // test %eax, %eax
         emit_bytes(0x85, 0xc0);
         // setz %cl
         emit_bytes(0x0f, 0x94, 0xc1);
         // push %rcx
         emit_bytes(0x51);
      }

      // i32 relops
      void emit_i32_eq() {
         // sete %dl
         emit_i32_relop(0x94);
      }

      void emit_i32_ne() {
         // sete %dl
         emit_i32_relop(0x95);
      }

      void emit_i32_lt_s() {
         // setl %dl
         emit_i32_relop(0x9c);
      }

      void emit_i32_lt_u() {
         // setl %dl
         emit_i32_relop(0x92);
      }

      void emit_i32_gt_s() {
         // setg %dl
         emit_i32_relop(0x9f);
      }

      void emit_i32_gt_u() {
         // seta %dl
         emit_i32_relop(0x97);
      }

      void emit_i32_le_s() {
         // setle %dl
         emit_i32_relop(0x9e);
      }

      void emit_i32_le_u() {
         // setbe %dl
         emit_i32_relop(0x96);
      }

      void emit_i32_ge_s() {
         // setge %dl
         emit_i32_relop(0x9d);
      }

      void emit_i32_ge_u() {
         // setae %dl
         emit_i32_relop(0x93);
      }

      void emit_i64_eqz() {
         // pop %rax
         emit_bytes(0x58);
         // xor %rcx, %rcx
         emit_bytes(0x48, 0x31, 0xc9);
         // test %rax, %rax
         emit_bytes(0x48, 0x85, 0xc0);
         // setz %cl
         emit_bytes(0x0f, 0x94, 0xc1);
         // push %rcx
         emit_bytes(0x51);
      }
      // i64 relops
      void emit_i64_eq() {
         // sete %dl
         emit_i64_relop(0x94);
      }

      void emit_i64_ne() {
         // sete %dl
         emit_i64_relop(0x95);
      }

      void emit_i64_lt_s() {
         // setl %dl
         emit_i64_relop(0x9c);
      }

      void emit_i64_lt_u() {
         // setl %dl
         emit_i64_relop(0x92);
      }

      void emit_i64_gt_s() {
         // setg %dl
         emit_i64_relop(0x9f);
      }

      void emit_i64_gt_u() {
         // seta %dl
         emit_i64_relop(0x97);
      }

      void emit_i64_le_s() {
         // setle %dl
         emit_i64_relop(0x9e);
      }

      void emit_i64_le_u() {
         // setbe %dl
         emit_i64_relop(0x96);
      }

      void emit_i64_ge_s() {
         // setge %dl
         emit_i64_relop(0x9d);
      }

      void emit_i64_ge_u() {
         // setae %dl
         emit_i64_relop(0x93);
      }

      // HACK:
      template<auto F>
      constexpr auto choose_fn() {
         if constexpr (use_softfloat) {
            return F;
         } else {
            return nullptr;
         }
      }

      // --------------- f32 relops ----------------------
      void emit_f32_eq() {
         emit_f32_relop(0x00, choose_fn<&::f32_eq>(), false, false);
      }

      void emit_f32_ne() {
         emit_f32_relop(0x00, choose_fn<&::f32_eq>(), false, true);
      }

      void emit_f32_lt() {
         emit_f32_relop(0x01, choose_fn<&::f32_lt>(), false, false);
      }

      void emit_f32_gt() {
         emit_f32_relop(0x01, choose_fn<&::f32_lt>(), true, false);
      }

      void emit_f32_le() {
         emit_f32_relop(0x02, choose_fn<&::f32_le>(), false, false);
      }

      void emit_f32_ge() {
         emit_f32_relop(0x02, choose_fn<&::f32_le>(), true, false);
      }

      // --------------- f64 relops ----------------------
      void emit_f64_eq() {
         emit_f64_relop(0x00, choose_fn<&::f64_eq>(), false, false);
      }

      void emit_f64_ne() {
         emit_f64_relop(0x00, choose_fn<&::f64_eq>(), false, true);
      }

      void emit_f64_lt() {
         emit_f64_relop(0x01, choose_fn<&::f64_lt>(), false, false);
      }

      void emit_f64_gt() {
         emit_f64_relop(0x01, choose_fn<&::f64_lt>(), true, false);
      }

      void emit_f64_le() {
         emit_f64_relop(0x02, choose_fn<&::f64_le>(), false, false);
      }

      void emit_f64_ge() {
         emit_f64_relop(0x02, choose_fn<&::f64_le>(), true, false);
      }

      // --------------- i32 unops ----------------------

      // FIXME: detect whether lzcnt/tzcnt are supported
      void emit_i32_clz() {
         // popq %rax
         emit_bytes(0x58);
         // lzcntl %eax, %eax
         emit_bytes(0xf3, 0x0f, 0xbd, 0xc0);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_i32_ctz() {
         // popq %rax
         emit_bytes(0x58);
         // tzcntl %eax, %eax
         emit_bytes(0xf3, 0x0f, 0xbc, 0xc0);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_i32_popcnt() {
         // popq %rax
         emit_bytes(0x58);
         // popcntl %eax, %eax
         emit_bytes(0xf3, 0x0f, 0xb8, 0xc0);
         // pushq %rax
         emit_bytes(0x50);
      }

      // --------------- i32 binops ----------------------

      void emit_i32_add() { emit_i32_binop(0x01, 0xc8, 0x50); }
      void emit_i32_sub() { emit_i32_binop(0x29, 0xc8, 0x50); }
      void emit_i32_mul() { emit_i32_binop(0x0f, 0xaf, 0xc1, 0x50); }
      void emit_i32_div_s() { emit_i32_binop(0x31, 0xd2, 0xf7, 0xf9, 0x50); }
      void emit_i32_div_u() { emit_i32_binop(0x31, 0xd2, 0xf7, 0xf1, 0x50); }
      void emit_i32_rem_s() { emit_i32_binop(0x31, 0xd2, 0xf7, 0xf9, 0x52); }
      void emit_i32_rem_u() { emit_i32_binop(0x31, 0xd2, 0xf7, 0xf1, 0x52); }
      void emit_i32_and() { emit_i32_binop(0x21, 0xc8, 0x50); }
      void emit_i32_or() { emit_i32_binop(0x09, 0xc8, 0x50); }
      void emit_i32_xor() { emit_i32_binop(0x31, 0xc8, 0x50); }
      void emit_i32_shl() { emit_i32_binop(0xd3, 0xe0, 0x50); }
      void emit_i32_shr_s() { emit_i32_binop(0xd3, 0xf8, 0x50); }
      void emit_i32_shr_u() { emit_i32_binop(0xd3, 0xe8, 0x50); }
      void emit_i32_rotl() { emit_i32_binop(0xd3, 0xc0, 0x50); }
      void emit_i32_rotr() { emit_i32_binop(0xd3, 0xc8, 0x50); }

      // --------------- i64 unops ----------------------

      // FIXME: detect whether lzcnt/tzcnt are supported
      void emit_i64_clz() {
         // popq %rax
         emit_bytes(0x58);
         // lzcntl %eax, %eax
         emit_bytes(0x48, 0xf3, 0x0f, 0xbd, 0xc0);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_i64_ctz() {
         // popq %rax
         emit_bytes(0x58);
         // tzcntl %eax, %eax
         emit_bytes(0x48, 0xf3, 0x0f, 0xbc, 0xc0);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_i64_popcnt() {
         // popq %rax
         emit_bytes(0x58);
         // popcntl %eax, %eax
         emit_bytes(0x48, 0xf3, 0x0f, 0xb8, 0xc0);
         // pushq %rax
         emit_bytes(0x50);
      }

      // --------------- i64 binops ----------------------

      void emit_i64_add() { emit_i64_binop(0x48, 0x01, 0xc8, 0x50); }
      void emit_i64_sub() { emit_i64_binop(0x48, 0x29, 0xc8, 0x50); }
      void emit_i64_mul() { emit_i64_binop(0x48, 0x0f, 0xaf, 0xc1, 0x50); }
      void emit_i64_div_s() { emit_i64_binop(0x48, 0x31, 0xd2, 0x48, 0xf7, 0xf9, 0x50); }
      void emit_i64_div_u() { emit_i64_binop(0x48, 0x31, 0xd2, 0x48, 0xf7, 0xf1, 0x50); }
      void emit_i64_rem_s() { emit_i64_binop(0x48, 0x31, 0xd2, 0x48, 0xf7, 0xf9, 0x52); }
      void emit_i64_rem_u() { emit_i64_binop(0x48, 0x31, 0xd2, 0x48, 0xf7, 0xf1, 0x52); }
      void emit_i64_and() { emit_i64_binop(0x48, 0x21, 0xc8, 0x50); }
      void emit_i64_or() { emit_i64_binop(0x48, 0x09, 0xc8, 0x50); }
      void emit_i64_xor() { emit_i64_binop(0x48, 0x31, 0xc8, 0x50); }
      void emit_i64_shl() { emit_i64_binop(0x48, 0xd3, 0xe0, 0x50); }
      void emit_i64_shr_s() { emit_i64_binop(0x48, 0xd3, 0xf8, 0x50); }
      void emit_i64_shr_u() { emit_i64_binop(0x48, 0xd3, 0xe8, 0x50); }
      void emit_i64_rotl() { emit_i64_binop(0x48, 0xd3, 0xc0, 0x50); }
      void emit_i64_rotr() { emit_i64_binop(0x48, 0xd3, 0xc8, 0x50); }

      // --------------- f32 unops ----------------------

      void emit_f32_abs() {
         // popq %rax; 
         emit_bytes(0x58);
         // movl 0x7fffffff, %ecx
         emit_bytes(0x8b, 0x0c, 0x25);
         emit_operand32(0x7fffffff);
         // andl %ecx, %eax
         emit_bytes(0x21, 0xc8);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_f32_neg() {
         // FIXME: use btc
         // popq %rax; 
         emit_bytes(0x58);
         // movl 0x80000000, %ecx
         emit_bytes(0x8b, 0x0c, 0x25);
         emit_operand32(0x80000000);
         // xorl %ecx, %eax
         emit_bytes(0x31, 0xc8);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_f32_ceil() {
         // roundss 0b1010, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0a, 0x01, 0x24, 0x0a);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f32_floor() {
         // roundss 0b1001, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0a, 0x01, 0x24, 0x09);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f32_trunc() {
         // roundss 0b1011, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0a, 0x01, 0x24, 0x0b);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f32_nearest() {
         // roundss 0b1000, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0a, 0x01, 0x24, 0x08);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f32_sqrt() {
         // sqrtss (%rsp), %xmm0
         emit_bytes(0xf3, 0x0f, 0x51, 0x04, 0x24);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }

      // --------------- f32 binops ----------------------

      void emit_f32_add() { emit_f32_binop(0x58); }
      void emit_f32_sub() { emit_f32_binop(0x5c); }
      void emit_f32_mul() { emit_f32_binop(0x59); }
      void emit_f32_div() { emit_f32_binop(0x5e); }
      void emit_f32_min() { emit_f32_binop(0x5d); }
      void emit_f32_max() { emit_f32_binop(0x5f); }

      void emit_f32_copysign() {
         // popq %rax; 
         emit_bytes(0x58);
         // movl 0x80000000, %ecx
         emit_bytes(0x8b, 0x0c, 0x25);
         emit_operand32(0x80000000);
         // andl %ecx, %eax
         emit_bytes(0x21, 0xc8);
         // popq %rdx
         emit_bytes(0x5a);
         // notl %ecx
         emit_bytes(0xf7, 0xd1);
         // andl %edx, %ecx
         emit_bytes(0x21, 0xd1);
         // orl %ecx, %eax
         emit_bytes(0x09, 0xc8);
         // pushq %rax
         emit_bytes(0x50);
      }
      
      // --------------- f64 unops ----------------------

      void emit_f64_abs() {
         // popq %rcx; 
         emit_bytes(0x59);
         // movabsq 0x7fffffffffffffff, %rax
         emit_bytes(0x48, 0xa1);
         emit_operand64(0x7fffffffffffffffull);
         // andq %rcx, %rax
         emit_bytes(0x48, 0x21, 0xc8);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_f64_neg() {
         // FIXME: use btc
         // popq %rcx; 
         emit_bytes(0x59);
         // movabsq 0x8000000000000000, %rax
         emit_bytes(0x48, 0xa1);
         emit_operand64(0x8000000000000000ull);
         // xorq %rcx, %rax
         emit_bytes(0x48, 0x31, 0xc8);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_f64_ceil() {
         // roundsd 0b1010, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0b, 0x01, 0x24, 0x0a);
         // movsd %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f64_floor() {
         // roundss 0b1001, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0b, 0x01, 0x24, 0x09);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f64_trunc() {
         // roundss 0b1011, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0b, 0x01, 0x24, 0x0b);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f64_nearest() {
         // roundss 0b1000, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0b, 0x01, 0x24, 0x08);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f64_sqrt() {
         // sqrtss (%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x51, 0x04, 0x24);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }

      // --------------- f64 binops ----------------------

      void emit_f64_add() { emit_f64_binop(0x58); }
      void emit_f64_sub() { emit_f64_binop(0x5c); }
      void emit_f64_mul() { emit_f64_binop(0x59); }
      void emit_f64_div() { emit_f64_binop(0x5e); }
      void emit_f64_min() { emit_f64_binop(0x5d); }
      void emit_f64_max() { emit_f64_binop(0x5f); }

      void emit_f64_copysign() {
         // popq %rcx; 
         emit_bytes(0x59);
         // movabsq 0x8000000000000000, %rax
         emit_bytes(0x48, 0xa1);
         emit_operand64(0x8000000000000000ull);
         // andq %rax, %rcx
         emit_bytes(0x48, 0x21, 0xc1);
         // popq %rdx
         emit_bytes(0x5a);
         // notq %rax
         emit_bytes(0x48, 0xf7, 0xd0);
         // andq %rdx, %rax
         emit_bytes(0x48, 0x21, 0xd0);
         // orq %rcx, %rax
         emit_bytes(0x48, 0x09, 0xc8);
         // pushq %rax
         emit_bytes(0x50);
      }

      // --------------- conversions --------------------


      void emit_i32_wrap_i64() {
         // Zero out the high 4 bytes
         // xor %eax, %eax
         emit_bytes(0x31, 0xc0);
         // mov %eax, 4(%rsp)
         emit_bytes(0x89, 0x44, 0x24, 0x04);
      }

      void emit_i32_trunc_s_f32() {
         // cvttss2si 8(%rsp), %eax
         emit_f2i(0xf3, 0x0f, 0x2c, 0x44, 0x24, 0x08);
         // mov %eax, (%rsp)
         emit_bytes(0x89, 0x04 ,0x24);
      }

      void emit_i32_trunc_u_f32() {
         // cvttss2si 8(%rsp), %rax
         emit_f2i(0xf3, 0x48, 0x0f, 0x2c, 0x44, 0x24, 0x08);
         // mov %eax, (%rsp)
         emit_bytes(0x89, 0x04 ,0x24);
         // shl $32, %rax
         emit_bytes(0x48, 0xc1, 0xe0, 0x20);
         // test %eax, %eax
         emit_bytes(0x85, 0xc0);
         // jnz FP_ERROR_HANDLER
         emit_bytes(0x0f, 0x85);
         void * addr = emit_branch_target32();
         // FIXME: adjust branch
      }
      void emit_i32_trunc_s_f64() {
         // cvttsd2si 8(%rsp), %eax
         emit_f2i(0xf2, 0x0f, 0x2c, 0x44, 0x24, 0x08);
         // mov %eax, (%rsp)
         emit_bytes(0x89, 0x04 ,0x24);
      }

      void emit_i32_trunc_u_f64() {
         // cvttss2si 8(%rsp), %rax
         emit_f2i(0xf3, 0x48, 0x0f, 0x2c, 0x44, 0x24, 0x08);
         // mov %eax, (%rsp)
         emit_bytes(0x89, 0x04 ,0x24);
         // shl $32, %rax
         emit_bytes(0x48, 0xc1, 0xe0, 0x20);
         // test %eax, %eax
         emit_bytes(0x85, 0xc0);
         // jnz FP_ERROR_HANDLER
         emit_bytes(0x0f, 0x85);
         void * addr = emit_branch_target32();
         // FIXME: adjust branch
      }

      void emit_i64_extend_s_i32() {
         // movslq (%rsp), %rax
         emit_bytes(0x48, 0x63, 0x04, 0x24);
         // mov %rax, (%rsp)
         emit_bytes(0x48, 0x89, 0x04, 0x24);
      }

      void emit_i64_extend_u_i32() { /* Nothing to do */ }
      
      void emit_i64_trunc_s_f32() {
         // cvttss2si (%rsp), %rax
         emit_f2i(0xf3, 0x48, 0x0f, 0x2c, 0x44, 0x24, 0x08);
         // mov %rax, (%rsp)
         emit_bytes(0x48, 0x89, 0x04 ,0x24);
      }
      void emit_i64_trunc_u_f32() {
        // movss -2**63, %xmm0
        // addss (%rsp), %xmm0
        // cvtss2si (%rsp), %rax
        // btc $63, %rax
        // 
        // FIXME:
        unimplemented();
      }
      void emit_i64_trunc_s_f64() {
         // cvttsd2si (%rsp), %rax
         emit_f2i(0xf2, 0x48, 0x0f, 0x2c, 0x44, 0x24, 0x08);
         // mov %rax, (%rsp)
         emit_bytes(0x48, 0x89, 0x04 ,0x24);
      }
      void emit_i64_trunc_u_f64() { unimplemented(); }

      void emit_f32_convert_s_i32() {
         // cvtsi2ssl (%rsp), %xmm0
         emit_bytes(0xf3, 0x0f, 0x2a, 0x04, 0x24);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }
      void emit_f32_convert_u_i32() {
         // zero-extend to 64-bits
         // cvtsi2sslq (%rsp), %xmm0
         emit_bytes(0xf3, 0x48, 0x0f, 0x2a, 0x04, 0x24);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }
      void emit_f32_convert_s_i64() {
         // cvtsi2sslq (%rsp), %xmm0
         emit_bytes(0xf3, 0x48, 0x0f, 0x2a, 0x04, 0x24);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }
      void emit_f32_convert_u_i64() { unimplemented(); }
      void emit_f32_demote_f64() {
         // cvtsd2ss (%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x5a, 0x04, 0x24);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }
      void emit_f64_convert_s_i32() {
         //  cvtsi2sdl (%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x2a, 0x04, 0x24);
         // movsd %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }
      void emit_f64_convert_u_i32() {
         //  cvtsi2sdq (%rsp), %xmm0
         emit_bytes(0xf2, 0x48, 0x0f, 0x2a, 0x04, 0x24);
         // movsd %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }
      void emit_f64_convert_s_i64() {
         //  cvtsi2sdq (%rsp), %xmm0
         emit_bytes(0xf2, 0x48, 0x0f, 0x2a, 0x04, 0x24);
         // movsd %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }
      void emit_f64_convert_u_i64() { unimplemented(); }
      void emit_f64_promote_f32() {
         // cvtss2sd (%rsp), %xmm0
         emit_bytes(0xf3, 0x0f, 0x5a, 0x04, 0x24);
         // movsd %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }
      
      void emit_i32_reinterpret_f32() { /* Nothing to do */ }
      void emit_i64_reinterpret_f64() { /* Nothing to do */ }
      void emit_f32_reinterpret_i32() { /* Nothing to do */ }
      void emit_f64_reinterpret_i64() { /* Nothing to do */ }

      void emit_error() { unimplemented(); }

      // --------------- random  ------------------------
      static void fix_branch(void* branch, void* target) {
         auto branch_ = static_cast<uint8_t*>(branch);
         auto target_ = static_cast<uint8_t*>(target);
         auto relative = static_cast<uint32_t>(target_ - (branch_ + 4));
         if((target_ - (branch_ + 4)) > 0x7FFFFFFFll ||
            (target_ - (branch_ + 4)) < -0x80000000ll) unimplemented();
         memcpy(branch, &relative, 4);
      }

      // A 64-bit absolute address is used for function calls whose
      // address is too far away for a 32-bit relative call.
      static void fix_branch64(void* branch, void* target) {
         memcpy(branch, &target, 8);
      }

      using fn_type = uint64_t(*)(void* context, void* memory);
      void finalize(function_body& body) {
         body.jit_code = reinterpret_cast<fn_type>(_allocator.setpos(code));
      }
    private:

      module& _mod;
      jit_allocator& _allocator;
      const func_type& _ft;
      unsigned char * code;
      void emit_byte(uint8_t val) { *code++ = val; }
      void emit_bytes() {}
      template<class... T>
      void emit_bytes(uint8_t val0, T... vals) {
         emit_byte(val0);
         emit_bytes(vals...);
      }
      void emit_operand32(uint32_t val) { memcpy(code, &val, sizeof(val)); code += sizeof(val); }
      void emit_operand64(uint64_t val) { memcpy(code, &val, sizeof(val)); code += sizeof(val); }
      void emit_operandf32(float val) { memcpy(code, &val, sizeof(val)); code += sizeof(val); }
      void emit_operandf64(double val) { memcpy(code, &val, sizeof(val)); code += sizeof(val); }
      template<class T>
      void emit_operand_ptr(T* val) { memcpy(code, &val, sizeof(val)); code += sizeof(val); }

     void* emit_branch_target32() {
        void * result = code;
        emit_operand32(3735928555u - static_cast<uint32_t>(reinterpret_cast<uintptr_t>(code)));
        return result;
     }

      static void unimplemented() { EOS_WB_ASSERT(false, wasm_parse_exception, "Sorry, not implemented."); }

      void emit_multipop(uint32_t count) {
         if(count > 0 && count != 0x80000001) {
            // add depth_change*8, %rsp
            if(count & 0xF0000000) unimplemented();
            emit_bytes(0x48, 0x81, 0xc4); // TODO: Prefer imm8 where appropriate
            emit_operand32(count * 8); // FIXME: handle overflow
         }
      }

      template<class... T>
      void emit_load_impl(uint32_t offset, T... loadop) {
         // pop %rax
         emit_bytes(0x58);
         // add offset, %rax // FIXME: offset should not be sign-extended
         if (offset & 0x80000000) unimplemented();
         emit_bytes(0x48, 0x05);
         emit_operand32(offset);
         // add %rsi, %rax
         emit_bytes(0x48, 0x01, 0xf0);
         // from the caller
         emit_bytes(static_cast<uint8_t>(loadop)...);
         // push RAX
         emit_bytes(0x50);
      }

      template<class... T>
      void emit_store_impl(uint32_t offset, T... storeop) {
         // pop RCX
         emit_bytes(0x59);
         // pop RAX
         emit_bytes(0x58);
         // add $offset, %rax // FIXME: offset should not be sign-extended
         if (offset & 0x80000000) unimplemented();
         emit_bytes(0x48, 0x05);
         emit_operand32(offset);
         // add %rsi, %rax
         emit_bytes(0x48, 0x01, 0xf0);
         // from the caller
         emit_bytes(static_cast<uint8_t>(storeop)...);;
      }

      void emit_i32_relop(uint8_t opcode) {
         // popq %rax
         emit_bytes(0x58);
         // popq %rcx
         emit_bytes(0x59);
         // xorq %rdx, %rdx
         emit_bytes(0x48, 0x31, 0xd2);
         // cmpl %eax, %ecx
         emit_bytes(0x39, 0xc1);
         // SETcc %dl
         emit_bytes(0x0f, opcode, 0xc2);
         // pushq %rdx
         emit_bytes(0x52);
      }

      template<class... T>
      void emit_i64_relop(uint8_t opcode) {
         // popq %rax
         emit_bytes(0x58);
         // popq %rcx
         emit_bytes(0x59);
         // xorq %rdx, %rdx
         emit_bytes(0x48, 0x31, 0xd2);
         // cmpq %rax, %rcx
         emit_bytes(0x48, 0x39, 0xc1);
         // SETcc %dl
         emit_bytes(0x0f, opcode, 0xc2);
         // pushq %rdx
         emit_bytes(0x52);
      }

      void emit_f32_relop(uint8_t opcode, bool (*softfloatfun)(float32_t, float32_t), bool switch_params, bool flip_result) {
         if constexpr (use_softfloat) {
            // popq rsi
            // popq rdi
            // call softfloatfun
            // pushq rax
         } else {
            // ucomiss+seta/setae is shorter but can't handle eq/ne
            if(!switch_params) {
               // movss (%rsp), %xmm0
               emit_bytes(0xf3, 0x0f, 0x10, 0x04, 0x24);
               // cmpCCss 8(%rsp), %xmm0
               emit_bytes(0xf3, 0x0f, 0xc2, 0x44, 0x24, 0x08, opcode);
            } else {
               // movss 8(%rsp), %xmm0
               emit_bytes(0xf3, 0x0f, 0x10, 0x44, 0x24, 0x08);
               // cmpCCss (%rsp), %xmm0
               emit_bytes(0xf3, 0x0f, 0xc2, 0x04, 0x24, opcode);
            }               
            // movd %xmm0, %eax
            emit_bytes(0x66, 0x0f, 0x7e, 0xc0);
            if (!flip_result) {
               // andl 1, eax
               emit_bytes(0x23, 0x04, 0x25, 0x01, 0x00, 0x00, 0x00);
            } else {
               // incl %eax {0xffffffff, 0} -> {0, 1}
               emit_bytes(0xff, 0xc0);
            }
            // leaq 16(%rsp), %rsp
            emit_bytes(0x48, 0x8d, 0x64, 0x24, 0x10);
            // pushq %rax
            emit_bytes(0x50);
         }
      }

      void emit_f64_relop(uint8_t opcode, bool (*softfloatfun)(float64_t, float64_t), bool switch_params, bool flip_result) {
         if constexpr (use_softfloat) {
            // popq rsi
            // popq rdi
            // call softfloatfun
            // pushq rax
         } else {
            // ucomisd+seta/setae is shorter but can't handle eq/ne
            if(!switch_params) {
               // movsd (%rsp), %xmm0
               emit_bytes(0xf2, 0x0f, 0x10, 0x04, 0x24);
               // cmpCCsd 8(%rsp), %xmm0
               emit_bytes(0xf2, 0x0f, 0xc2, 0x44, 0x24, 0x08, opcode);
            } else {
               // movsd 8(%rsp), %xmm0
               emit_bytes(0xf2, 0x0f, 0x10, 0x44, 0x24, 0x08);
               // cmpCCsd (%rsp), %xmm0
               emit_bytes(0xf2, 0x0f, 0xc2, 0x04, 0x24, opcode);
            }               
            // movd %xmm0, %eax
            emit_bytes(0x66, 0x0f, 0x7e, 0xc0);
            if (!flip_result) {
               // andl 1, eax
               emit_bytes(0x23, 0x04, 0x25, 0x01, 0x00, 0x00, 0x00);
            } else {
               // incl %eax {0xffffffff, 0} -> {0, 1}
               emit_bytes(0xff, 0xc0);
            }
            // leaq 16(%rsp), %rsp
            emit_bytes(0x48, 0x8d, 0x64, 0x24, 0x10);
            // pushq %rax
            emit_bytes(0x50);
         }
      }

      template<class... T>
      void emit_i32_binop(T... op) {
         // popq %rcx
         emit_bytes(0x59);
         // popq %rax
         emit_bytes(0x58);
         // OP %eax, %ecx
         emit_bytes(static_cast<uint8_t>(op)...);
         // pushq %rax
         // emit_bytes(0x50);
      }

      template<class... T>
      void emit_i64_binop(T... op) {
         // popq %rcx
         emit_bytes(0x59);
         // popq %rax
         emit_bytes(0x58);
         // OP %eax, %ecx
         emit_bytes(static_cast<uint8_t>(op)...);
      }

      void emit_f32_binop(uint8_t op) {
         // movss 8(%rsp), %xmm0
         emit_bytes(0xf3, 0x0f, 0x10, 0x44, 0x24, 0x08);
         // OPss (%rsp), %xmm0
         emit_bytes(0xf3, 0x0f, op, 0x04, 0x24);
         // leaq 8(%rsp), %rsp
         emit_bytes(0x48, 0x8d, 0x64, 0x24, 0x08);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f64_binop(uint8_t op) {
         // movsd 8(%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x10, 0x44, 0x24, 0x08);
         // OPsd (%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, op, 0x04, 0x24);
         // leaq 8(%rsp), %rsp
         emit_bytes(0x48, 0x8d, 0x64, 0x24, 0x08);
         // movsd %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }

      // Beware: This pushes and pops mxcsr around the user op.  Remember to adjust access to %rsp in the caller.
      // Note uses %rcx after the user instruction
      template<class... T>
      void emit_f2i(T... op) {
         // mov 0x0x1f80, %eax // round-to-even/all exceptions masked/no exceptions set
         emit_bytes(0x66, 0xb8, 0x80, 0x1f);
         // push %rax
         emit_bytes(0x50);
         // ldmxcsr (%rsp)
         emit_bytes(0x0f, 0xae, 0x14, 0x24);
         // user op
         emit_bytes(op...);
         // stmxcsr (%rsp)
         emit_bytes(0x0f, 0xae, 0x1c, 0x24);
         // pop %rcx
         emit_bytes(0x59);
         // test %cl, 0x1 // invalid
         emit_bytes(0xf6, 0xc1, 0x01);
         // jnz FP_ERROR_HANDLER
         emit_bytes(0x0f, 0x85);
         void * handler = emit_branch_target32();
         // FIXME: implement the handler
      }

      bool is_host_function(uint32_t funcnum) { return false; }
      void call_host_function(const func_type& ft, uint32_t funcnum) {}

      template<auto F, typename Context>
      static uint64_t host_function_wrapper(void* stack, Context* context) {
         // unpack args
         // call
      }

#if 0
      union stack_elem {
         uint32_t i32;
         uint64_t i64;
         float f32;
         double f64;
      };
      template <typename T>
      void start(stack_elem* data, int count, void(*fun)(), void* context) {
         asm {
            movq data, %rax;
            movq count, %rcx;
            testq %rcx, %rcx;
            jz done;
           loop:
            movq (%rax), %rdx;
            lea 8(%rax), %rax;
            pushq %rdx;
            decq %rcx;
            jnz loop;
           done:
            movq context, %rsi;
            movq fun, %rax
            callq %rax;
         }
      }
#endif
   };
   
}}
