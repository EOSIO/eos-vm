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
         encoder(uint8_t* start, uint64_t offset) : start(start), block(start), end(start+offset) {}

         inline void reset(uint8_t* start, uint64_t offset) {
            start = start;
            set(start, offset);
         }
         inline void set(uint8_t* start, uint64_t offset) {
            block = start;
            end   = start + offset;
         }
         inline void set_end_relative(uint64_t e) { end = start+e; }
         inline void add_to_end(uint64_t e) { end += e; }
         inline void* current_loc() const { return static_cast<void*>(block); }
         inline void assert_block() const { assert(block == end); }

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
         inline void emit_modrm(T&& t, U&& u) {
            emit(modrm{is_mem_value(t) || is_mem_value(u) ? (uint8_t)0b00 : (uint8_t)0b11, get_value(u), get_value(t)}.value);
         }

         template <typename T>
         inline void emit_rex(T&& op) {
            if constexpr (is_mem_reg_type<T>() || is_mem_type<T>() || is_reg_type<T>())
               emit(prefix::rex{ 1, 0, 0, get_value(op) > 7}.value);
         }

         template <typename T, typename U>
         inline void emit_rex(T&& op1, U&& op2) {
            uint8_t reg = 0, rm = 0;
            if constexpr (is_mem_reg_type<T>() || is_mem_type<T>() || is_reg_type<T>())
               rm = get_value(op1) > 7;
            if constexpr (is_mem_reg_type<U>() || is_mem_type<U>() || is_reg_type<U>())
               reg = get_value(op2) > 7;
            emit(prefix::rex{1, reg, 0, rm}.value);
         }

         inline void emit_opcode(uint8_t op) { emit(op); }

         template <typename T>
         inline void maybe_emit_rex(T&& op) {
            if constexpr (bitwidth_v<T> == 64) {
               emit_rex(std::move(op));
            }
         }

         template <typename T, typename U>
         inline void maybe_emit_rex(T&& op1, U&& op2) {
            if constexpr (bitwidth_v<T> == 64) {
               if constexpr (bitwidth_v<U> == 64) {
                  emit_rex(std::move(op1), std::move(op2));
               } else {
                  emit_rex(std::move(op1));
               }
            } else if constexpr (bitwidth_v<U> == 64) {
               emit_rex(uint8_t(0), std::move(op2));
            }
         }

         template <typename T>
         inline void maybe_emit_prefix(T&& op) {
            if constexpr (bitwidth_v<T> == 16)
               emit(prefix::operand_size_override);
         }

         template <uint8_t Opcode, typename T, typename U>
         inline void emit_and_impl(T&& op1, U&& op2) {
            maybe_emit_prefix(op1);
            maybe_emit_rex(op1, op2);
            emit_opcode(Opcode);
            emit_modrm(std::move(op1), 4);
            emit(std::move(op2));
         }

         template <uint8_t Opcode, typename T, typename U>
         inline void emit_cmp_impl(T&& op1, U&& op2) {
            maybe_emit_prefix(op1);
            maybe_emit_rex(op1, op2);
            emit_opcode(Opcode);
            emit_modrm(std::move(op1), 7);
            emit(std::move(op2));
         }

         template <uint8_t Opcode, typename T, typename U>
         inline void emit_mov1_impl(T&& op1, U&& op2) {
            maybe_emit_prefix(op1);
            maybe_emit_rex(op1, op2);
            emit_opcode(Opcode);
            if constexpr (std::is_integral_v<std::decay_t<U>>) {
               emit_modrm(std::move(op1), 0);
               emit(std::move(op2));
            }
            else
               emit_modrm(std::move(op1), std::move(op2));
         }

         template <uint8_t Opcode, typename T, typename U>
         inline void emit_mov2_impl(T&& op1, U&& op2) {
            maybe_emit_prefix(op1);
            maybe_emit_rex(op1, op2);
            emit_opcode(Opcode + get_value(std::move(op1)));
            emit(std::move(op2));
         }

         template <typename T, typename O>
         inline void emit_jcc_impl(T&& op, O&& opcode) {
            emit(opcode);
            emit(std::move(op));
         }

         template <uint8_t Opcode, typename T>
         inline void emit_push1_impl(T&& op) {
            maybe_emit_prefix(op);
            if constexpr (Opcode == 0xFF) {
               maybe_emit_rex(op);
               emit_opcode(Opcode);
               emit_modrm(std::move(op), 6);
            } else {
               uint8_t v = get_value(op) > 7;
               if (v)
                  emit(prefix::rex{0, 0, 0, v}.value);
               emit_opcode(Opcode + (get_value(op) & 0x7));
            }
         }

         INSTRUCTION(and,
               H2(mem_reg8,  uint8_t)  { emit_and_impl<0x80>(std::move(operand1), std::move(operand2)); }, // r/m8, imm8
               H2(mem_reg16, uint16_t) { emit_and_impl<0x81>(std::move(operand1), std::move(operand2)); }, // r/m16, imm16
               H2(mem_reg32, uint32_t) { emit_and_impl<0x81>(std::move(operand1), std::move(operand2)); }, // r/m32, imm32
               H2(mem_reg64, uint32_t) { emit_and_impl<0x81>(std::move(operand1), std::move(operand2)); }, // r/m64, imm32
               H2(mem_reg16, uint8_t)  { emit_and_impl<0x83>(std::move(operand1), std::move(operand2)); }, // r/m16, imm8
               H2(mem_reg32, uint8_t)  { emit_and_impl<0x83>(std::move(operand1), std::move(operand2)); }, // r/m32, imm8
               H2(mem_reg64, uint8_t)  { emit_and_impl<0x83>(std::move(operand1), std::move(operand2)); }  // r/m64, imm8
         );

         INSTRUCTION(cmp,
               H2(mem_reg8,  uint8_t)  { emit_cmp_impl<0x80>(std::move(operand1), std::move(operand2)); }, // r/m8, imm8
               H2(mem_reg16, uint16_t) { emit_cmp_impl<0x81>(std::move(operand1), std::move(operand2)); }, // r/m16, imm16
               H2(mem_reg32, uint32_t) { emit_cmp_impl<0x81>(std::move(operand1), std::move(operand2)); }, // r/m32, imm32
               H2(mem_reg64, uint32_t) { emit_cmp_impl<0x81>(std::move(operand1), std::move(operand2)); }, // r/m64, imm32
               H2(mem_reg16, uint8_t)  { emit_cmp_impl<0x83>(std::move(operand1), std::move(operand2)); }, // r/m16, imm8
               H2(mem_reg32, uint8_t)  { emit_cmp_impl<0x83>(std::move(operand1), std::move(operand2)); }, // r/m32, imm8
               H2(mem_reg64, uint8_t)  { emit_cmp_impl<0x83>(std::move(operand1), std::move(operand2)); }, // r/m64, imm8
         );

         INSTRUCTION(mov,
               H2(mem_reg8,  reg8)  { emit_mov1_impl<0x88>(std::move(operand1), std::move(operand2)); }, // r/m8, r8
               H2(mem_reg16, reg16) { emit_mov1_impl<0x89>(std::move(operand1), std::move(operand2)); }, // r/m16, r16
               H2(mem_reg32, reg32) { emit_mov1_impl<0x89>(std::move(operand1), std::move(operand2)); }, // r/m32, r32
               H2(mem_reg64, reg64) { emit_mov1_impl<0x89>(std::move(operand1), std::move(operand2)); }, // r/m64, r64
               H2(reg8,  mem8)      { emit_mov1_impl<0x8A>(std::move(operand2), std::move(operand1)); }, // r8, r/m8
               H2(reg16, mem16)     { emit_mov1_impl<0x8B>(std::move(operand2), std::move(operand1)); }, // r16, r/m16
               H2(reg32, mem32)     { emit_mov1_impl<0x8B>(std::move(operand2), std::move(operand1)); }, // r32, r/m32
               H2(reg64, mem64)     { emit_mov1_impl<0x8B>(std::move(operand2), std::move(operand1)); }, // r64, r/m64
               H2(reg8,  uint8_t)   { emit_mov2_impl<0xB0>(std::move(operand1), std::move(operand2)); }, // r8, imm8
               H2(reg16, uint16_t)  { emit_mov2_impl<0xB8>(std::move(operand1), std::move(operand2)); }, // r16, imm16
               H2(reg32, uint32_t)  { emit_mov2_impl<0xB8>(std::move(operand1), std::move(operand2)); }, // r32, imm32
               H2(reg64, uint64_t)  { emit_mov2_impl<0xB8>(std::move(operand1), std::move(operand2)); }, // r64, imm64
               H2(mem8,  uint8_t)   { emit_mov1_impl<0xC6>(std::move(operand1), std::move(operand2)); }, // m8, imm8
               H2(mem16, uint16_t)  { emit_mov1_impl<0xC7>(std::move(operand1), std::move(operand2)); }, // m16, imm16
               H2(mem32, uint32_t)  { emit_mov1_impl<0xC7>(std::move(operand1), std::move(operand2)); }, // m32, imm32
               H2(mem64, uint64_t)  { emit_mov1_impl<0xC7>(std::move(operand1), std::move(operand2)); }  // m64, imm64

         );

         INSTRUCTION(je,
               H1(uint8_t)  { emit_jcc_impl(std::move(operand), uint8_t{0x74}); },  // je rel8
               H1(uint32_t) { emit_jcc_impl(std::move(operand), uint32_t{0xF84}); }, // je rel16
         );

         INSTRUCTION(call,
               H1(mem_reg64) { // call indirect
                  emit_opcode(0xFF);
                  emit_modrm(operand, 2);
               }
         );

         INSTRUCTION(push,
               H1(mem16) { emit_push1_impl<0xFF>(std::move(operand)); }, // push m16
               H1(mem32) { emit_push1_impl<0xFF>(std::move(operand)); }, // push m32
               H1(mem64) { emit_push1_impl<0xFF>(std::move(operand)); }, // push m64
               H1(reg16) { emit_push1_impl<0x50>(std::move(operand)); }, // push r16
               H1(reg32) { emit_push1_impl<0x50>(std::move(operand)); }, // push r32
               H1(reg64) { emit_push1_impl<0x50>(std::move(operand)); }, // push r64

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

      private:
         uint8_t* start;
         uint8_t* block;
         uint8_t* end;
         bool     default_32 = true;
         bool     mode_64 = false;
   };

#undef INSTRUCTION
#undef H0
#undef H1
#undef H2

}}} // namespace eosio::vm::x86_64
