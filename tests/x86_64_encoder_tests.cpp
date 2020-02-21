#include <eosio/vm/allocator.hpp>
#include <eosio/vm/x86_64/encoder.hpp>

#include <catch2/catch.hpp>

#include <iostream>

using namespace eosio;
using namespace eosio::vm;
using namespace eosio::vm::x86_64;

template <typename T>
int _to_bytes(T t, std::vector<uint8_t>& vec) {
   if constexpr (sizeof(T) == 1) {
      vec.push_back((uint8_t)t);
   } else if constexpr (sizeof(T) == 2) {
      vec.push_back((uint8_t)(t & 0xFF));
      vec.push_back((uint8_t)((t >> 8) & 0xFF));
   } else if constexpr (sizeof(T) == 4) {
      vec.push_back((uint8_t)(t & 0xFF));
      vec.push_back((uint8_t)((t >> 8) & 0xFF));
      vec.push_back((uint8_t)((t >> 16) & 0xFF));
      vec.push_back((uint8_t)((t >> 24) & 0xFF));
   } else {
      vec.push_back((uint8_t)(t & 0xFF));
      vec.push_back((uint8_t)((t >> 8) & 0xFF));
      vec.push_back((uint8_t)((t >> 16) & 0xFF));
      vec.push_back((uint8_t)((t >> 24) & 0xFF));
      vec.push_back((uint8_t)((t >> 32) & 0xFF));
      vec.push_back((uint8_t)((t >> 40) & 0xFF));
      vec.push_back((uint8_t)((t >> 48) & 0xFF));
      vec.push_back((uint8_t)((t >> 56) & 0xFF));
   }
   return 0;
}

template <typename... Ts>
struct to_bytes_impl;

template <>
struct to_bytes_impl<> {
   static void value(std::vector<uint8_t>&) {}
};

template <typename T, typename... Ts>
struct to_bytes_impl<T, Ts...> {
   static void value(std::vector<uint8_t>& v, T t, Ts... ts) {
      _to_bytes(t, v);
      ::to_bytes_impl<Ts...>::value(v, ts...);
   }
};

template <typename T>
struct to_bytes_impl<T> {
   static void value(std::vector<uint8_t>& v, T t) {
      _to_bytes(t, v);
   }
};

template <typename... Ts>
std::vector<uint8_t> to_bytes(Ts... args) {
   std::vector<uint8_t> ret;
   to_bytes_impl<Ts...>::value(ret, args...);
   return ret;
}

uint8_t  u8(uint8_t v) { return v; }
uint16_t u16(uint16_t v) { return v; }
uint32_t u32(uint32_t v) { return v; }
uint64_t u64(uint64_t v) { return v; }

void (*test_ptr)();

#define TRY_EMIT(Op, Value, ...)                    \
   {                                                \
      auto bytes = Value;                           \
      buff = (uint8_t*)alloc.start_code(); \
      encoder(buff).emit_ ## Op(__VA_ARGS__);       \
      for (uint32_t i=0; i < bytes.size(); ++i) {   \
         CHECK((int)bytes[i] == (int)buff[i]);      \
      }                                             \
      encoder(buff+bytes.size()).emit_int3();        \
      encoder(buff+bytes.size()+1).emit_ret();        \
      alloc.end_code<true>();                       \
         }

#define BYTES(...) \
   to_bytes(__VA_ARGS__)

TEST_CASE("Testing x86_64 `and` instruction", "[x86_64 `and` instruction]") {

   growable_allocator alloc(64*1024);
   alloc.alloc<uint8_t>(64*1024);

   uint8_t* buff;
   // r/m8 AND imm8
   TRY_EMIT(and, BYTES(u8(0x80), u8(0xe4), u8(8)),   reg8{regs::ah}, u8(8));
   void (*t)() = (void(*)())alloc._code_base;
   (*t)();

   TRY_EMIT(and, BYTES(u8(0x80), u8(0xe5), u8(32)),  reg8{regs::ch}, u8(32));
   TRY_EMIT(and, BYTES(u8(0x80), u8(0xe2), u8(64)),  reg8{regs::dl}, u8(64));
   TRY_EMIT(and, BYTES(u8(0x80), u8(0x23), u8(8)),   mem8{reg64{regs::rbx}}, u8(8));
   TRY_EMIT(and, BYTES(u8(0x80), u8(0x20), u8(200)), mem8{reg64{regs::rax}}, u8(200));

   // r/m16 AND imm16
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x81), u8(0xe3), u16(0xFFF)),  reg16{regs::bx}, u16(0xFFF));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x81), u8(0xe2), u16(0xFFF)),  reg16{regs::dx}, u16(0xFFF));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x81), u8(0xe2), u16(0xFFFF)), reg16{regs::dx}, u16(0xFFFF));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x81), u8(0x20), u16(200)),    mem16{reg64{regs::rax}}, u16(200));

   // r/m32 AND imm32
   TRY_EMIT(and, BYTES(u8(0x81), u8(0xe2), u32(0xFFFF)),     reg32{regs::edx}, u32(0xFFFF));
   TRY_EMIT(and, BYTES(u8(0x81), u8(0xe7), u32(0xFFFF)),     reg32{regs::edi}, u32(0xFFFF));
   TRY_EMIT(and, BYTES(u8(0x81), u8(0xe7), u32(0xFFFFFFFF)), reg32{regs::edi}, u32(0xFFFFFFFF));

   // r/m64 AND imm32
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x81), u8(0xe0), u32(125)),        reg64{regs::rax}, u32(125));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x81), u8(0xe2), u32(500)),        reg64{regs::rdx}, u32(500));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x81), u8(0xe2), u32(0xFFFFFFFF)), reg64{regs::rdx}, u32(0xFFFFFFFF));

   // r/m16 AND imm8
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x83), u8(0xe0), u8(8)),    reg16{regs::ax}, u8(8));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x83), u8(0xe2), u8(8)),    reg16{regs::dx}, u8(8));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x83), u8(0xe0), u8(0xFE)), reg16{regs::ax}, u8(0xFE));

   // r/m32 AND imm8
   TRY_EMIT(and, BYTES(u8(0x83), u8(0xe0), u8(8)),    reg32{regs::eax}, u8(8));
   TRY_EMIT(and, BYTES(u8(0x83), u8(0xe2), u8(8)),    reg32{regs::edx}, u8(8));
   TRY_EMIT(and, BYTES(u8(0x83), u8(0xe0), u8(0xFE)), reg32{regs::eax}, u8(0xFE));

   // r/m64 AND imm8
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x83), u8(0xe0), u8(0x8)),  reg64{regs::rax}, u8(8));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x83), u8(0xe2), u8(0x8)),  reg64{regs::rdx}, u8(8));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x83), u8(0xe2), u8(0xFF)), reg64{regs::rdx}, u8(0xFF));

   // call indirect
   TRY_EMIT(call, BYTES(u8(0xFF), u8(0xd0)), reg64{regs::rax});

   // ret
   TRY_EMIT(ret, BYTES(u8(0xc3)));
   TRY_EMIT(ret, BYTES(u8(0xC2), u16(200)), u16(200));
}
