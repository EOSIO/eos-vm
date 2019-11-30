#pragma once

#include <eosio/vm/utils.hpp>

namespace eosio { namespace vm { namespace x86_64 {
   namespace detail {
      // internal representation
      struct bits {

      };
   } // namespace detail

   enum class regs : uint8_t {
      rax = 0,
      rcx = 1,
      rdx = 2,
      rbx = 3,
      rsp = 4,
      rbp = 5,
      rsi = 6,
      rdi = 7,
      r8  = 8,
      r9  = 9,
      r10 = 10,
      r11 = 11,
      r12 = 12,
      r13 = 13,
      r14 = 14,
      r15 = 15
   };

   enum class addressing_modes : uint16_t {
      rel8off,
      rel16off,
      rel32off,
      reg16,
      reg32,
      reg64,
      mem16,
      mem32,
      mem64
   };

   struct reg8 {
      constexpr reg8(regs r) : value(static_cast<uint8_t>(r)) {}
      uint8_t value;
   };
   struct reg16 {
      constexpr reg16(regs r) : value(static_cast<uint8_t>(r)) {}
      uint8_t value;
   };
   struct reg32 {
      constexpr reg32(regs r) : value(static_cast<uint8_t>(r)) {}
      uint8_t value;
   };
   struct reg64 {
      constexpr reg64(regs r) : value(static_cast<uint8_t>(r)) {}
      uint8_t value;
   };
   struct mem_reg8 {
      constexpr mem_reg8(regs r) : reg(static_cast<uint8_t>(r)) {}
      constexpr mem_reg8(uintptr_t m) : mem(m) {}
      uint8_t   reg = 0;
      uintptr_t mem = 0;
   };
   struct mem_reg16 {
      constexpr mem_reg16(regs r) : reg(static_cast<uint8_t>(r)) {}
      constexpr mem_reg16(uintptr_t m) : mem(m) {}
      uint8_t   reg = 0;
      uintptr_t mem = 0;
   };
   struct mem_reg32 {
      constexpr mem_reg32(regs r) : reg(static_cast<uint8_t>(r)) {}
      constexpr mem_reg32(uintptr_t m) : mem(m) {}
      uint8_t   reg = 0;
      uintptr_t mem = 0;
   };
   struct mem_reg64 {
      constexpr mem_reg64(regs r) : reg(static_cast<uint8_t>(r)) {}
      constexpr mem_reg64(uintptr_t m) : mem(m) {}
      uint8_t   reg = 0;
      uintptr_t mem = 0;
   };

/*
   template <addressing_modes Mode>
   constexpr auto addressing_mode_to_type() {
      if constexpr (Mode == addressing_modes::rel8off)
         return (uint8_t)0;
      if constexpr (Mode == addressing_modes::rel16off)
         return (uint16_t)0;
      if constexpr (Mode == addressing_modes::rel32off)
         return (uint32_t)0;
      if constexpr (Mode == addressing_modes::reg16)
         return opcode_registers::rax;
      if constexpr (Mode == addressing_modes::reg32)
         return opcode_registers::rax;
      if constexpr (Mode == addressing_modes::reg64)
         return opcode_registers::rax;
      if constexpr (Mode == addressing_modes::mem16)
         return (uint16_t*)0;
      if constexpr (Mode == addressing_modes::mem32)
         return (uint32_t*)0;
      if constexpr (Mode == addressing_modes::mem64)
         return (uint64_t*)0;
   };

   enum class registers : uint8_t {
      rax,
      rbx,
      rcx,
      rdx,
      rsp,
      rbp,
      rsi,
      rdi,
      r8,
      r9,
      r10,
      r11,
      r12,
      r13,
      r14,
      r15,
      al,
      ah,
      ax,
      eax,
      bl,
      bh,
      bx,
      ebx,
      cl,
      ch,
      cx,
      ecx,
      dl,
      dh,
      dx,
      edx,
      sp,
      esp,
      bp,
      ebp,
      si,
      esi,
      di,
      edi,
      mmx0,
      mmx1,
      mmx2,
      mmx3,
      mmx4,
      mmx5,
      mmx6,
      mmx7,
      xmm0,
      xmm1,
      xmm2,
      xmm3,
      xmm4,
      xmm5,
      xmm6,
      xmm7,
      xmm8,
      xmm9,
      xmm10,
      xmm11,
      xmm12,
      xmm13,
      xmm14,
      xmm15,
      ymm0,
      ymm1,
      ymm2,
      ymm3,
      ymm4,
      ymm5,
      ymm6,
      ymm7,
      ymm8,
      ymm9,
      ymm10,
      ymm11,
      ymm12,
      ymm13,
      ymm14,
      ymm15,
      es,
      cs,
      ss,
      ds,
      fs,
      gs
   };
*/
   namespace prefix {
      struct group1 {
         static constexpr uint8_t lock  = 0xf0;
         static constexpr uint8_t repne = 0xf2;
         static constexpr uint8_t repnz = 0xf2;
         static constexpr uint8_t rep   = 0xf3;
         static constexpr uint8_t repe  = 0xf3;
         static constexpr uint8_t repz  = 0xf3;
      };
      struct group2 {
         static constexpr uint8_t cs_override = 0x2e; // 64-bit these are ignored
         static constexpr uint8_t ss_override = 0x36; // 64-bit these are ignore;
         static constexpr uint8_t ds_override = 0x3e; // 64-bit these are ignore;
         static constexpr uint8_t es_override = 0x26; // 64-bit these are ignore;
         static constexpr uint8_t fs_override = 0x64; // 64-bit these are ignore;
         static constexpr uint8_t gs_override = 0x65; // 64-bit these are ignore;
         static constexpr uint8_t branch_not_taken = 0x2e;
         static constexpr uint8_t branch_taken     = 0x3e;
      };
      struct group3 {
         static constexpr uint8_t operand_size_override = 0x66;
      };
      struct group4 {
         static constexpr uint8_t address_size_override = 0x67;
      };

      struct rex {
         rex(uint8_t v) : value(v) {}
         rex(uint8_t W, uint8_t R, uint8_t X, uint8_t B) : b0100(4), W(W), R(R), X(X), B(B) {}
         union {
            struct {
               uint8_t B : 1;
               uint8_t X : 1;
               uint8_t R : 1;
               uint8_t W : 1;
               uint8_t b0100 : 4;
            };
            uint8_t value;
         };
      };
   } // namespace prefix

   struct modrm {
      modrm(uint8_t m, uint8_t r, uint8_t rm) : mod(m), reg(r), rm(rm) {}
      modrm(uint8_t val) : value(val) {}
      union {
         struct {
            uint8_t rm  : 3;
            uint8_t reg : 3;
            uint8_t mod : 2;
         };
         uint8_t value;
      };
   };

   struct sib {
      uint8_t scale : 2;
      uint8_t index : 3;
      uint8_t base  : 3;
   };

   template <std::size_t Length=1>
   struct opcode {
      opcode(uint32_t op) : op(op) {}
      std::size_t emit(uint8_t* b)const {
         if constexpr (Length == 2)
            b[1] = 0x0f;
         b[0] = op;
         return Length;
      }

      /*
      uint32_t pack()const {
         uint32_t ret;
         uint8_t* bytes = (uint8_t*)&ret;
         if constexpr (Length == 3)
            ret[3] = 0x0f;
         if constexpr (Length >= 2)
            ret[2] = 0x38;
         if constexpr (Length >= 1)
            ret[1] = 0x3a;
         ret[0] = op;
         return ret;
      }
      */

      uint8_t op = 0;
   };
/*
   //template <std::size_t
   struct instruction {
      opcode op;
      modrm  m;
      sib    s;
      uint32_t displacement;
      uint32_t immediate;
   };
*/

#ifdef INSTRUCTION
#error "INSTRUCTION should not be defined"
#endif

// H -> handler
#ifdef H1
#error "H1 should not be defined"
#endif
#define H1(Ty) [&](Ty operand)

#ifdef H2
#error "H2 should not be defined"
#endif
#define H2(Ty1, Ty2) [&](Ty1 operand1, Ty2 operand2)

// M -> mnenomic
// addressing modes and handlers
#define INSTRUCTION(M, ...)                                     \
   template <typename... Ts>                                    \
   auto emit_##M (Ts&&... operands) {                           \
      overloaded{ __VA_ARGS__ }(std::forward<Ts>(operands)...); \
   }

   //TODO add a type for indirect registers
   class encoder {
      public:
         encoder(uint8_t* b) : block(b) {}

         void set_block_ptr(uint8_t* bp) { block = bp; }
         void* current_loc()const { return static_cast<void*>(block); }

         template <typename T>
         std::size_t emit(T&& value, std::size_t rep=1) {
            auto size = sizeof(T)*rep;
            for (std::size_t i=0; i < rep; ++i) {
               memcpy(block, &value, size);
               block += sizeof(T);
            }
            return size;
         }

         INSTRUCTION(and,
               H2(mem_reg64, uint8_t) { // reg64, imm8
                  prefix::rex r{1, operand1.reg > 7, 0, 0}; // W set for 64 bit operand
                  emit(r.value);       // emit rex prefix
                  emit((uint8_t)0x83); // opcode
                  modrm m{0b11, /* register mode */
                          0b100, /* /4 extension */
                          operand1.reg};
                  emit(m.value);
                  emit(operand2);
               }
               );

         INSTRUCTION(call,
               H1(uint16_t) { // rel16off
                  emit((uint8_t)0xe8); // opcode
                  emit(operand); // displacement
               },
               H1(uint32_t) { // rel32off
                  emit((uint8_t)0xe8); //opcode
                  emit(operand); // displacement
               },
               H1(mem_reg16) { // reg16
                  emit((uint8_t)0xff); // opcode
                  modrm m{0b11, /* register mode */
                          0b010, /* /2 extension */
                          operand.reg};
                  emit(m.value);
               },
               H1(mem_reg32) { // reg32
                  emit((uint8_t)0xff); // opcode
                  modrm m{0b11, /* register mode */
                          0b010, /* /2 extension */
                          operand.reg};
                  emit(m.value);
               },
               H1(mem_reg64) { // reg64
                  emit((uint8_t)0xff); // opcode
                  modrm m{0b11, /* register mode */
                          0b010, /* /2 extension */
                          operand.reg};
                  emit(m.value);
               }
               );

         INSTRUCTION(cmp,
               H2(mem_reg32, uint32_t) { // reg32, imm32
                  emit((uint8_t)0x81); // opcode
                  modrm m{0b11, /* register mode */
                          0b111, /* /7 extension for immediate */
                          operand1.reg};
                  emit(m.value); // emit modrm byte
                  emit(operand2);
               }
               );

         INSTRUCTION(dec,
               H1(mem_reg8) { // reg8
                  emit((uint8_t)0xfe);
                  modrm m{0b11, /* register mode */
                          0b1, /* extension */
                          operand.reg};
                  emit(m.value);
               }
               );

         INSTRUCTION(mov,
               H2(reg32, uint32_t) { // reg32, imm32
                  emit((uint8_t)0xb8 + operand1.value); // opcode + register
                  emit(operand2);
               },
               H2(reg64, uint64_t) { // reg64, imm64
                  prefix::rex r{1, operand1.value > 7, 0, 0};
                  emit(r.value); // emit rex prefix
                  emit((uint8_t)0xb8+operand1.value); // opcode + r64
                  emit(operand2);
               },
               H2(mem_reg64, reg64) { // reg/mem64, reg64
                  prefix::rex r{1, operand1.reg > 7, 0, operand2.value > 7};
                  emit(r.value); // emit rex prefix
                  emit((uint8_t)0x89); // opcode
                  modrm m{0xb11, /* register mode */
                          operand1.reg,
                          operand2.value};
                  emit(m.value);
               },
               H2(reg16, mem_reg16) { // reg16, reg/mem16
                  emit((uint8_t)0x8b);
               );

         INSTRUCTION(je,
               H1(uint8_t) { // rel8off
                  emit((uint8_t)0x74); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               },
               H1(uint16_t) { // rel16off
                  emit((uint8_t)0x0f); // prefix
                  emit((uint8_t)0x84); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               },
               H2(uint32_t) { // rel32off
                  emit((uint8_t)0x0f); // prefix
                  emit((uint8_t)0x84); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               }
               );

         INSTRUCTION(jnz,
               H1(uint8_t) { // rel8off
                  emit((uint8_t)0x75);
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               },
               H1(uint16_t) { // rel16off
                  emit((uint8_t)0x0f); // prefix
                  emit((uint8_t)0x85); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               },
               H1(uint32_t) { // rel32off
                  emit((uint8_t)0x0f); // prefix
                  emit((uint8_t)0x85); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               }
               );

         INSTRUCTION(jmp,
               H1(uint8_t) { // rel8off
                  emit((uint8_t)0xe9); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               },
               H1(uint16_t) { // rel16off
                  emit((uint8_t)0xe9); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               },
               H1(uint32_t) { // rel32off
                  emit((uint8_t)0xe9); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               },
               );

         INSTRUCTION(pop,
               H1(reg16) { // reg16
                  emit((uint8_t)0x58 + operand.value);
               },
               H1(reg32) { // reg32
                  emit((uint8_t)0x58 + operand.value);
               },
               H1(reg64) { // reg64
                  emit((uint8_t)0x58 + operand.value);
               }
               );

         INSTRUCTION(push,
               H1(reg16) { // reg16
                  emit((uint8_t)0x50 + operand.value); // opcode + register
               },
               H1(reg32) { // reg32
                  emit((uint8_t)0x50 + operand.value); // opcode + register
               },
               H1(reg64) { // reg64
                  emit((uint8_t)0x50 + operand.value); // opcode + register
               }
               );

         INSTRUCTION(xor,
               H2(mem_reg8, reg8) { // reg8, reg8
                  emit((uint8_t)0x30); // opcode
                  modrm m{0b11, /* register mode */
                          operand1.reg,
                          operand2.value};
                  emit(m.value);
               },
               H2(mem_reg16, reg16) { // reg16, reg16
                  emit((uint8_t)0x31); // opcode
                  modrm m{0b11, /* register mode */
                          operand1.reg,
                          operand2.value};
                  emit(m.value);
               },
               H2(mem_reg32, reg32) { // reg32, reg32
                  emit((uint8_t)0x31); // opcode
                  modrm m{0b11, /* register mode */
                          operand1.reg,
                          operand2.value};
                  emit(m.value);
               },
               H2(mem_reg64, reg64) { // reg64, reg64
                  prefix::rex r{1, operand1.value > 7, 0, operand2.value > 7};
                  emit((uint8_t)0x31); // opcode
                  modrm m{0b11, /* register mode */
                          operand1.reg,
                          operand2.value};
                  emit(m.value);
               },

      private:
         uint8_t* block;
   };

#undef INSTRUCTION
#undef H1
#undef H2

}}} // namespace eosio::vm::x86_64
