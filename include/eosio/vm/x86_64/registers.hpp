#pragma once

namespace eosio { namespace vm { namespace x86_64 {
   template <std::size_t Bits>
   struct reg {
      constexpr reg(uint8_t r) : value(r) {}
      constexpr explicit operator uint8_t() const { return value; }
      constexpr uint8_t get_value() const { return value & 0x7; }
      constexpr bool is_extended() const { return value > 0x7; }
      uint8_t value;
      static constexpr uint8_t size = Bits;
   };

   namespace registers {
      // define 8 bit registers
      inline constexpr reg<8> al  = {0b000};
      inline constexpr reg<8> ah  = {0b100};
      inline constexpr reg<8> cl  = {0b001};
      inline constexpr reg<8> ch  = {0b101};
      inline constexpr reg<8> dl  = {0b010};
      inline constexpr reg<8> dh  = {0b110};
      inline constexpr reg<8> bl  = {0b011};
      inline constexpr reg<8> bh  = {0b111};
      inline constexpr reg<8> spl = {0b100};
      inline constexpr reg<8> bpl = {0b101};
      inline constexpr reg<8> sil = {0b110};
      inline constexpr reg<8> dil = {0b111};
      inline constexpr reg<8> r8b = {0b1000};
      inline constexpr reg<8> r9b = {0b1001};
      inline constexpr reg<8> r10b = {0b1010};
      inline constexpr reg<8> r11b = {0b1011};
      inline constexpr reg<8> r12b = {0b1100};
      inline constexpr reg<8> r13b = {0b1101};
      inline constexpr reg<8> r14b = {0b1110};
      inline constexpr reg<8> r15b = {0b1111};

      // define 16 bit registers
      inline constexpr reg<16> ax  = {0b000};
      inline constexpr reg<16> cx  = {0b001};
      inline constexpr reg<16> dx  = {0b010};
      inline constexpr reg<16> bx  = {0b011};
      inline constexpr reg<16> sp = {0b100};
      inline constexpr reg<16> bp = {0b101};
      inline constexpr reg<16> si = {0b110};
      inline constexpr reg<16> di = {0b111};
      inline constexpr reg<16> r8w = {0b1000};
      inline constexpr reg<16> r9w = {0b1001};
      inline constexpr reg<16> r10w = {0b1010};
      inline constexpr reg<16> r11w = {0b1011};
      inline constexpr reg<16> r12w = {0b1100};
      inline constexpr reg<16> r13w = {0b1101};
      inline constexpr reg<16> r14w = {0b1110};
      inline constexpr reg<16> r15w = {0b1111};

      // define 32 bit registers
      inline constexpr reg<32> eax  = {0b000};
      inline constexpr reg<32> ecx  = {0b001};
      inline constexpr reg<32> edx  = {0b010};
      inline constexpr reg<32> ebx  = {0b011};
      inline constexpr reg<32> esp = {0b100};
      inline constexpr reg<32> ebp = {0b101};
      inline constexpr reg<32> esi = {0b110};
      inline constexpr reg<32> edi = {0b111};
      inline constexpr reg<32> r8d = {0b1000};
      inline constexpr reg<32> r9d = {0b1001};
      inline constexpr reg<32> r10d = {0b1010};
      inline constexpr reg<32> r11d = {0b1011};
      inline constexpr reg<32> r12d = {0b1100};
      inline constexpr reg<32> r13d = {0b1101};
      inline constexpr reg<32> r14d = {0b1110};
      inline constexpr reg<32> r15d = {0b1111};

      // define 64 bit registers
      inline constexpr reg<64> rax  = {0b000};
      inline constexpr reg<64> rcx  = {0b001};
      inline constexpr reg<64> rdx  = {0b010};
      inline constexpr reg<64> rbx  = {0b011};
      inline constexpr reg<64> rsp = {0b100};
      inline constexpr reg<64> rbp = {0b101};
      inline constexpr reg<64> rsi = {0b110};
      inline constexpr reg<64> rdi = {0b111};
      inline constexpr reg<64> r8 = {0b1000};
      inline constexpr reg<64> r9 = {0b1001};
      inline constexpr reg<64> r10 = {0b1010};
      inline constexpr reg<64> r11 = {0b1011};
      inline constexpr reg<64> r12 = {0b1100};
      inline constexpr reg<64> r13 = {0b1101};
      inline constexpr reg<64> r14 = {0b1110};
      inline constexpr reg<64> r15 = {0b1111};
   } // namespace registers

   using reg8  = reg<8>;
   using reg16 = reg<16>;
   using reg32 = reg<32>;
   using reg64 = reg<64>;
}}} // namespace eosio::vm::x86_64
