#include <eosio/vm/allocator.hpp>
#include <eosio/vm/x86_64/encoder.hpp>

#include <eosio/vm/x86_64/writer.hpp>

#include <catch2/catch.hpp>

#include <iostream>

using namespace eosio;
using namespace eosio::vm;
using namespace eosio::vm::x86_64;
using namespace eosio::vm::x86_64::registers;

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
      uint8_t buff[512];                            \
      encoder(buff, 512).emit_ ## Op(__VA_ARGS__);       \
      for (uint32_t i=0; i < bytes.size(); ++i) {   \
         CHECK((int)bytes[i] == (int)buff[i]);      \
      }                                             \
   }

#define BYTES(...) \
   to_bytes(__VA_ARGS__)

TEST_CASE("Testing x86_64 `and` instruction", "[x86_64 `and` instruction]") {
   // r/m8 AND imm8
   TRY_EMIT(and, BYTES(u8(0x80), u8(0xe4), u8(8)),   ah,         u8(8));
   TRY_EMIT(and, BYTES(u8(0x80), u8(0xe5), u8(32)),  ch,         u8(32));
   TRY_EMIT(and, BYTES(u8(0x80), u8(0xe2), u8(64)),  dl,         u8(64));
   TRY_EMIT(and, BYTES(u8(0x80), u8(0x23), u8(8)),   mem8{rbx, 0}, u8(8));
   TRY_EMIT(and, BYTES(u8(0x80), u8(0x20), u8(200)), mem8{rax, 0}, u8(200));

   // r/m16 AND imm16
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x81), u8(0xe3), u16(0xFFF)),  bx,         u16(0xFFF));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x81), u8(0xe2), u16(0xFFF)),  dx,         u16(0xFFF));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x81), u8(0xe2), u16(0xFFFF)), dx,         u16(0xFFFF));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x81), u8(0x20), u16(200)),    mem16{rax, 0}, u16(200));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x81), u8(0x21), u16(200)),    mem16{rcx, 0}, u16(200));

   // r/m32 AND imm32
   TRY_EMIT(and, BYTES(u8(0x81), u8(0xe2), u32(0xFFFF)),     edx,        u32(0xFFFF));
   TRY_EMIT(and, BYTES(u8(0x81), u8(0xe7), u32(0xFFFF)),     edi,        u32(0xFFFF));
   TRY_EMIT(and, BYTES(u8(0x81), u8(0xe7), u32(0xFFFFFFFF)), edi,        u32(0xFFFFFFFF));
   TRY_EMIT(and, BYTES(u8(0x81), u8(0x20), u32(200)),        mem32{rax, 0}, u32(200));
   TRY_EMIT(and, BYTES(u8(0x81), u8(0x21), u32(200)),        mem32{rcx, 0}, u32(200));

   // r/m64 AND imm32
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x81), u8(0xe0), u32(125)),        rax,        u32(125));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x81), u8(0xe2), u32(500)),        rdx,        u32(500));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x81), u8(0xe2), u32(0xFFFFFFFF)), rdx,        u32(0xFFFFFFFF));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x81), u8(0x20), u32(200)),        mem64{rax, 0}, u32(200));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x81), u8(0x21), u32(200)),        mem64{rcx, 0}, u32(200));

   // r/m16 AND imm8
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x83), u8(0xe0), u8(8)),    ax,         u8(8));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x83), u8(0xe2), u8(8)),    dx,         u8(8));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x83), u8(0xe0), u8(0xFE)), ax,         u8(0xFE));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x83), u8(0x20), u8(200)),  mem16{rax, 0}, u8(200));
   TRY_EMIT(and, BYTES(u8(0x66), u8(0x83), u8(0x21), u8(200)),  mem16{rcx, 0}, u8(200));

   // r/m32 AND imm8
   TRY_EMIT(and, BYTES(u8(0x83), u8(0xe0), u8(8)),    eax,        u8(8));
   TRY_EMIT(and, BYTES(u8(0x83), u8(0xe2), u8(8)),    edx,        u8(8));
   TRY_EMIT(and, BYTES(u8(0x83), u8(0xe0), u8(0xFE)), eax,        u8(0xFE));
   TRY_EMIT(and, BYTES(u8(0x83), u8(0x20), u8(200)),  mem32{rax, 0}, u8(200));
   TRY_EMIT(and, BYTES(u8(0x83), u8(0x21), u8(200)),  mem32{rcx, 0}, u8(200));

   // r/m64 AND imm8
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x83), u8(0xe0), u8(0x8)),  rax,        u8(8));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x83), u8(0xe2), u8(0x8)),  rdx,        u8(8));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x83), u8(0xe2), u8(0xFF)), rdx,        u8(0xFF));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x83), u8(0x20), u8(200)),  mem64{rax, 0}, u8(200));
   TRY_EMIT(and, BYTES(u8(0x48), u8(0x83), u8(0x21), u8(200)),  mem64{rcx, 0}, u8(200));

}

TEST_CASE("Testing x86_64 `cmp` instruction", "[x86_64 `cmp` instruction]") {
   // r/m8 CMP imm8
   TRY_EMIT(cmp, BYTES(u8(0x80), u8(0xfc), u8(8)),   ah,         u8(8));
   TRY_EMIT(cmp, BYTES(u8(0x80), u8(0xfd), u8(32)),  ch,         u8(32));
   TRY_EMIT(cmp, BYTES(u8(0x80), u8(0xfa), u8(64)),  dl,         u8(64));
   TRY_EMIT(cmp, BYTES(u8(0x80), u8(0x3b), u8(8)),   mem8{rbx, 0}, u8(8));
   TRY_EMIT(cmp, BYTES(u8(0x80), u8(0x38), u8(200)), mem8{rax, 0}, u8(200));

   // r/m16 CMP imm16
   TRY_EMIT(cmp, BYTES(u8(0x66), u8(0x81), u8(0xfb), u16(0xFFF)),  bx,         u16(0xFFF));
   TRY_EMIT(cmp, BYTES(u8(0x66), u8(0x81), u8(0xfa), u16(0xFFF)),  dx,         u16(0xFFF));
   TRY_EMIT(cmp, BYTES(u8(0x66), u8(0x81), u8(0xfa), u16(0xFFFF)), dx,         u16(0xFFFF));
   TRY_EMIT(cmp, BYTES(u8(0x66), u8(0x81), u8(0x38), u16(200)),    mem16{rax, 0}, u16(200));
   TRY_EMIT(cmp, BYTES(u8(0x66), u8(0x81), u8(0x39), u16(200)),    mem16{rcx, 0}, u16(200));

   // r/m32 CMP imm32
   TRY_EMIT(cmp, BYTES(u8(0x81), u8(0xfa), u32(0xFFFF)),     edx,        u32(0xFFFF));
   TRY_EMIT(cmp, BYTES(u8(0x81), u8(0xff), u32(0xFFFF)),     edi,        u32(0xFFFF));
   TRY_EMIT(cmp, BYTES(u8(0x81), u8(0xff), u32(0xFFFFFFFF)), edi,        u32(0xFFFFFFFF));
   TRY_EMIT(cmp, BYTES(u8(0x81), u8(0x38), u32(200)),        mem32{rax, 0}, u32(200));
   TRY_EMIT(cmp, BYTES(u8(0x81), u8(0x39), u32(200)),        mem32{rcx, 0}, u32(200));

   // r/m64 CMP imm32
   TRY_EMIT(cmp, BYTES(u8(0x48), u8(0x81), u8(0xf8), u32(125)),        rax,        u32(125));
   TRY_EMIT(cmp, BYTES(u8(0x48), u8(0x81), u8(0xfa), u32(500)),        rdx,        u32(500));
   TRY_EMIT(cmp, BYTES(u8(0x48), u8(0x81), u8(0xfa), u32(0xFFFFFFFF)), rdx,        u32(0xFFFFFFFF));
   TRY_EMIT(cmp, BYTES(u8(0x48), u8(0x81), u8(0x38), u32(200)),        mem64{rax, 0}, u32(200));
   TRY_EMIT(cmp, BYTES(u8(0x48), u8(0x81), u8(0x39), u32(200)),        mem64{rcx, 0}, u32(200));

   // r/m16 CMP imm8
   TRY_EMIT(cmp, BYTES(u8(0x66), u8(0x83), u8(0xf8), u8(8)),    ax,         u8(8));
   TRY_EMIT(cmp, BYTES(u8(0x66), u8(0x83), u8(0xfa), u8(8)),    dx,         u8(8));
   TRY_EMIT(cmp, BYTES(u8(0x66), u8(0x83), u8(0xf8), u8(0xFE)), ax,         u8(0xFE));
   TRY_EMIT(cmp, BYTES(u8(0x66), u8(0x83), u8(0x38), u8(200)),  mem16{rax, 0}, u8(200));
   TRY_EMIT(cmp, BYTES(u8(0x66), u8(0x83), u8(0x39), u8(200)),  mem16{rcx, 0}, u8(200));

   // r/m32 CMP imm8
   TRY_EMIT(cmp, BYTES(u8(0x83), u8(0xf8), u8(8)),    eax,        u8(8));
   TRY_EMIT(cmp, BYTES(u8(0x83), u8(0xfa), u8(8)),    edx,        u8(8));
   TRY_EMIT(cmp, BYTES(u8(0x83), u8(0xf8), u8(0xFE)), eax,        u8(0xFE));
   TRY_EMIT(cmp, BYTES(u8(0x83), u8(0x38), u8(200)),  mem32{rax, 0}, u8(200));
   TRY_EMIT(cmp, BYTES(u8(0x83), u8(0x39), u8(200)),  mem32{rcx, 0}, u8(200));

   // r/m64 CMP imm8
   TRY_EMIT(cmp, BYTES(u8(0x48), u8(0x83), u8(0xf8), u8(0x8)),  rax,        u8(8));
   TRY_EMIT(cmp, BYTES(u8(0x48), u8(0x83), u8(0xfa), u8(0x8)),  rdx,        u8(8));
   TRY_EMIT(cmp, BYTES(u8(0x48), u8(0x83), u8(0xfa), u8(0xFF)), rdx,        u8(0xFF));
   TRY_EMIT(cmp, BYTES(u8(0x48), u8(0x83), u8(0x38), u8(200)),  mem64{rax, 0}, u8(200));
   TRY_EMIT(cmp, BYTES(u8(0x48), u8(0x83), u8(0x39), u8(200)),  mem64{rcx, 0}, u8(200));

}

TEST_CASE("Testing x86_64 `mov` instruction", "[x86_64 `mov` instruction]") {
   // r/m8 MOV r8
   TRY_EMIT(mov, BYTES(u8(0x88), u8(0xc4)), ah, al);
   TRY_EMIT(mov, BYTES(u8(0x88), u8(0xe0)), al, ah);
   TRY_EMIT(mov, BYTES(u8(0x88), u8(0x03)), mem8{rbx, 0}, al);
   TRY_EMIT(mov, BYTES(u8(0x88), u8(0x21)), mem8{rcx, 0}, ah);

   // r/m16 MOV r16
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0x89), u8(0xd8)), ax, bx);
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0x89), u8(0xd1)), cx, dx);
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0x89), u8(0x03)), mem16{rbx, 0}, ax);
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0x89), u8(0x11)), mem16{rcx, 0}, dx);

   // r/m32 MOV r32
   TRY_EMIT(mov, BYTES(u8(0x89), u8(0xd8)), eax, ebx);
   TRY_EMIT(mov, BYTES(u8(0x89), u8(0xf1)), ecx, esi);
   TRY_EMIT(mov, BYTES(u8(0x89), u8(0x03)), mem32{rbx, 0}, eax);
   TRY_EMIT(mov, BYTES(u8(0x89), u8(0x11)), mem32{rcx, 0}, edx);

   // r/m64 MOV r64
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0x89), u8(0xc8)), rax, rcx);
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0x89), u8(0xcc)), rsp, rcx);
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0x89), u8(0x03)), mem64{rbx, 0}, rax);
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0x89), u8(0x11)), mem64{rcx, 0}, rdx);

   // r8 MOV r/m8
   TRY_EMIT(mov, BYTES(u8(0x8a), u8(0x03)), al, mem8{rbx, 0});
   TRY_EMIT(mov, BYTES(u8(0x8a), u8(0x21)), ah, mem8{rcx, 0});

   // r16 MOV r/m16
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0x8b), u8(0x03)), ax, mem16{rbx, 0});
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0x8b), u8(0x0a)), cx, mem16{rdx, 0});

   // r32 MOV r/m32
   TRY_EMIT(mov, BYTES(u8(0x8b), u8(0x03)), eax, mem32{rbx, 0});
   TRY_EMIT(mov, BYTES(u8(0x8b), u8(0x0e)), ecx, mem32{rsi, 0});

   // r64 MOV r/m64
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0x8b), u8(0x01)), rax, mem64{rcx, 0});
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0x8b), u8(0x22)), rsp, mem64{rdx, 0});

   // r8 MOV imm8
   TRY_EMIT(mov, BYTES(u8(0xb3), u8(0x0f)), bl, u8(0x0f));
   TRY_EMIT(mov, BYTES(u8(0xb4), u8(0xff)), ah, u8(0xff));
   TRY_EMIT(mov, BYTES(u8(0xb6), u8(0x33)), dh, u8(0x33));

   // r16 MOV imm16
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0xbb), u16(0x0f)), bx, u16(0x0f));
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0xb8), u16(0xff)), ax, u16(0xff));
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0xba), u16(0x33)), dx, u16(0x33));

   // r32 MOV imm32
   TRY_EMIT(mov, BYTES(u8(0xbb), u32(0x0f)), ebx, u32(0x0f));
   TRY_EMIT(mov, BYTES(u8(0xb8), u32(0xff)), eax, u32(0xff));
   TRY_EMIT(mov, BYTES(u8(0xba), u32(0x33)), edx, u32(0x33));

   // r64 MOV imm64
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0xbb), u64(0x0f)), rbx, u64(0x0f));
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0xb8), u64(0xff)), rax, u64(0xff));
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0xba), u64(0x33)), rdx, u64(0x33));

   // r8 MOV imm8
   TRY_EMIT(mov, BYTES(u8(0xc6), u8(0x03), u8(0x0f)), mem8{rbx, 0}, u8(0x0f));
   TRY_EMIT(mov, BYTES(u8(0xc6), u8(0x00), u8(0xff)), mem8{rax, 0}, u8(0xff));
   TRY_EMIT(mov, BYTES(u8(0xc6), u8(0x02), u8(0x33)), mem8{rdx, 0}, u8(0x33));

   // r16 MOV imm16
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0xc7), u8(0x03), u16(0x0f)), mem16{rbx, 0}, u16(0x0f));
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0xc7), u8(0x00), u16(0xff)), mem16{rax, 0}, u16(0xff));
   TRY_EMIT(mov, BYTES(u8(0x66), u8(0xc7), u8(0x02), u16(0x33)), mem16{rdx, 0}, u16(0x33));

   // r32 MOV imm32
   TRY_EMIT(mov, BYTES(u8(0xc7), u8(0x03), u32(0x0f)), mem32{ebx, 0}, u32(0x0f));
   TRY_EMIT(mov, BYTES(u8(0xc7), u8(0x00), u32(0xff)), mem32{eax, 0}, u32(0xff));
   TRY_EMIT(mov, BYTES(u8(0xc7), u8(0x02), u32(0x33)), mem32{edx, 0}, u32(0x33));

   // r64 MOV imm64
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0xc7), u8(0x03), u64(0x0f)), mem64{rbx, 0}, u64(0x0f));
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0xc7), u8(0x00), u64(0xff)), mem64{rax, 0}, u64(0xff));
   TRY_EMIT(mov, BYTES(u8(0x48), u8(0xc7), u8(0x02), u64(0x33)), mem64{rdx, 0}, u64(0x33));
}

TEST_CASE("Testing x86_64 `call` instruction", "[x86_64 `call` instruction]") {
   // call indirect
   TRY_EMIT(call, BYTES(u8(0xFF), u8(0xd0)), rax);
}

TEST_CASE("Testing x86_64 `push` instruction", "[x86_64 `push` instruction]") {
   // push mem16
   TRY_EMIT(push, BYTES(u8(0x66), u8(0xFF), u8(0x30)), mem16{rax, 0});
   TRY_EMIT(push, BYTES(u8(0x66), u8(0xFF), u8(0x32)), mem16{rdx, 0});
   TRY_EMIT(push, BYTES(u8(0x66), u8(0xFF), u8(0x37)), mem16{rdi, 0});

   // push mem32
   TRY_EMIT(push, BYTES(u8(0xFF), u8(0x30)), mem32{rax, 0});
   TRY_EMIT(push, BYTES(u8(0xFF), u8(0x32)), mem32{rdx, 0});
   TRY_EMIT(push, BYTES(u8(0xFF), u8(0x37)), mem32{rdi, 0});

   // push mem64
   TRY_EMIT(push, BYTES(u8(0x48), u8(0xFF), u8(0x30)), mem64{rax, 0});
   TRY_EMIT(push, BYTES(u8(0x48), u8(0xFF), u8(0x32)), mem64{rdx, 0});
   TRY_EMIT(push, BYTES(u8(0x48), u8(0xFF), u8(0x37)), mem64{rdi, 0});

   // push reg16
   TRY_EMIT(push, BYTES(u8(0x66), u8(0x50)), ax);
   TRY_EMIT(push, BYTES(u8(0x66), u8(0x52)), dx);
   TRY_EMIT(push, BYTES(u8(0x66), u8(0x57)), di);

   // push reg32
   TRY_EMIT(push, BYTES(u8(0x50)), eax);
   TRY_EMIT(push, BYTES(u8(0x52)), edx);
   TRY_EMIT(push, BYTES(u8(0x57)), edi);

   // push reg64
   TRY_EMIT(push, BYTES(u8(0x50)), rax);
   TRY_EMIT(push, BYTES(u8(0x52)), rdx);
   TRY_EMIT(push, BYTES(u8(0x57)), rdi);
   TRY_EMIT(push, BYTES(u8(0x41), u8(0x50)), r8);

}

TEST_CASE("Testing x86_64 `ret` instruction", "[x86_64 `ret` instruction]") {
   // ret
   TRY_EMIT(ret, BYTES(u8(0xc3)));
   TRY_EMIT(ret, BYTES(u8(0xC2), u16(200)), u16(200));
}
