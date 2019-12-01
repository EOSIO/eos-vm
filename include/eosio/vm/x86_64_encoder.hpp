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
      r15 = 15,
      rip
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

   struct optional_ty {};

   struct reg8 {
      constexpr reg8(regs r) : value(static_cast<uint8_t>(r)) {}
      uint8_t reg;
   };
   struct reg16 {
      constexpr reg16(regs r) : value(static_cast<uint8_t>(r)) {}
      uint8_t reg;
   };
   struct reg32 {
      constexpr reg32(regs r) : value(static_cast<uint8_t>(r)) {}
      uint8_t reg;
   };
   struct reg64 {
      constexpr reg64(regs r) : value(static_cast<uint8_t>(r)) {}
      uint8_t reg;
   };

   template <typename T>
   inline constexpr bool is_reg() { return std::is_same_v<T, reg8>  ||
                                           std::is_same_v<T, reg16> ||
                                           std::is_same_v<T, reg32> ||
                                           std::is_same_v<T, reg64>; }

   template <typename T>
   inline constexpr bool is_reg_v = is_reg<T>();

   struct sib {
      sib(uint8_t s, uint8_t i, uint8_t b) : scale(s), index(i), base(b) {}
      sib(uint8_t val) : value(val) {}
      union {
         struct {
            uint8_t base  : 3;
            uint8_t index : 3;
            uint8_t scale : 2;
         };
      };
      uint8_t value;
   };

   struct mem {
      template <typename Reg1, typename Reg2=optional_ty>
      constexpr mem(Reg1 base, Reg2 index, uint8_t scale)
         : value(scale, index.reg, base.reg) {}
      sib value = sib{0};
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

   template <typename T>
   inline constexpr bool is_mem_reg() { return std::is_same_v<T, mem_reg8>  ||
                                               std::is_same_v<T, mem_reg16> ||
                                               std::is_same_v<T, mem_reg32> ||
                                               std::is_same_v<T, mem_reg64>; }

   template <typename T>
   inline constexpr bool is_mem_reg_v = is_mem_reg<T>();

   struct mem8 {
      constexpr mem8(regs r, uint8_t o) : reg(static_cast<uint8_t>(r)), off(o) {}
      uint8_t reg = 0;
      uint8_t off = 0;
   };
   struct mem16 {
      constexpr mem16(regs r, uint8_t o) : reg(static_cast<uint8_t>(r)), off(o) {}
      uint8_t  reg = 0;
      uint16_t off = 0;
   };
   struct mem32 {
      constexpr mem32(regs r, uint8_t o) : reg(static_cast<uint8_t>(r)), off(o) {}
      uint8_t  reg = 0;
      uint32_t off = 0;
   };
   struct mem64 {
      constexpr mem64(regs r, uint8_t o) : reg(static_cast<uint8_t>(r)), off(o) {}
      uint8_t  reg = 0;
      uint64_t off = 0;
   };

   template <typename T>
   inline constexpr bool is_mem() { return std::is_same_v<T, mem8>  ||
                                           std::is_same_v<T, mem16> ||
                                           std::is_same_v<T, mem32> ||
                                           std::is_same_v<T, mem64>; }

   template <typename T>
   inline constexpr bool is_mem_v = is_mem<T>();



   struct modrm {
      template <typename T, typename U=optional_ty, uint8_t Extension=0xFF>
      modrm(T operand1, U operand2={}) {
         constexpr bool is_reg1 = is_reg_v<T> || (is_mem_reg_v<T>;
         constexpr bool is_reg2 = is_reg_v<U> || is_mem_reg_v<U>;
         mod = 0;
         rm = 0;
         reg = 0;
         if constexpr (is_reg_or_mem_reg1) {
            if (Extension != 0xFF)
               reg = operand1.reg;
            else {
               reg = Extension;
               rm = operand1.reg;
            }
         }
         if constexpr (is_reg_or_mem_reg2) {
            rm = operand2.reg;
         }
         if constexpr ((is_reg_or_mem_reg1 && is_reg_or_mem_reg2) ||
                       (is_reg_or_mem_reg1 && std::is_same_v<U, optional_modrm_ty>)) {
            mod = 0b11; /* register mode */
         } else if constexpr (
      }

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


#ifdef INSTRUCTION
#error "INSTRUCTION should not be defined"
#endif

// H -> handler
#ifdef H0
#error "H0 should not be defined"
#endif
#define H0() [&]()

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

   //TODO this is by no means perfect
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
               },
               H2(mem_reg64, uint32_t) { // reg/mem64, imm32
                  prefix::rex r{1, operand1.reg > 7, 0, 0}; // W set for 64 bit operand
                  emit(r.value);       // emit rex prefix
                  emit((uint8_t)0x81); // opcode
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
               H1(uint8_t) { // AL, imm8
                  emit((uint8_t)0x3c); // opcode
                  emit(operand);
               },
               H1(uint16_t) { // AX, imm16
                  emit((uint8_t)0x3d);
                  emit(operand);
               },
               H1(uint32_t) { // EAX, imm32
                  emit((uint8_t)0x3d);
                  emit(operand);
               },
               H1(uint64_t) { // RAX, imm64
                  /* TODO  handle RAX in above case */
               },
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

         INSTRUCTION(div,
               H1(mem_reg8) { // reg/mem8
                  emit((uint8_t)0xf6); // opcode
                  modrm m{0b11,  /* register mode */
                          0b110, /* extension */
                          operand.reg};
                  emit(m.value);
               },
               H1(mem_reg16) { // reg/mem16
                  emit((uint8_t)0xf7); // opcode
                  modrm m{0b11,  /* register mode */
                          0b110, /* extension */
                          operand.reg};
                  emit(m.value);
               },
               H1(mem_reg32) { // reg/mem32
                  emit((uint8_t)0xf7); // opcode
                  modrm m{0b11,  /* register mode */
                          0b110, /* extension */
                          operand.reg};
                  emit(m.value);
               },
               H1(mem_reg64) { // reg/mem64
                  prefix::rex r{1, operand.reg > 7, 0, 0};
                  emit(r.value);
                  emit((uint8_t)0xf7); // opcode
                  modrm m{0b11,  /* register mode */
                          0b110, /* extension */
                          operand.reg};
                  emit(m.value);
               }
               );

         INSTRUCTION(mov,
               H2(reg32, uint32_t) { // reg32, imm32
                  emit((uint8_t)0xb8 + operand1.reg); // opcode + register
                  emit(operand2);
               },
               H2(reg64, uint64_t) { // reg64, imm64
                  prefix::rex r{1, operand1.reg > 7, 0, 0};
                  emit(r.value); // emit rex prefix
                  emit((uint8_t)0xb8+operand1.reg); // opcode + r64
                  emit(operand2);
               },
               H2(mem_reg64, reg64) { // reg/mem64, reg64
                  prefix::rex r{1, operand1.reg > 7, 0, operand2.reg > 7};
                  emit(r.value); // emit rex prefix
                  emit((uint8_t)0x89); // opcode
                  modrm m{0xb11, /* register mode */
                          operand1.reg,
                          operand2.reg};
                  emit(m.value);
               },
               H2(reg16, mem_reg16) { // reg16, reg/mem16
                  emit((uint8_t)0x8b);
                  sib s{0b0, // scale 1
                        operand2.reg,
                        operand2.reg};
                  modrm m{0b0,
                          operand1.reg,
                          0b100}; // use sib
                  emit(m.value);
                  emit(s.value);
               },
               H2(reg32, mem_reg32) { // reg32, reg/mem32
                  emit((uint8_t)0x8b);
                  sib s{0b0, // scale 1
                        operand2.reg,
                        operand2.reg};
                  modrm m{0b0,
                          operand1.reg,
                          0b100}; // use sib
                  emit(m.value);
                  emit(s.value);
               },
               H2(reg64, mem_reg64) { // reg64, reg/mem64
                  prefix::rex r{1, operand1.reg > 7, operand2.reg > 7, operand2.reg > 7};
                  emit(r.value);
                  emit((uint8_t)0x8b);
                  sib s{0b0, // scale 1
                        operand2.reg,
                        operand2.reg};
                  modrm m{0b0,
                          operand1.reg,
                          0b100}; // use sib
                  emit(m.value);
                  emit(s.value);
               },
               );

      INSTRUCTION(jae,
               H1(uint8_t) { // rel8off
                  emit((uint8_t)0x73); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               },
               H1(uint16_t) { // rel16off
                  emit((uint8_t)0x03); // prefix
                  emit((uint8_t)0x83); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               },
               H2(uint32_t) { // rel32off
                  emit((uint8_t)0x0f); // prefix
                  emit((uint8_t)0x83); // opcode
                  void* ret = (void*)block;
                  emit(operand);
                  return ret;
               }
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

         INSTRUCTION(jz,
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

         INSTRUCTION(lea,
               H2(reg16, mem32) { // reg16, mem
                  emit((uint8_t)0x8d); // opcode
                  bool is_rip = operand2.reg == static_cast<uint8_t>(regs::rip);
                  bool is_rbp = operand2.reg == static_cast<uint8_t>(regs::rbp);
                  uint8_t mod = is_rip ? 0b00 : is_rbp ? 0b01 : 0b11;
                  modrm m(mod,
                          operand1.reg,
                          is_rip || is_rbp ? 0b101 : operand2.reg);
                  emit(m.value);

         INSTRUCTION(pop,
               H1(reg16) { // reg16
                  emit((uint8_t)0x58 + operand.reg);
               },
               H1(reg32) { // reg32
                  emit((uint8_t)0x58 + operand.reg);
               },
               H1(reg64) { // reg64
                  emit((uint8_t)0x58 + operand.reg);
               }
               );

         INSTRUCTION(push,
               H1(reg16) { // reg16
                  emit((uint8_t)0x50 + operand.reg); // opcode + register
               },
               H1(reg32) { // reg32
                  emit((uint8_t)0x50 + operand.reg); // opcode + register
               },
               H1(reg64) { // reg64
                  emit((uint8_t)0x50 + operand.reg); // opcode + register
               }
               );

         INSTRUCTION(test,
               H2(mem_reg16, reg16) { // reg/mem16, reg16
                  emit((uint8_t)0x85); // opcode
                  modrm m(0b11, /* register mode */
                        operand1.reg,
                        operand2.reg);
                  emit(m.value);
               },
               H2(mem_reg32, reg32) { // reg/mem32, reg32
                  emit((uint8_t)0x85); // opcode
                  modrm m(0b11, /* register mode */
                        operand1.reg,
                        operand2.reg);
                  emit(m.value);
               },
               H2(mem_reg64, reg64) { // reg/mem64, reg64
                  emit((uint8_t)0x85); // opcode
                  modrm m(0b11, /* register mode */
                        operand1.reg,
                        operand2.reg);
                  emit(m.value);
               },
               );

         INSTRUCTION(ret,
               H0() {
                  emit((uint8_t)0xc3)
               }
               );

         INSTRUCTION(xor,
               H2(mem_reg8, reg8) { // reg8, reg8
                  emit((uint8_t)0x30); // opcode
                  modrm m{0b11, /* register mode */
                          operand1.reg,
                          operand2.reg};
                  emit(m.value);
               },
               H2(mem_reg16, reg16) { // reg16, reg16
                  emit((uint8_t)0x31); // opcode
                  modrm m{0b11, /* register mode */
                          operand1.reg,
                          operand2.reg};
                  emit(m.value);
               },
               H2(mem_reg32, reg32) { // reg32, reg32
                  emit((uint8_t)0x31); // opcode
                  modrm m{0b11, /* register mode */
                          operand1.reg,
                          operand2.reg};
                  emit(m.value);
               },
               H2(mem_reg64, reg64) { // reg64, reg64
                  prefix::rex r{1, operand1.reg > 7, 0, operand2.reg > 7};
                  emit((uint8_t)0x31); // opcode
                  modrm m{0b11, /* register mode */
                          operand1.reg,
                          operand2.reg};
                  emit(m.value);
               },

      private:
         uint8_t* block;
   };

#undef INSTRUCTION
#undef H0
#undef H1
#undef H2

}}} // namespace eosio::vm::x86_64
