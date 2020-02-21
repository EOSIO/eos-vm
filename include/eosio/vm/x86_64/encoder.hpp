#pragma once

#include <eosio/vm/exceptions.hpp>
#include <eosio/vm/utils.hpp>
#include <eosio/vm/variant.hpp>
#include <eosio/vm/x86_64/types.hpp>

#include <limits>

namespace eosio { namespace vm { namespace x86_64 {
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

         template <typename T, typename U>
         void emit_legacy_prefix(T&& op1, U&& op2) {
            bool has_emitted_oso = false;
            bool has_emitted_aso = false;
            const auto& compute_for_prefix = [&](const auto& op) {
               if (is_reg(op)) {
                  const auto& r1 = get_reg(op);
                  if (r1.size == 16 && default_32 && !has_emitted_oso) {
                     emit(prefix::operand_size_override);
                     has_emitted_oso = true;
                  }
               } else if (is_mem(op)) {
                  const auto& m1 = get_mem(op);
                  if (m1.value.base.size == 32 && !has_emitted_aso) {
                     emit(prefix::address_size_override);
                     has_emitted_aso = true;
                  }
               }
            };

            compute_for_prefix(op1);
            compute_for_prefix(op2);
         }

         template <typename T, typename U>
         inline void emit_rex_prefix(T&& op1, U&& op2) {
            prefix::rex rexp;
            bool should_emit = false;
            const auto& compute_reg = [&](const auto& r) {
               rexp.W = r.size == 64;
               rexp.R = r.reg > 7;
            };
            const auto& compute_mem = [&](const auto& m) {
               rexp.W = !m.is_32_bit;
               rexp.B = m.sib.base > 7;
               rexp.X = m.sib.index > 7;
            };

            if (!mode_64) {
               if (is_reg(op1))
                  compute_reg(get_reg(op1));
               if (is_reg(op2))
                  compute_reg(get_reg(op2));
               if (is_mem(op1))
                  compute_mem(get_mem(op1));
               if (is_mem(op2))
                  compute_mem(get_mem(op2));
            }
         }

         template <typename T, typename U>
         inline void emit_instruction(T&& op1, U&& op2, bool default_32, bool mode_64) {
            this->default_32 = default_32;
            this->mode_64 = mode_64;
            emit_legacy_prefix(std::forward<T>(op1), std::forward<U>(op2));
            emit_rex_prefix(std::forward<T>(op1), std::forward<U>(op2));
         }

         template <typename T>
         inline void emit_operand(T&& t, uint8_t extension) {
            if (is_mem(t))
               emit_modrm(get_mem(t), extension);
            else
               emit_modrm(get_reg(t), extension);
         }

         inline void emit_modrm(uint8_t reg, uint8_t extension) {
            emit(modrm{0b11, extension, reg}.value);
         }

         template <typename Mem>
         inline void emit_modrm(Mem&& m, uint8_t extension) {
            emit(modrm{0b00, extension, m.base}.value);
         }

         inline void emit_rex(uint8_t reg) { emit(prefix::rex{ 1, reg > 7, 0, 0}.value); }

         inline void emit_opcode(uint8_t op) { emit(op); }

         template <typename T, typename U>
         inline void emit_and_impl(T&& op1, U&& op2, uint8_t opcode) {
            if constexpr (std::is_same_v<std::decay_t<T>, mem_reg16>)
               emit(prefix::operand_size_override);
            if constexpr (std::is_same_v<std::decay_t<T>, mem_reg64>)
               emit_rex(get_reg(op1));
            emit_opcode(opcode);
            emit_operand(op1, 4);
            emit(op2);
         }

         INSTRUCTION(and,
               H2(mem_reg8,  uint8_t)  { emit_and_impl(std::move(operand1), std::move(operand2), 0x80); }, // r/m8, imm8
               H2(mem_reg16, uint16_t) { emit_and_impl(std::move(operand1), std::move(operand2), 0x81); }, // r/m16, imm16
               H2(mem_reg32, uint32_t) { emit_and_impl(std::move(operand1), std::move(operand2), 0x81); }, // r/m32, imm32
               H2(mem_reg64, uint32_t) { emit_and_impl(std::move(operand1), std::move(operand2), 0x81); }, // r/m64, imm32
               H2(mem_reg16, uint8_t)  { emit_and_impl(std::move(operand1), std::move(operand2), 0x83); }, // r/m16, imm8
               H2(mem_reg32, uint8_t)  { emit_and_impl(std::move(operand1), std::move(operand2), 0x83); }, // r/m32, imm8
               H2(mem_reg64, uint8_t)  { emit_and_impl(std::move(operand1), std::move(operand2), 0x83); }  // r/m64, imm8
         );

         INSTRUCTION(call,
               H1(mem_reg64) { // call indirect
                  emit_opcode(0xFF);
                  emit_modrm(get_reg(operand), 2);
               }
         );

         INSTRUCTION(ret,
               H0() { // near return
                  emit_opcode(0xC3);
               },
               H1(uint16_t) { // return pop imm16 bytes
                  emit_opcode(0xC2);
                  emit(operand);
               }
         );

         INSTRUCTION(int3,
               H0() { // break point
                  emit_opcode(0xCC);
               }
         );

            #if 0
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
               H1(uint32_t) { // rel32off
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
               H1(uint32_t) { // rel32off
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
               H1(uint32_t) { // rel32off
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
               }
               );
#endif
      private:
         uint8_t* block;
         bool     default_32 = true;
         bool     mode_64 = false;
   };

#undef INSTRUCTION
#undef H0
#undef H1
#undef H2

}}} // namespace eosio::vm::x86_64
