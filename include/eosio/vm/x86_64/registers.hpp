#pragma once

namespace eosio { namespace vm { namespace x86_64 {
   struct regs {
      static constexpr uint8_t al  = 0b000;
      static constexpr uint8_t ah  = 0b100;
      static constexpr uint8_t ax  = 0b000;
      static constexpr uint8_t eax = 0b000;
      static constexpr uint8_t rax = 0b000;

      static constexpr uint8_t cl  = 0b001;
      static constexpr uint8_t ch  = 0b101;
      static constexpr uint8_t cx  = 0b001;
      static constexpr uint8_t ecx = 0b001;
      static constexpr uint8_t rcx = 0b001;

      static constexpr uint8_t dl  = 0b010;
      static constexpr uint8_t dh  = 0b110;
      static constexpr uint8_t dx  = 0b010;
      static constexpr uint8_t edx = 0b010;
      static constexpr uint8_t rdx = 0b010;

      static constexpr uint8_t bl  = 0b011;
      static constexpr uint8_t bh  = 0b111;
      static constexpr uint8_t bx  = 0b011;
      static constexpr uint8_t ebx = 0b011;
      static constexpr uint8_t rbx = 0b011;

      static constexpr uint8_t spl = 0b100;
      static constexpr uint8_t sp  = 0b100;
      static constexpr uint8_t esp = 0b100;
      static constexpr uint8_t rsp = 0b100;

      static constexpr uint8_t bpl = 0b101;
      static constexpr uint8_t bp  = 0b101;
      static constexpr uint8_t ebp = 0b101;
      static constexpr uint8_t rbp = 0b101;

      static constexpr uint8_t sil = 0b110;
      static constexpr uint8_t si  = 0b110;
      static constexpr uint8_t esi = 0b110;
      static constexpr uint8_t rsi = 0b110;

      static constexpr uint8_t dil = 0b111;
      static constexpr uint8_t di  = 0b111;
      static constexpr uint8_t edi = 0b111;
      static constexpr uint8_t rdi = 0b111;

      static constexpr uint8_t r8b = 0b1000;
      static constexpr uint8_t r8w = 0b1000;
      static constexpr uint8_t r8d = 0b1000;
      static constexpr uint8_t r8  = 0b1000;

      static constexpr uint8_t r9b = 0b1001;
      static constexpr uint8_t r9w = 0b1001;
      static constexpr uint8_t r9d = 0b1001;
      static constexpr uint8_t r9  = 0b1001;

      static constexpr uint8_t r10b = 0b1010;
      static constexpr uint8_t r10w = 0b1010;
      static constexpr uint8_t r10d = 0b1010;
      static constexpr uint8_t r10  = 0b1010;

      static constexpr uint8_t r11b = 0b1011;
      static constexpr uint8_t r11w = 0b1011;
      static constexpr uint8_t r11d = 0b1011;
      static constexpr uint8_t r11  = 0b1011;

      static constexpr uint8_t r12b = 0b1100;
      static constexpr uint8_t r12w = 0b1100;
      static constexpr uint8_t r12d = 0b1100;
      static constexpr uint8_t r12  = 0b1100;

      static constexpr uint8_t r13b = 0b1101;
      static constexpr uint8_t r13w = 0b1101;
      static constexpr uint8_t r13d = 0b1101;
      static constexpr uint8_t r13  = 0b1101;

      static constexpr uint8_t r14b = 0b1110;
      static constexpr uint8_t r14w = 0b1110;
      static constexpr uint8_t r14d = 0b1110;
      static constexpr uint8_t r14  = 0b1110;

      static constexpr uint8_t r15b = 0b1111;
      static constexpr uint8_t r15w = 0b1111;
      static constexpr uint8_t r15d = 0b1111;
      static constexpr uint8_t r15  = 0b1111;
   };

   // types used to represent a register operand or indirect register operand
   struct reg8 {
      constexpr reg8(uint8_t r) : reg(r) {}
      uint8_t reg;
      static constexpr uint8_t size = 8;
   };
   struct reg16 {
      constexpr reg16(uint8_t r) : reg(r) {}
      uint8_t reg;
      static constexpr uint8_t size = 16;
   };
   struct reg32 {
      constexpr reg32(uint8_t r) : reg(r) {}
      uint8_t reg;
      static constexpr uint8_t size = 32;
   };
   struct reg64 {
      constexpr reg64(uint8_t r) : reg(r) {}
      uint8_t reg;
      static constexpr uint8_t size = 64;
   };

}}} // namespace eosio::vm::x86_64
