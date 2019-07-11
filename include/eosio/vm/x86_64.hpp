#pragma once

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
   class machine_code_writer {
    public:
      machine_code_writer(growable_allocator&, std::size_t source_bytes) {
         code = allocator.alloc(source_bytes * 32 + 128); // FIXME: Guess at upper bound on function size
      }

      // FIXME: initialize locals
      void emit_prologue(const func_type& ft, const guarded_vector<local_entry>& locals) {
         // pushq RBP
         emit_bytes(0x55);
         // movq RSP, RBP
         emit_bytes(0x48, 0x89, 0xe5);
         // xor RAX, RAX
         for(std::size_t i = 0; i < locals.size(); ++i) {
            // pushq RAX
         }
      }
      void emit_epilogue(const func_type& ft, const guarded_vector<local_entry>& locals) {
         if(ft.result_count != 0) {
            // pop RAX
            emit_bytes(0x58);
         }
         emit_multipop(locals.size());
         // popq RBP
         emit_bytes(0x5d);
         // retq
         emit_bytes(0xc3);
      }
      
      void emit_unreachable() { call_host_function<&unreachable>(); }
      void emit_nop() {}
      void* emit_end() { return code; }
      void emit_return(uint32_t depth_change) {
         // Return is defined as equivalent to branching to the outermost label
         emit_br(depth_change);
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
         void* branch_addr = code;
         emit_operand32(0xEFBEADDE);
         return branch_addr;
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
         void* branch_addr = code;
         emit_operand32(0xEFBEADDE);
         return branch_addr;
      }
      void* emit_br_if(uint32_t depth_change) {
         // pop RAX
         emit_bytes(0x58);
         // add RSP, depth_change*8
         emit_multipop(depth_change);
         // test EAX, EAX
         emit_bytes(0x85, 0xC0);
         // jnz DEST
         emit_bytes(0x0F, 0x85);
         void* branch_addr = code;
         emit_operand32(0xEFBEADDE);
         return branch_addr;
      }

      void emit_br_table() {
         // requires refactoring of parser
      }

      void emit_call(const func_type& ft, uint32_t funcnum) {
         if(is_host_function(funcnum)) {
            call_host_function(ft, funcnum);
         } else {
            // callq TARGET
            emit_bytes(0xe8);
            register_call(code, funcnum);
            emit_operand32(0xEFBEADDE);
         }
      }

      void emit_call_indirect(const func_type& ft) {
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

      // FIXME: handle parameters
      void emit_get_local(uint32_t local_idx) {
         // mov -8*local_idx(EBP), RAX
         emit_bytes(0x48, 0x8b, 0x85);
         emit_operand32(-8 * local_idx);
         // push RAX
         emit_bytes(0x50);
      }

      void emit_set_local(uint32_t local_idx) {
         // pop RAX
         emit_bytes(0x58);
         // mov RAX, -8*local_idx(EBP)
         emit_bytes(0x48, 0x89, 0x85);
         emit_operand32(-8 * local_idx);
      }

      void emit_tee_local(uint32_t local_idx) {
         // pop RAX
         emit_bytes(0x58);
         // push RAX
         emit_bytes(0x50);
         // mov RAX, -8*local_idx(EBP)
         emit_bytes(0x48, 0x89, 0x85);
         emit_operand32(-8 * local_idx);
      }

      void emit_get_global();
      void emit_set_global();

      void emit_i32_load(uint32_t offset, uint32_t alignment) {
         // movl (RAX), EAX
         emit_load_impl(offset, 0x8b, 0x00);
      }

      void emit_i64_load(uint32_t offset, uint32_t alignment) {
         // movq (RAX), RAX
         emit_load_impl(offset, 0x48, 0x8b, 0x00);
      }

      void emit_f32_load(uint32_t offset, uint32_t alignment) {
         // movl (RAX), EAX
         emit_load_impl(offset, 0x8b, 0x00);
      }

      void emit_f64_load(uint32_t offset, uint32_t alignment) {
         // movq (RAX), RAX
         emit_load_impl(offset, 0x48, 0x8b, 0x00);
      }

      void emit_i32_load8_s(uint32_t offset, uint32_t alignment) {
         // movsbl (RAX), EAX; 
         emit_load_impl(offset, 0x0F, 0xbe, 0x00);
      }

      void emit_i32_load16_s(uint32_t offset, uint32_t alignment) {
         // movswl (RAX), EAX; 
         emit_load_impl(offset, 0x0F, 0xbf, 0x00);
      }

      void emit_i32_load8_u(uint32_t offset, uint32_t alignment) {
         // movzbl (RAX), EAX; 
         emit_load_impl(offset, 0x0f, 0xb6, 0x00);
      }

      void emit_i32_load16_u(uint32_t offset, uint32_t alignment) {
         // movzwl (RAX), EAX; 
         emit_load_impl(offset, 0x0f, 0xb7, 0x00);
      }

      void emit_i64_load8_s(uint32_t offset, uint32_t alignment) {
         // movsbq (RAX), RAX; 
         emit_load_impl(offset, 0x48, 0x0F, 0xbe, 0x00);
      }

      void emit_i64_load16_s(uint32_t offset, uint32_t alignment) {
         // movswq (RAX), RAX; 
         emit_load_impl(offset, 0x48, 0x0F, 0xbf, 0x00);
      }

      void emit_i64_load32_s(uint32_t offset, uint32_t alignment) {
         // movslq (RAX), RAX
         emit_load_impl(offset, 0x48, 0x63, 0x00);
      }

      void emit_i64_load8_u(uint32_t offset, uint32_t alignment) {
         // movzbl (RAX), EAX; 
         emit_load_impl(offset, 0x0f, 0xb6, 0x00);
      }

      void emit_i64_load16_u(uint32_t offset, uint32_t alignment) {
         // movzwl (RAX), EAX; 
         emit_load_impl(offset, 0x0f, 0xb7, 0x00);
      }

      void emit_i64_load32_s(uint32_t offset, uintt32_t alignment) {
         // movl (RAX), EAX
         emit_load_impl(offset, 0x8b, 0x00);
      }

      void emit_i32_store(uint32_t offset, uint32_t alignment) {
         // movl ECX, (RAX)
         emit_store_impl(0x89, 0x08);
      }

      void emit_i64_store(uint32_t offset, uint32_t alignment) {
         // movl ECX, (RAX)
         emit_store_impl(0x48, 0x89, 0x08);
      }

      void emit_f32_store(uint32_t offset, uint32_t alignment) {
         // movl ECX, (RAX)
         emit_store_impl(0x89, 0x08);
      }

      void emit_f64_store(uint32_t offset, uint32_t alignment) {
         // movl ECX, (RAX)
         emit_store_impl(0x48, 0x89, 0x08);
      }

      void emit_i32_store8(uint32_t offset, uint32_t alignment) {
         // movb CL, (RAX)
         emit_store_impl(0x88, 0x08);
      }

      void emit_i32_store16(uint32_t offset, uint32_t alignment) {
         // movb CX, (RAX)
         emit_store_impl(0x66, 0x89, 0x08);
      }

      void emit_i64_store8(uint32_t offset, uint32_t alignment) {
         // movb CL, (RAX)
         emit_store_impl(0x88, 0x08);
      }

      void emit_i64_store16(uint32_t offset, uint32_t alignment) {
         // movb CX, (RAX)
         emit_store_impl(0x66, 0x89, 0x08);
      }

      void emit_i64_store32(uint32_t offset, uint32_t alignment) {
         // movl ECX, (RAX)
         emit_store_impl(0x89, 0x08);
      }

      void emit_current_memory();
      void emit_grow_memory();

      void emit_i32_const(uint32_t value) {
         // mov value, %eax
         emit_bytes(0x8b, 0x04, 0x25);
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

      void emit_f32_const();
      void emit_f64_const();

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

      void emit_i32_neq() {
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

      // i64 relops
      void emit_i64_eq() {
         // sete %dl
         emit_i64_relop(0x94);
      }

      void emit_i64_neq() {
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

      // --------------- f32 relops ----------------------
      void emit_f32_eq() {
         emit_f32_relop(0x00, &f32_eq, false, false);
      }

      void emit_f32_ne() {
         emit_f32_relop(0x00, &f32_eq, false, true);
      }

      void emit_f32_lt() {
         emit_f32_relop(0x01, &f32_lt, false, false);
      }

      void emit_f32_gt() {
         emit_f32_relop(0x01, &f32_lt, true, false);
      }

      void emit_f32_le() {
         emit_f32_relop(0x02, &f32_le, false, false);
      }

      void emit_f32_ge() {
         emit_f32_relop(0x02, &f32_le, true, false);
      }

      // --------------- f64 relops ----------------------
      void emit_f64_eq() {
         emit_f64_relop(0x00, &f64_eq, false, false);
      }

      void emit_f64_ne() {
         emit_f64_relop(0x00, &f64_eq, false, true);
      }

      void emit_f64_lt() {
         emit_f64_relop(0x01, &f64_lt, false, false);
      }

      void emit_f64_gt() {
         emit_f64_relop(0x01, &f64_lt, true, false);
      }

      void emit_f64_le() {
         emit_f64_relop(0x02, &f64_le, false, false);
      }

      void emit_f64_ge() {
         emit_f64_relop(0x02, &f64_le, true, false);
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
      void emit_i64_mul() { emit_i64_binop(0x48 0x0f, 0xaf, 0xc1, 0x50); }
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
         // FIXME: make sure that fp exceptions do the right thing here.
         // cvttss2si (%rsp), %eax
         emit_bytes(0xf3, 0x0f, 0x2c, 0x04, 0x24);
         // mov %eax, (%rsp)
         emit_bytes(0x89, 0x04 ,0x24);
      }

      void emit_i32_trunc_u_f32();
      void emit_i32_trunc_s_f64();
      void emit_i32_trunc_u_f64();

      void emit_i64_extend_s_i32() {
         // movslq (%rsp), %rax
         emit_bytes(0x48, 0x63, 0x04, 0x24);
         // mov %rax, (%rsp)
         emit_bytes(0x48, 0x89, 0x04, 0x24);
      }

      void emit_i64_extend_u_i32() { /* Nothing to do */ }
      
      void emit_i64_trunc_s_f32();
      void emit_i64_trunc_u_f32();
      void emit_i64_trunc_s_f64();
      void emit_i64_trunc_u_f64();

      void emit_f32_convert_s_i32();
      // ... More conversions skipped for now

      // --------------- random  ------------------------
      void fix_branch(void* branch, void* target) {
         auto branch_ = static_cast<uint8_t*>(branch);
         auto target_ = static_cast<uint8_t*>(target);
         auto relative = static_cast<uint32_t>(target_ - (branch_ + 4));
         memcpy(branch, 4, &relative);
      }

      void finish_function() {
         emit_epilogue();
      }

      using fn_type = void(*)(void* context, void* memory);
      fn_type release() {
         return reinterpret_cast<fn_type>(_allocator.make_executable());
      }
    private:

      jit_allocator _allocator;
      unsigned char * code;
      void emit_byte(uint8_t val) { *code++ = val; }
      void emit_bytes() {}
      template<class T>
      void emit_bytes(uint8_t val0, T... vals) {
         emit_byte(val0);
         emit_bytes(vals...);
      }
      void emit_operand32(uint32_t val) { memcpy(code, &val, sizeof(val)); code += sizeof(val); }
      void emit_operand64(uint64_t val) { memcpy(code, &val, sizeof(val)); code += sizeof(val); }

      void emit_multipop(uint32_t count) {
         if(count > 0) {
            // add RSP, depth_change*8
            emit_bytes(0x48, 0x01, 0x24, 0x25); // TODO: Prefer imm8 where appropriate
            emit_operand32(depth_change * 8); // FIXME: handle overflow
         }
      }

      template<class... T>
      void emit_load_impl(uint32_t offset, T... loadop) {
         // pop RAX
         emit_bytes(0x58);
         // add RAX, offset
         emit_bytes(0x48, 0x01, 0x04, 0x25);
         emit_operand32(offset);
         // add RAX, RSI
         emit_bytes(0x48, 0x01, 0xc6);
         // from the caller
         emit_bytes(static_cast<uint8_t>(loadop)...);
         // push RAX
         emit_bytes(0x50);
      }

      template<class T>
      void emit_store_impl(uint32_t offset, T... storeop) {
         // pop RCX
         emit_bytes(0x59);
         // pop RAX
         emit_bytes(0x58);
         // add RAX, offset
         emit_bytes(0x48, 0x01, 0x04, 0x25);
         emit_operand32(offset);
         // add RAX, RSI
         emit_bytes(0x48, 0x01, 0xc6);
         // from the caller
         emit_bytes(static_cast<uint8_t>(loadop)...);;
      }

      void emit_i32_relop(uint8_t opcode) {
         // popq %rcx
         emit_bytes(0x59);
         // popq %rax
         emit_bytes(0x58);
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
         // popq %rcx
         emit_bytes(0x59);
         // popq %rax
         emit_bytes(0x58);
         // xorq %rdx, %rdx
         emit_bytes(0x48, 0x31, 0xd2);
         // cmpq %rax, %rcx
         emit_bytes(0x48, 0x39, 0xc1);
         // SETcc %dl
         emit_bytes(0x0f, opcode, 0xc2);
         // pushq %rdx
         emit_bytes(0x52);
      }

      void emit_float32_relop(uint8_t opcode, bool (*softfloatfun)(float32_t, float32_t), bool switch_params, bool flip_result) {
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

      void emit_float64_relop(uint8_t opcode, bool (*softfloatfun)(float64_t, float64_t), bool switch_params, bool flip_result) {
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
