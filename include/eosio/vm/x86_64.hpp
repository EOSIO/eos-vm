#pragma once

#include <eosio/vm/allocator.hpp>
#include <eosio/vm/types.hpp>

#include <eosio/vm/softfloat.hpp>

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
   template<typename Context>
   class machine_code_writer {
    public:
      machine_code_writer(growable_allocator& alloc, std::size_t source_bytes, module& mod) :
         _mod(mod) {
         // FIXME: Guess at upper bound on function size.  This should be an upper bound
         // except for the prologue and epilogue, but is not as tight as it could be.
         _mod.j_alloc = jit_allocator{source_bytes * 64};
         code = _mod.j_alloc.alloc();
         // always emit these functions
         fpe_handler = emit_error_handler(&on_fp_error);
         call_indirect_handler = emit_error_handler(&on_call_indirect_error);
         type_error_handler = emit_error_handler(&on_type_error);
         stack_overflow_handler = emit_error_handler(&on_stack_overflow);
         _mod.j_alloc.setpos(code);
      }
      ~machine_code_writer() { _mod.j_alloc.make_executable(); }

      void emit_prologue(const func_type& ft, const guarded_vector<local_entry>& locals, uint32_t funcnum) {
         _ft = &_mod.types[_mod.functions[funcnum]];
         start_function(code, funcnum);
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
      void emit_epilogue(const func_type& ft, const guarded_vector<local_entry>& locals, uint32_t funcnum) {
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

      void emit_unreachable() {
         // mov %rsp, rcx; andq $-16, %rsp; push rcx; push %rcx
         emit_bytes(0x48, 0x89, 0xe1);
         emit_bytes(0x48, 0x83, 0xe4, 0xf0);
         emit_bytes(0x51);
         emit_bytes(0x51);
         // movabsq &on_unreachable, %rax
         emit_bytes(0x48, 0xb8);
         emit_operand_ptr(&on_unreachable);
         // callq *%rax
         emit_bytes(0xff, 0xd0);
         // mov (%rsp), %rsp
         emit_bytes(0x48, 0x8b, 0x24, 0x24);
      }
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
         // jz DEST
         emit_bytes(0x0F, 0x84);
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
         // test EAX, EAX
         emit_bytes(0x85, 0xC0);

         if(depth_change == 0u || depth_change == 0x80000001u) {
            // jnz DEST
            emit_bytes(0x0F, 0x85);
            return emit_branch_target32();
         } else {
            // jz SKIP
            emit_bytes(0x0f, 0x84);
            void* skip = emit_branch_target32();
            // add depth_change*8, %rsp
            emit_multipop(depth_change);
            // jmp DEST
            emit_bytes(0xe9);
            void* result = emit_branch_target32();
            // SKIP:
            fix_branch(skip, code);
            return result;
         }
      }

      // Generate a binary search.
      struct br_table_generator {
         void* emit_case(uint32_t depth_change) {
            while(true) {
               assert(!stack.empty() && "The parser is supposed to handle the number of elements in br_table.");
               auto [min, max, label] = stack.back();
               stack.pop_back();
               if (label) {
                  fix_branch(label, _this->code);
               }
               if (max - min > 1) {
                  // Emit a comparison to the midpoint of the current range
                  uint32_t mid = min + (max - min)/2;
                  // cmp i, %mid
                  _this->emit_bytes(0x3d);
                  _this->emit_operand32(mid);
                  // jae MID
                  _this->emit_bytes(0x0f, 0x83);
                  void* mid_label = _this->emit_branch_target32();
                  stack.push_back({mid,max,mid_label});
                  stack.push_back({min,mid,nullptr});
               } else {
                  assert(min == _i);
                  _i++;
                  if (depth_change == 0u || depth_change == 0x80000001u) {
                     if(label) {
                        return label;
                     } else {
                        // jmp TARGET
                        _this->emit_bytes(0xe9);
                        return _this->emit_branch_target32();
                     }
                  } else {
                     // jne NEXT
                    _this->emit_multipop(depth_change);
                    // jmp TARGET
                    _this->emit_bytes(0xe9);
                    return _this->emit_branch_target32();
                  }
               }
            }

         }
         void* emit_default(uint32_t depth_change) {
            void* result = emit_case(depth_change);
            assert(stack.empty() && "unexpected default.");
            return result;
         }
         machine_code_writer * _this;
         int _i = 0;
         struct stack_item {
            uint32_t min;
            uint32_t max;
            void* branch_target = nullptr;
         };
         // stores a stack of ranges to be handled.
         // the ranges are strictly contiguous and non-ovelapping, with
         // the lower values at the back.
         std::vector<stack_item> stack;
      };
      br_table_generator emit_br_table(uint32_t table_size) {
         // pop %rax
         emit_bytes(0x58);
         // Increase the size by one to account for the default.
         // The current algorithm handles this correctly, without
         // any special cases.
         return { this, 0, { {0, table_size+1, nullptr} } };
      }

      void register_call(void* ptr, uint32_t funcnum) {
         auto& vec = _function_relocations;
         if(funcnum >= vec.size()) vec.resize(funcnum + 1);
         if(void** addr = std::get_if<void*>(&vec[funcnum])) {
            fix_branch(ptr, *addr);
         } else {
            std::get<std::vector<void*>>(vec[funcnum]).push_back(ptr);
         }
      }
      void start_function(void* func_start, uint32_t funcnum) {
         auto& vec = _function_relocations;
         if(funcnum >= vec.size()) vec.resize(funcnum + 1);
         for(void* branch : std::get<std::vector<void*>>(vec[funcnum])) {
            fix_branch(branch, func_start);
         }
         vec[funcnum] = func_start;
      }

      void emit_call(const func_type& ft, uint32_t funcnum) {
         emit_check_call_depth();
         if(is_host_function(funcnum)) {
            // mov $funcnum, %edx
            emit_bytes(0xba);
            emit_operand32(funcnum);
            // pushq %rdi
            emit_bytes(0x57);
            // pushq %rsi
            emit_bytes(0x56);
            // lea 16(%rsp), %rsi
            emit_bytes(0x48, 0x8d, 0x74, 0x24, 0x10);
            // mov %rsp, rcx; andq $-16, %rsp; push rcx; push %rcx
            emit_bytes(0x48, 0x89, 0xe1);
            emit_bytes(0x48, 0x83, 0xe4, 0xf0);
            emit_bytes(0x51);
            emit_bytes(0x51);
            // movabsq $call_host_function, %rax
            emit_bytes(0x48, 0xb8);
            emit_operand_ptr(&call_host_function);
            // callq *%rax
            emit_bytes(0xff, 0xd0);
            // mov (%rsp), %rsp
            emit_bytes(0x48, 0x8b, 0x24, 0x24);
            // popq %rsi
            emit_bytes(0x5e);
            // popq %rdi
            emit_bytes(0x5f);
            // addq ft.param_types.size()*8, %rsp
            emit_multipop(ft.param_types.size());
            if(ft.return_count != 0)
               // pushq %rax
               emit_bytes(0x50);
         } else {
            // callq TARGET
            emit_bytes(0xe8);
            void * branch = emit_branch_target32();
            emit_multipop(ft.param_types.size());
            register_call(branch, funcnum - _mod.get_imported_functions_size());
            if(ft.return_count != 0)
               // pushq %rax
               emit_bytes(0x50);
         }
         emit_check_call_depth_end();
      }

      void emit_call_indirect(const func_type& ft, uint32_t functypeidx) {
         emit_check_call_depth();
         auto& table = _mod.tables[0].table;
         void * functions = &_mod.fast_functions[0];
         void * code_with_offset = &_mod.code[0].jit_code;
         functypeidx = _mod.type_aliases[functypeidx];
         // pop %rax
         emit_bytes(0x58);
         // cmp $size, %rax
         emit_bytes(0x48, 0x3d);
         emit_operand32(table.size());
         // jae ERROR
         emit_bytes(0x0f, 0x83);
         fix_branch(emit_branch_target32(), call_indirect_handler);
         // shl $2, %rax
         emit_bytes(0x48, 0xc1, 0xe0, 0x02);
         // movabsq $table, %rdx
         emit_bytes(0x48, 0xba);
         emit_operand_ptr(table.raw());
         // add %rdx, %rax
         emit_bytes(0x48, 0x01, 0xd0);
         // movl (%rax), %eax // function index
         emit_bytes(0x8b, 0x00);
         // movl %eax, %ecx
         emit_bytes(0x89, 0xc1);
         // shl $2, %rcx
         emit_bytes(0x48, 0xc1, 0xe1, 0x02);
         // movabsq $functions, %rdx
         emit_bytes(0x48, 0xba);
         emit_operand_ptr(functions);
         // add %rdx, %rcx
         emit_bytes(0x48, 0x01, 0xd1);
         // movl (%rcx), %ecx // function type
         emit_bytes(0x8b, 0x09);
         // cmp functypeidx, %ecx
         emit_bytes(0x81, 0xf9);
         emit_operand32(functypeidx);
         // jne ERROR
         emit_bytes(0x0f, 0x85);
         fix_branch(emit_branch_target32(), type_error_handler);
         // sub $num_imported_functions, %eax
         emit_bytes(0x81, 0xe8);
         emit_operand32(_mod.get_imported_functions_size());
         // jb IMPORTED
         emit_bytes(0x0f, 0x8c);
         void * imported = emit_branch_target32();
         // imul sizeof(function_body), %rax, %rax
         static_assert(sizeof(function_body) < 256);
         emit_bytes(0x48, 0x6b, 0xc0, sizeof(function_body));
         // movabsq $code_with_offset, %rdx
         emit_bytes(0x48, 0xba);
         emit_operand_ptr(code_with_offset);
         // addq %rdx, %rax
         emit_bytes(0x48, 0x01, 0xd0);
         // movq (%rax), %rax
         emit_bytes(0x48, 0x8b, 0x00);
         // callq *%rax
         emit_bytes(0xff, 0xd0);
         // jmp DONE
         emit_bytes(0xe9);
         void * done = emit_branch_target32();
         // IMPORTED:
         fix_branch(imported, code);
         // add $num_imported_functions, %eax
         emit_bytes(0x81, 0xc0);
         emit_operand32(_mod.get_imported_functions_size());
         // mov %eax, %edx
         emit_bytes(0x89, 0xc2);
         // pushq %rdi
         emit_bytes(0x57);
         // pushq %rsi
         emit_bytes(0x56);
         // lea 16(%rsp), %rsi
         emit_bytes(0x48, 0x8d, 0x74, 0x24, 0x10);
         // mov %rsp, rcx; andq $-16, %rsp; push rcx; push %rcx
         emit_bytes(0x48, 0x89, 0xe1);
         emit_bytes(0x48, 0x83, 0xe4, 0xf0);
         emit_bytes(0x51);
         emit_bytes(0x51);
         // movabsq $call_host_function, %rax
         emit_bytes(0x48, 0xb8);
         emit_operand_ptr(&call_host_function);
         // callq *%rax
         emit_bytes(0xff, 0xd0);
         // mov (%rsp), %rsp
         emit_bytes(0x48, 0x8b, 0x24, 0x24);
         // popq %rsi
         emit_bytes(0x5e);
         // popq %rdi
         emit_bytes(0x5f);
         // DONE:
         fix_branch(done, code);
         emit_multipop(ft.param_types.size());
         if(ft.return_count != 0)
            // pushq %rax
            emit_bytes(0x50);
         emit_check_call_depth_end();
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
         if (local_idx < _ft->param_types.size()) {
            // mov 8*(local_idx)(%RBP), RAX
            emit_bytes(0x48, 0x8b, 0x85);
            emit_operand32(8 * (_ft->param_types.size() - local_idx + 1));
            // push RAX
            emit_bytes(0x50);
         } else {
            // mov -8*(local_idx+1)(%RBP), RAX
            emit_bytes(0x48, 0x8b, 0x85);
            emit_operand32(-8 * (local_idx - _ft->param_types.size() + 1));
            // push RAX
            emit_bytes(0x50);
         }
      }

      void emit_set_local(uint32_t local_idx) {
         if (local_idx < _ft->param_types.size()) {
            // pop RAX
            emit_bytes(0x58);
            // mov RAX, -8*local_idx(EBP)
            emit_bytes(0x48, 0x89, 0x85);
            emit_operand32(8 * (_ft->param_types.size() - local_idx + 1));
         } else {
            // pop RAX
            emit_bytes(0x58);
            // mov RAX, -8*local_idx(EBP)
            emit_bytes(0x48, 0x89, 0x85);
            emit_operand32(-8 * (local_idx - _ft->param_types.size() + 1));
         }
      }

      void emit_tee_local(uint32_t local_idx) {
         if (local_idx < _ft->param_types.size()) {
            // pop RAX
            emit_bytes(0x58);
            // push RAX
            emit_bytes(0x50);
            // mov RAX, -8*local_idx(EBP)
            emit_bytes(0x48, 0x89, 0x85);
            emit_operand32(8 * (_ft->param_types.size() - local_idx + 1));
         } else {
            // pop RAX
            emit_bytes(0x58);
            // push RAX
            emit_bytes(0x50);
            // mov RAX, -8*local_idx(EBP)
            emit_bytes(0x48, 0x89, 0x85);
            emit_operand32(-8 * (local_idx - _ft->param_types.size() + 1));
         }
      }

      void emit_get_global(uint32_t globalidx) {
         auto& gl = _mod.globals[globalidx];
         void *ptr = &gl.current.value;
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
         void *ptr = &gl.current.value;
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

      void emit_current_memory() {
         // pushq %rdi
         emit_bytes(0x57);
         // pushq %rsi
         emit_bytes(0x56);
         // movabsq $current_memory, %rax
         emit_bytes(0x48, 0xb8);
         emit_operand_ptr(&current_memory);
         // call *%rax
         emit_bytes(0xff, 0xd0);
         // pop %rsi
         emit_bytes(0x5e);
         // pop %rdi
         emit_bytes(0x5f);
         // push %rax
         emit_bytes(0x50);
      }
      void emit_grow_memory() {
         // popq %rax
         emit_bytes(0x58);
         // pushq %rdi
         emit_bytes(0x57);
         // pushq %rsi
         emit_bytes(0x56);
         // movq %rax, %rsi
         emit_bytes(0x48, 0x89, 0xc6);
         // movabsq $grow_memory, %rax
         emit_bytes(0x48, 0xb8);
         emit_operand_ptr(&grow_memory);
         // call *%rax
         emit_bytes(0xff, 0xd0);
         // pop %rsi
         emit_bytes(0x5e);
         // pop %rdi
         emit_bytes(0x5f);
         // push %rax
         emit_bytes(0x50);
      }

      void emit_i32_const(uint32_t value) {
         // mov $value, %eax
         emit_bytes(0xb8);
         emit_operand32(value);
         // push %rax
         emit_bytes(0x50);
      }

      void emit_i64_const(uint64_t value) {
         // movabsq $value, %rax
         emit_bytes(0x48, 0xb8);
         emit_operand64(value);
         // push %rax
         emit_bytes(0x50);
      }

      void emit_f32_const(float value) { 
         // mov $value, %eax
         emit_bytes(0xb8);
         emit_operandf32(value);
         // push %rax
         emit_bytes(0x50);
      }
      void emit_f64_const(double value) {
         // movabsq $value, %rax
         emit_bytes(0x48, 0xb8);
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

      template<auto F>
      static float32_t adapt_f32_unop(float32_t arg) {
         return ::to_softfloat32(static_cast<decltype(F)>(F)(::from_softfloat32(arg)));
      }
      template<auto F>
      static float32_t adapt_f32_binop(float32_t lhs, float32_t rhs) {
         return ::to_softfloat32(static_cast<decltype(F)>(F)(::from_softfloat32(lhs), ::from_softfloat32(rhs)));
      }
      template<auto F>
      static bool adapt_f32_cmp(float32_t lhs, float32_t rhs) {
         return static_cast<decltype(F)>(F)(::from_softfloat32(lhs), ::from_softfloat32(rhs));
      }

      template<auto F>
      static float64_t adapt_f64_unop(float64_t arg) {
         return ::to_softfloat64(static_cast<decltype(F)>(F)(::from_softfloat64(arg)));
      }
      template<auto F>
      static float64_t adapt_f64_binop(float64_t lhs, float64_t rhs) {
         return ::to_softfloat64(static_cast<decltype(F)>(F)(::from_softfloat64(lhs), ::from_softfloat64(rhs)));
      }
      template<auto F>
      static bool adapt_f64_cmp(float64_t lhs, float64_t rhs) {
         return static_cast<decltype(F)>(F)(::from_softfloat64(lhs), ::from_softfloat64(rhs));
      }

      // HACK: avoid linking to softfloat if we aren't using it
      // and also avoid passing arguments in floating point registers,
      // since softfloat uses integer registers.
      template<auto F>
      constexpr auto choose_fn() {
         if constexpr (use_softfloat) {
            if constexpr (std::is_same_v<decltype(F), float(*)(float)>) {
               return &adapt_f32_unop<F>;
            } else if constexpr(std::is_same_v<decltype(F), float(*)(float,float)>) {
               return &adapt_f32_binop<F>;
            } else if constexpr(std::is_same_v<decltype(F), bool(*)(float,float)>) {
               return &adapt_f32_cmp<F>;
            } else if constexpr (std::is_same_v<decltype(F), double(*)(double)>) {
               return &adapt_f64_unop<F>;
            } else if constexpr(std::is_same_v<decltype(F), double(*)(double,double)>) {
               return &adapt_f64_binop<F>;
            } else if constexpr(std::is_same_v<decltype(F), bool(*)(double,double)>) {
               return &adapt_f64_cmp<F>;
            } else {
               static_assert(std::is_same_v<decltype(F), void>, "Unhandled function type");
            }
         } else {
            return nullptr;
         }
      }

      // --------------- f32 relops ----------------------
      void emit_f32_eq() {
         emit_f32_relop(0x00, choose_fn<&_eosio_f32_eq>(), false, false);
      }

      void emit_f32_ne() {
         emit_f32_relop(0x00, choose_fn<&_eosio_f32_eq>(), false, true);
      }

      void emit_f32_lt() {
         emit_f32_relop(0x01, choose_fn<&_eosio_f32_lt>(), false, false);
      }

      void emit_f32_gt() {
         emit_f32_relop(0x01, choose_fn<&_eosio_f32_lt>(), true, false);
      }

      void emit_f32_le() {
         emit_f32_relop(0x02, choose_fn<&_eosio_f32_le>(), false, false);
      }

      void emit_f32_ge() {
         emit_f32_relop(0x02, choose_fn<&_eosio_f32_le>(), true, false);
      }

      // --------------- f64 relops ----------------------
      void emit_f64_eq() {
         emit_f64_relop(0x00, choose_fn<&_eosio_f64_eq>(), false, false);
      }

      void emit_f64_ne() {
         emit_f64_relop(0x00, choose_fn<&_eosio_f64_eq>(), false, true);
      }

      void emit_f64_lt() {
         emit_f64_relop(0x01, choose_fn<&_eosio_f64_lt>(), false, false);
      }

      void emit_f64_gt() {
         emit_f64_relop(0x01, choose_fn<&_eosio_f64_lt>(), true, false);
      }

      void emit_f64_le() {
         emit_f64_relop(0x02, choose_fn<&_eosio_f64_le>(), false, false);
      }

      void emit_f64_ge() {
         emit_f64_relop(0x02, choose_fn<&_eosio_f64_le>(), true, false);
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
      // cdq; idiv %ecx; pushq %rax
      void emit_i32_div_s() { emit_i32_binop(0x99, 0xf7, 0xf9, 0x50); }
      void emit_i32_div_u() { emit_i32_binop(0x31, 0xd2, 0xf7, 0xf1, 0x50); }
      void emit_i32_rem_s() {
         // pop %rcx
         emit_bytes(0x59);
         // pop %rax
         emit_bytes(0x58);
         // cmp $-1, %edx
         emit_bytes(0x83, 0xf9, 0xff);
         // je MINUS1
         emit_bytes(0x0f, 0x84);
         void* minus1 = emit_branch_target32();
         // cdq
         emit_bytes(0x99);
         // idiv %ecx
         emit_bytes(0xf7, 0xf9);
         // jmp END
         emit_bytes(0xe9);
         void* end = emit_branch_target32();
         // MINUS1:
         fix_branch(minus1, code);
         // xor %edx, %edx
         emit_bytes(0x31, 0xd2);
         // END:
         fix_branch(end, code);
         // push %rdx
         emit_bytes(0x52);
      }
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
         // lzcntq %eax, %eax
         emit_bytes(0xf3, 0x48, 0x0f, 0xbd, 0xc0);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_i64_ctz() {
         // popq %rax
         emit_bytes(0x58);
         // tzcntq %eax, %eax
         emit_bytes(0xf3, 0x48, 0x0f, 0xbc, 0xc0);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_i64_popcnt() {
         // popq %rax
         emit_bytes(0x58);
         // popcntq %rax, %rax
         emit_bytes(0xf3, 0x48, 0x0f, 0xb8, 0xc0);
         // pushq %rax
         emit_bytes(0x50);
      }

      // --------------- i64 binops ----------------------

      void emit_i64_add() { emit_i64_binop(0x48, 0x01, 0xc8, 0x50); }
      void emit_i64_sub() { emit_i64_binop(0x48, 0x29, 0xc8, 0x50); }
      void emit_i64_mul() { emit_i64_binop(0x48, 0x0f, 0xaf, 0xc1, 0x50); }
      // cdq; idiv %rcx; pushq %rax
      void emit_i64_div_s() { emit_i64_binop(0x48, 0x99, 0x48, 0xf7, 0xf9, 0x50); }
      void emit_i64_div_u() { emit_i64_binop(0x48, 0x31, 0xd2, 0x48, 0xf7, 0xf1, 0x50); }
      void emit_i64_rem_s() {
         // pop %rcx
         emit_bytes(0x59);
         // pop %rax
         emit_bytes(0x58);
         // cmp $-1, %rcx
         emit_bytes(0x48, 0x83, 0xf9, 0xff);
         // je MINUS1
         emit_bytes(0x0f, 0x84);
         void* minus1 = emit_branch_target32();
         // cqo
         emit_bytes(0x48, 0x99);
         // idiv %rcx
         emit_bytes(0x48, 0xf7, 0xf9);
         // jmp END
         emit_bytes(0xe9);
         void* end = emit_branch_target32();
         // MINUS1:
         fix_branch(minus1, code);
         // xor %edx, %edx
         emit_bytes(0x31, 0xd2);
         // END:
         fix_branch(end, code);
         // push %rdx
         emit_bytes(0x52);
      }
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
         // andl 0x7fffffff, %eax
         emit_bytes(0x25);
         emit_operand32(0x7fffffff);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_f32_neg() {
         // popq %rax
         emit_bytes(0x58);
         // xorl 0x80000000, %eax
         emit_bytes(0x35);
         emit_operand32(0x80000000);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_f32_ceil() {
         // roundss 0b1010, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0a, 0x04, 0x24, 0x0a);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f32_floor() {
         // roundss 0b1001, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0a, 0x04, 0x24, 0x09);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f32_trunc() {
         // roundss 0b1011, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0a, 0x04, 0x24, 0x0b);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f32_nearest() {
         // roundss 0b1000, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0a, 0x04, 0x24, 0x08);
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
      void emit_f32_min() {
        // mov (%rsp), %eax
        emit_bytes(0x8b, 0x04, 0x24);
        // test %eax, %eax
        emit_bytes(0x85, 0xc0);
        // je ZERO
        emit_bytes(0x0f, 0x84);
        void* zero = emit_branch_target32();
        // movss 8(%rsp), %xmm0
        emit_bytes(0xf3, 0x0f, 0x10, 0x44, 0x24, 0x08);
        // minss (%rsp), %xmm0
        emit_bytes(0xf3, 0x0f, 0x5d, 0x04, 0x24);
        // jmp DONE
        emit_bytes(0xe9);
        void* done = emit_branch_target32();
        // ZERO:
        fix_branch(zero, code);
        // movss (%rsp), %xmm0
        emit_bytes(0xf3, 0x0f, 0x10, 0x04, 0x24);
        // minss 8(%rsp), %xmm0
        emit_bytes(0xf3, 0x0f, 0x5d, 0x44, 0x24, 0x08);
        // DONE:
        fix_branch(done, code);
        // add $8, %rsp
        emit_bytes(0x48, 0x83, 0xc4, 0x08);
        // movss %xmm0, (%rsp)
        emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }
      void emit_f32_max() {
        // mov (%rsp), %eax
        emit_bytes(0x8b, 0x04, 0x24);
        // test %eax, %eax
        emit_bytes(0x85, 0xc0);
        // je ZERO
        emit_bytes(0x0f, 0x84);
        void* zero = emit_branch_target32();
        // movss (%rsp), %xmm0
        emit_bytes(0xf3, 0x0f, 0x10, 0x04, 0x24);
        // maxss 8(%rsp), %xmm0
        emit_bytes(0xf3, 0x0f, 0x5f, 0x44, 0x24, 0x08);
        // jmp DONE
        emit_bytes(0xe9);
        void* done = emit_branch_target32();
        // ZERO:
        fix_branch(zero, code);
        // movss 8(%rsp), %xmm0
        emit_bytes(0xf3, 0x0f, 0x10, 0x44, 0x24, 0x08);
        // maxss (%rsp), %xmm0
        emit_bytes(0xf3, 0x0f, 0x5f, 0x04, 0x24);
        // DONE:
        fix_branch(done, code);
        // add $8, %rsp
        emit_bytes(0x48, 0x83, 0xc4, 0x08);
        // movss %xmm0, (%rsp)
        emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f32_copysign() {
         // popq %rax; 
         emit_bytes(0x58);
         // andl 0x80000000, %eax
         emit_bytes(0x25);
         emit_operand32(0x80000000);
         // popq %rcx
         emit_bytes(0x59);
         // andl 0x7fffffff, %ecx
         emit_bytes(0x81, 0xe1);
         emit_operand32(0x7fffffff);
         // orl %ecx, %eax
         emit_bytes(0x09, 0xc8);
         // pushq %rax
         emit_bytes(0x50);
      }
      
      // --------------- f64 unops ----------------------

      void emit_f64_abs() {
         // popq %rcx; 
         emit_bytes(0x59);
         // movabsq $0x7fffffffffffffff, %rax
         emit_bytes(0x48, 0xb8);
         emit_operand64(0x7fffffffffffffffull);
         // andq %rcx, %rax
         emit_bytes(0x48, 0x21, 0xc8);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_f64_neg() {
         // popq %rcx; 
         emit_bytes(0x59);
         // movabsq $0x8000000000000000, %rax
         emit_bytes(0x48, 0xb8);
         emit_operand64(0x8000000000000000ull);
         // xorq %rcx, %rax
         emit_bytes(0x48, 0x31, 0xc8);
         // pushq %rax
         emit_bytes(0x50);
      }

      void emit_f64_ceil() {
         // roundsd 0b1010, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0b, 0x04, 0x24, 0x0a);
         // movsd %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f64_floor() {
         // roundsd 0b1001, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0b, 0x04, 0x24, 0x09);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f64_trunc() {
         // roundsd 0b1011, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0b, 0x04, 0x24, 0x0b);
         // movss %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f64_nearest() {
         // roundsd 0b1000, (%rsp), %xmm0
         emit_bytes(0x66, 0x0f, 0x3a, 0x0b, 0x04, 0x24, 0x08);
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
      void emit_f64_min() {
         // mov (%rsp), %rax
         emit_bytes(0x48, 0x8b, 0x04, 0x24);
         // test %rax, %rax
         emit_bytes(0x48, 0x85, 0xc0);
         // je ZERO
         emit_bytes(0x0f, 0x84);
         void* zero = emit_branch_target32();
         // movsd 8(%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x10, 0x44, 0x24, 0x08);
         // minsd (%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x5d, 0x04, 0x24);
         // jmp DONE
         emit_bytes(0xe9);
         void* done = emit_branch_target32();
         // ZERO:
         fix_branch(zero, code);
         // movsd (%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x10, 0x04, 0x24);
         // minsd 8(%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x5d, 0x44, 0x24, 0x08);
         // DONE:
         fix_branch(done, code);
         // add $8, %rsp
         emit_bytes(0x48, 0x83, 0xc4, 0x08);
         // movsd %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }
      void emit_f64_max() {
         // mov (%rsp), %rax
         emit_bytes(0x48, 0x8b, 0x04, 0x24);
         // test %rax, %rax
         emit_bytes(0x48, 0x85, 0xc0);
         // je ZERO
         emit_bytes(0x0f, 0x84);
         void* zero = emit_branch_target32();
         // maxsd (%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x10, 0x04, 0x24);
         // maxsd 8(%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x5f, 0x44, 0x24, 0x08);
         // jmp DONE
         emit_bytes(0xe9);
         void* done = emit_branch_target32();
         // ZERO:
         fix_branch(zero, code);
         // movsd 8(%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x10, 0x44, 0x24, 0x08);
         // maxsd (%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x5f, 0x04, 0x24);
         // DONE:
         fix_branch(done, code);
         // add $8, %rsp
         emit_bytes(0x48, 0x83, 0xc4, 0x08);
         // movsd %xmm0, (%rsp)
         emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }

      void emit_f64_copysign() {
         // popq %rcx; 
         emit_bytes(0x59);
         // movabsq 0x8000000000000000, %rax
         emit_bytes(0x48, 0xb8);
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
         // shr $32, %rax
         emit_bytes(0x48, 0xc1, 0xe8, 0x20);
         // test %eax, %eax
         emit_bytes(0x85, 0xc0);
         // jnz FP_ERROR_HANDLER
         emit_bytes(0x0f, 0x85);
         fix_branch(emit_branch_target32(), fpe_handler);
      }
      void emit_i32_trunc_s_f64() {
         // cvttsd2si 8(%rsp), %eax
         emit_f2i(0xf2, 0x0f, 0x2c, 0x44, 0x24, 0x08);
         // movq %rax, (%rsp)
         emit_bytes(0x48, 0x89, 0x04 ,0x24);
      }

      void emit_i32_trunc_u_f64() {
         // cvttsd2si 8(%rsp), %rax
         emit_f2i(0xf2, 0x48, 0x0f, 0x2c, 0x44, 0x24, 0x08);
         // movq %rax, (%rsp)
         emit_bytes(0x48, 0x89, 0x04 ,0x24);
         // shr $32, %rax
         emit_bytes(0x48, 0xc1, 0xe8, 0x20);
         // test %eax, %eax
         emit_bytes(0x85, 0xc0);
         // jnz FP_ERROR_HANDLER
         emit_bytes(0x0f, 0x85);
         fix_branch(emit_branch_target32(), fpe_handler);
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
         // mov $0x5f000000, %eax
         emit_bytes(0xb8);
         emit_operand32(0x5f000000);
         // movss (%rsp), %xmm0
         emit_bytes(0xf3, 0x0f, 0x10, 0x04, 0x24);
         // mov %eax, (%rsp)
         emit_bytes(0x89, 0x04, 0x24);
         // movss (%rsp), %xmm1
         emit_bytes(0xf3, 0x0f, 0x10, 0x0c, 0x24);
         // movaps %xmm0, %xmm2
         emit_bytes(0x0f, 0x28, 0xd0);
         // subss %xmm1, %xmm2
         emit_bytes(0xf3, 0x0f, 0x5c, 0xd1);
         // cvttss2siq %xmm2, %rax
         emit_f2i(0xf3, 0x48, 0x0f, 0x2c, 0xc2);
         // movabsq $0x8000000000000000, %rcx
         emit_bytes(0x48, 0xb9);
         emit_operand64(0x8000000000000000);
         // xorq %rax, %rcx
         emit_bytes(0x48, 0x31, 0xc1);
         // cvttss2siq %xmm0, %rax
         emit_bytes(0xf3, 0x48, 0x0f, 0x2c, 0xc0);
         // xor %rdx, %rdx
         emit_bytes(0x48, 0x31, 0xd2);
         // ucomiss %xmm0, %xmm1
         emit_bytes(0x0f, 0x2e, 0xc8);
         // cmovaq %rax, %rdx
         emit_bytes(0x48, 0x0f, 0x47, 0xd0);
         // cmovbeq %rcx, %rax
         emit_bytes(0x48, 0x0f, 0x46, 0xc1);
         // mov %rax, (%rsp)
         emit_bytes(0x48, 0x89, 0x04, 0x24);
         // bt $63, %rdx
         emit_bytes(0x48, 0x0f, 0xba, 0xe2, 0x3f);
         // jc FP_ERROR_HANDLER
         emit_bytes(0x0f, 0x82);
         fix_branch(emit_branch_target32(), fpe_handler);
      }
      void emit_i64_trunc_s_f64() {
         // cvttsd2si (%rsp), %rax
         emit_f2i(0xf2, 0x48, 0x0f, 0x2c, 0x44, 0x24, 0x08);
         // mov %rax, (%rsp)
         emit_bytes(0x48, 0x89, 0x04 ,0x24);
      }
      void emit_i64_trunc_u_f64() {
         // movabsq $0x43e0000000000000, %rax
         emit_bytes(0x48, 0xb8);
         emit_operand64(0x43e0000000000000);
         // movsd (%rsp), %xmm0
         emit_bytes(0xf2, 0x0f, 0x10, 0x04, 0x24);
         // movq %rax, (%rsp)
         emit_bytes(0x48, 0x89, 0x04, 0x24);
         // movsd (%rsp), %xmm1
         emit_bytes(0xf2, 0x0f, 0x10, 0x0c, 0x24);
         // movapd %xmm0, %xmm2
         emit_bytes(0x66, 0x0f, 0x28, 0xd0);
         // subsd %xmm1, %xmm2
         emit_bytes(0xf2, 0x0f, 0x5c, 0xd1);
         // cvttsd2siq %xmm2, %rax
         emit_f2i(0xf2, 0x48, 0x0f, 0x2c, 0xc2);
         // movabsq $0x8000000000000000, %rcx
         emit_bytes(0x48, 0xb9);
         emit_operand64(0x8000000000000000);
         // xorq %rax, %rcx
         emit_bytes(0x48, 0x31, 0xc1);
         // cvttsd2siq %xmm0, %rax
         emit_bytes(0xf2, 0x48, 0x0f, 0x2c, 0xc0);
         // xor %rdx, %rdx
         emit_bytes(0x48, 0x31, 0xd2);
         // ucomisd %xmm0, %xmm1
         emit_bytes(0x66, 0x0f, 0x2e, 0xc8);
         // cmovaq %rax, %rdx
         emit_bytes(0x48, 0x0f, 0x47, 0xd0);
         // cmovbeq %rcx, %rax
         emit_bytes(0x48, 0x0f, 0x46, 0xc1);
         // mov %rax, (%rsp)
         emit_bytes(0x48, 0x89, 0x04, 0x24);
         // bt $63, %rdx
         emit_bytes(0x48, 0x0f, 0xba, 0xe2, 0x3f);
         // jc FP_ERROR_HANDLER
         emit_bytes(0x0f, 0x82);
         fix_branch(fpe_handler, emit_branch_target32());
      }

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
      void emit_f32_convert_u_i64() {
        // movq (%rsp), %rax
        emit_bytes(0x48, 0x8b, 0x04, 0x24);
        // testq %rax, %rax
        emit_bytes(0x48, 0x85, 0xc0);
        // js LARGE
        emit_bytes(0x0f, 0x88);
        void * large = emit_branch_target32();
        // cvtsi2ssq %rax, %xmm0
        emit_bytes(0xf3, 0x48, 0x0f, 0x2a, 0xc0);
        // jmp done
        emit_bytes(0xe9);
        void* done = emit_branch_target32();
        // LARGE:
        fix_branch(large, code);
        // movq %rax, %rcx
        emit_bytes(0x48, 0x89, 0xc1);
        // shrq %rax
        emit_bytes(0x48, 0xd1, 0xe8);
        // andl $1, %ecx
        emit_bytes(0x83, 0xe1, 0x01);
        // orq %rcx, %rax
        emit_bytes(0x48, 0x09, 0xc8);
        // cvtsi2ssq %rax, %xmm0
        emit_bytes(0xf3, 0x48, 0x0f, 0x2a, 0xc0);
        // addss %xmm0, %xmm0
        emit_bytes(0xf3, 0x0f, 0x58, 0xc0);
        // DONE:
        fix_branch(done, code);
        // xorl %eax, %eax
        emit_bytes(0x31, 0xc0);
        // movl %eax, 4(%rsp)
        emit_bytes(0x89, 0x44, 0x24, 0x04);
        // movss %xmm0, (%rsp)
        emit_bytes(0xf3, 0x0f, 0x11, 0x04, 0x24);
      }
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
      void emit_f64_convert_u_i64() {
        // movq (%rsp), %rax
        emit_bytes(0x48, 0x8b, 0x04, 0x24);
        // testq %rax, %rax
        emit_bytes(0x48, 0x85, 0xc0);
        // js LARGE
        emit_bytes(0x0f, 0x88);
        void * large = emit_branch_target32();
        // cvtsi2sdq %rax, %xmm0
        emit_bytes(0xf2, 0x48, 0x0f, 0x2a, 0xc0);
        // jmp done
        emit_bytes(0xe9);
        void* done = emit_branch_target32();
        // LARGE:
        fix_branch(large, code);
        // movq %rax, %rcx
        emit_bytes(0x48, 0x89, 0xc1);
        // shrq %rax
        emit_bytes(0x48, 0xd1, 0xe8);
        // andl $1, %ecx
        emit_bytes(0x83, 0xe1, 0x01);
        // orq %rcx, %rax
        emit_bytes(0x48, 0x09, 0xc8);
        // cvtsi2sdq %rax, %xmm0
        emit_bytes(0xf2, 0x48, 0x0f, 0x2a, 0xc0);
        // addsd %xmm0, %xmm0
        emit_bytes(0xf2, 0x0f, 0x58, 0xc0);
        // DONE:
        fix_branch(done, code);
        // movsd %xmm0, (%rsp)
        emit_bytes(0xf2, 0x0f, 0x11, 0x04, 0x24);
      }
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

      using fn_type = native_value(*)(void* context, void* memory);
      void finalize(function_body& body) {
         body.jit_code = reinterpret_cast<fn_type>(_mod.j_alloc.setpos(code));
      }

      template<typename... Args>
      static native_value invoke(fn_type fn, void* context, void* linear_memory, Args&... args) {
         native_value args_raw[] = { transform_arg(args)... };
         return invoke_impl<sizeof...(Args)>(args_raw, fn, context, linear_memory);
      }

    private:

      module& _mod;
      const func_type* _ft;
      unsigned char * code;
      std::vector<std::variant<std::vector<void*>, void*>> _function_relocations;
      void* fpe_handler;
      void* call_indirect_handler;
      void* type_error_handler;
      void* stack_overflow_handler;

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

      void emit_check_call_depth() {
         // decl %ebx
         emit_bytes(0xff, 0xcb);
         // jz stack_overflow
         emit_bytes(0x0f, 0x84);
         fix_branch(emit_branch_target32(), stack_overflow_handler);
      }
      void emit_check_call_depth_end() {
         // incl %ebx
         emit_bytes(0xff, 0xc3);
      }

      static void unimplemented() { EOS_WB_ASSERT(false, wasm_parse_exception, "Sorry, not implemented."); }

      // clobbers %rax if the high bit of count is set.
      void emit_multipop(uint32_t count) {
         if(count > 0 && count != 0x80000001) {
            if (count & 0x80000000) {
               // mov (%rsp), %rax
               emit_bytes(0x48, 0x8b, 0x04, 0x24);
            }
            if(count & 0x70000000) {
               // This code is probably unreachable.
               // int3
               emit_bytes(0xCC);
            }
            // add depth_change*8, %rsp
            emit_bytes(0x48, 0x81, 0xc4); // TODO: Prefer imm8 where appropriate
            emit_operand32(count * 8); // FIXME: handle overflow
            if (count & 0x80000000) {
               // push %rax
               emit_bytes(0x50);
            }
         }
      }

      template<class... T>
      void emit_load_impl(uint32_t offset, T... loadop) {
         // pop %rax
         emit_bytes(0x58);
         if (offset & 0x80000000) {
            // mov $offset, %ecx
            emit_bytes(0xb9);
            emit_operand32(offset);
            // add %rcx, %rax
            emit_bytes(0x48, 0x01, 0xc8);
         } else if (offset != 0) {
            // add offset, %rax
            emit_bytes(0x48, 0x05);
            emit_operand32(offset);
         }
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
         if (offset & 0x80000000) {
            // mov $offset, %ecx
            emit_bytes(0xb9);
            emit_operand32(offset);
            // add %rcx, %rax
            emit_bytes(0x48, 0x01, 0xc8);
         } else if (offset != 0) {
            // add offset, %rax
            emit_bytes(0x48, 0x05);
            emit_operand32(offset);
         }
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
            // pushq %rdi
            emit_bytes(0x57);
            // pushq %rsi
            emit_bytes(0x56);
            if(switch_params) {
               // movq 24(%rsp), %esi
               emit_bytes(0x8b, 0x74, 0x24, 0x18);
               // movq 16(%rsp), %edi
               emit_bytes(0x8b, 0x7c, 0x24, 0x10);
            } else {
               // movq 16(%rsp), %esi
               emit_bytes(0x8b, 0x74, 0x24, 0x10);
               // movq 24(%rsp), %edi
               emit_bytes(0x8b, 0x7c, 0x24, 0x18);
            }
            emit_align_stack();
            // movabsq $softfloatfun, %rax
            emit_bytes(0x48, 0xb8);
            emit_operand_ptr(softfloatfun);
            // callq *%rax
            emit_bytes(0xff, 0xd0);
            emit_restore_stack();
            // popq %rsi
            emit_bytes(0x5e);
            // popq %rdi
            emit_bytes(0x5f);
            if (flip_result) {
               // xor $0x1, %al
               emit_bytes(0x34, 0x01);
            }
            // addq $8, %rsp
            emit_bytes(0x48, 0x83, 0xc4, 0x08);
            // movq %rax, (%rsp)
            emit_bytes(0x48, 0x89, 0x04, 0x24);
         } else {
            // ucomiss+seta/setae is shorter but can't handle eq/ne
            if(switch_params) {
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
               // andl $1, %eax
               emit_bytes(0x83, 0xe0, 0x01);
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
            // pushq %rdi
            emit_bytes(0x57);
            // pushq %rsi
            emit_bytes(0x56);
            if(switch_params) {
               // movq 24(%rsp), %rsi
               emit_bytes(0x48, 0x8b, 0x74, 0x24, 0x18);
               // movq 16(%rsp), %rdi
               emit_bytes(0x48, 0x8b, 0x7c, 0x24, 0x10);
            } else {
               // movq 16(%rsp), %rsi
               emit_bytes(0x48, 0x8b, 0x74, 0x24, 0x10);
               // movq 24(%rsp), %rdi
               emit_bytes(0x48, 0x8b, 0x7c, 0x24, 0x18);
            }
            emit_align_stack();
            // movabsq $softfloatfun, %rax
            emit_bytes(0x48, 0xb8);
            emit_operand_ptr(softfloatfun);
            // callq *%rax
            emit_bytes(0xff, 0xd0);
            emit_restore_stack();
            // popq %rsi
            emit_bytes(0x5e);
            // popq %rdi
            emit_bytes(0x5f);
            if (flip_result) {
               // xor $0x1, %al
               emit_bytes(0x34, 0x01);
            }
            // addq $8, %rsp
            emit_bytes(0x48, 0x83, 0xc4, 0x08);
            // movq %rax, (%rsp)
            emit_bytes(0x48, 0x89, 0x04, 0x24);
         } else {
            // ucomisd+seta/setae is shorter but can't handle eq/ne
            if(switch_params) {
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
               // andl $1, eax
               emit_bytes(0x83, 0xe0, 0x01);
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
         emit_bytes(0xb8, 0x80, 0x1f, 0x00, 0x00);
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
         fix_branch(emit_branch_target32(), fpe_handler);
      }

      void* emit_error_handler(void (*handler)()) {
         void* result = code;
         // andq $-16, %rsp;
         emit_bytes(0x48, 0x83, 0xe4, 0xf0);
         // movabsq &on_unreachable, %rax
         emit_bytes(0x48, 0xb8);
         emit_operand_ptr(handler);
         // callq *%rax
         emit_bytes(0xff, 0xd0);
         return result;
      }

      void emit_align_stack() {
         // mov %rsp, rcx; andq $-16, %rsp; push rcx; push %rcx
         emit_bytes(0x48, 0x89, 0xe1);
         emit_bytes(0x48, 0x83, 0xe4, 0xf0);
         emit_bytes(0x51);
         emit_bytes(0x51);
      }

      void emit_restore_stack() {
         // mov (%rsp), %rsp
         emit_bytes(0x48, 0x8b, 0x24, 0x24);
      }

      bool is_host_function(uint32_t funcnum) { return funcnum < _mod.get_imported_functions_size(); }

      static native_value call_host_function(Context* context /*rdi*/, native_value* stack /*rsi*/, uint32_t idx /*edx*/) {
         // It's currently unsafe to throw through a jit frame, because we don't set up
         // the exception tables for them.
         try {
            return context->call_host_function(stack, idx);
         } catch(...) {
            vm::throw_();
         }
      }

      static uint32_t current_memory(Context* context /*rdi*/) {
         return context->current_linear_memory();
      }

      static uint32_t grow_memory(Context* context /*rdi*/, uint32_t pages) {
         return context->grow_linear_memory(pages);
      }

      static void on_unreachable() { vm::throw_(wasm_interpreter_exception{ "unreachable" }); }
      static void on_fp_error() { vm::throw_(wasm_interpreter_exception{ "floating point error" }); }
      static void on_call_indirect_error() { vm::throw_(wasm_interpreter_exception{ "call_indirect out of range" }); }
      static void on_type_error() { vm::throw_(wasm_interpreter_exception{ "call_indirect incorrect function type" }); }
      static void on_stack_overflow() { vm::throw_(wasm_interpreter_exception{ "stack overflow" }); }

      template<typename T>
      static native_value transform_arg(T value) {
         // make sure that the garbage bits are always zero.
         native_value result;
         std::memset(&result, 0, sizeof(result));
         auto transformed_value = transform_arg_impl(value);
         std::memcpy(&result, &transformed_value, sizeof(transformed_value));
         return result;
      }

      static uint32_t transform_arg_impl(bool value) { return value; }
      static uint32_t transform_arg_impl(uint32_t value) { return value; }
      static uint64_t transform_arg_impl(uint64_t value) { return value; }
      static float    transform_arg_impl(float value) { return value; }
      static double   transform_arg_impl(double value) { return value; }

      template<int Count>
      static native_value invoke_impl(native_value* data, fn_type fun, void* context, void* linear_memory) {
         static_assert(sizeof(native_value) == 8, "8-bytes expected for native_value");
         native_value result;
         int count = Count;
         unsigned stack_check = constants::max_call_depth;
         asm volatile(
            "sub $16, %%rsp; "
            "stmxcsr 8(%%rsp); "
            "mov $0x1f80, %%rax; "
            "mov %%rax, (%%rsp); "
            "ldmxcsr (%%rsp); "
            "test %[count], %[count]; "
            "jz 2f; "
            "1: "
            "movq (%[data]), %%rax; "
            "lea 8(%[data]), %[data]; "
            "pushq %%rax; "
            "dec %[count]; "
            "jnz 1b; "
            "2: "
            "callq *%[fun]; "
            "add %[StackOffset], %%rsp; "
            "ldmxcsr 8(%%rsp); "
            "add $16, %%rsp; "
            : [result] "=&a" (result), [count] "+r" (count) // output
            : [data] "r" (data), [fun] "r" (fun),
              [context] "D" (context), [linear_memory] "S" (linear_memory),
              [StackOffset] "n" (Count*8), "b" (stack_check) // input
            : "memory", "cc" // clobber
         );
         return result;
      }
   };
   
}}
