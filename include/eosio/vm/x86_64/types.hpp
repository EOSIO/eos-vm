#pragma once

#include <eosio/vm/exceptions.hpp>
#include <eosio/vm/utils.hpp>
#include <eosio/vm/variant.hpp>
#include <eosio/vm/x86_64/registers.hpp>

#include <limits>
#include <optional>

namespace eosio { namespace vm { namespace x86_64 {
   namespace prefix {
      inline constexpr uint8_t lock  = 0xf0;
      inline constexpr uint8_t repne = 0xf2;
      inline constexpr uint8_t repnz = 0xf2;
      inline constexpr uint8_t rep   = 0xf3;
      inline constexpr uint8_t repe  = 0xf3;
      inline constexpr uint8_t repz  = 0xf3;
      inline constexpr uint8_t cs_override = 0x2e; // 64-bit these are ignored
      inline constexpr uint8_t ss_override = 0x36; // 64-bit these are ignore;
      inline constexpr uint8_t ds_override = 0x3e; // 64-bit these are ignore;
      inline constexpr uint8_t es_override = 0x26; // 64-bit these are ignore;
      inline constexpr uint8_t fs_override = 0x64; // 64-bit these are ignore;
      inline constexpr uint8_t gs_override = 0x65; // 64-bit these are ignore;
      inline constexpr uint8_t branch_not_taken = 0x2e;
      inline constexpr uint8_t branch_taken     = 0x3e;
      inline constexpr uint8_t operand_size_override = 0x66;
      inline constexpr uint8_t address_size_override = 0x67;

      struct rex {
         constexpr rex(uint8_t v) : value(v) {}
         constexpr rex(uint8_t W, uint8_t R, uint8_t X, uint8_t B)
            : b0100(4), W(W), R(R), X(X), B(B) {}

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

   struct sib {
      constexpr sib(uint8_t s, uint8_t i, uint8_t b) : scale(s), index(i), base(b) {}
      constexpr sib(uint8_t val) : value(val) {}

      union {
         struct {
            uint8_t base  : 3;
            uint8_t index : 3;
            uint8_t scale : 2;
         };
         uint8_t value = 0;
      };
   };

   // type used to represent a memory operand
   template <std::size_t Size>
   struct mem {
      template <typename Reg1, typename Reg2>
      constexpr mem(Reg1 base, Reg2 index, uint8_t scale, uint32_t disp)
         : value(scale, index.value, base.value), displacement(disp),
           is_32_bit(base.size == 32) {
         //EOS_VM_ASSERT(base.size == 32 || base.size == 64, x86_64_encoder_exception, "base is not 32 or 64 bits");
      }

      template <typename Reg1, typename Reg2>
      constexpr mem(Reg1 base, Reg2 index, uint32_t disp)
         : value(0, index.value, base.value), displacement(disp),
           is_32_bit(base.size == 32) {
         //EOS_VM_ASSERT(base.size == 32 || base.size == 64, x86_64_encoder_exception, "base is not 32 or 64 bits");
      }

      template <typename Reg1>
      constexpr mem(Reg1 base, uint32_t disp)
         : value(0, 0, base.value), displacement(disp), skip_sib(true),
           is_32_bit(base.size == 32) {
         //EOS_VM_ASSERT(base.size == 32 || base.size == 64, x86_64_encoder_exception, "base is not 32 or 64 bits");
      }

      constexpr mem() = default;

      constexpr explicit operator uint8_t()const { return value.value; }

      constexpr bool is_displacement_8()const {
         return displacement <= std::numeric_limits<uint8_t>::max();
      }

      constexpr bool has_displacement()const { return displacement > 0; }

      sib      value        = sib{0};
      uint32_t displacement = 0;
      // used for memory operands with only a base and displacement
      bool     skip_sib     = false;
      bool     is_32_bit    = false;
      static constexpr uint8_t size = Size;
   };

   using mem8  = mem<8>;
   using mem16 = mem<16>;
   using mem32 = mem<32>;
   using mem64 = mem<64>;

   using mem_reg8  = variant<reg8,  mem8>;
   using mem_reg16 = variant<reg16, mem16>;
   using mem_reg32 = variant<reg32, mem32>;
   using mem_reg64 = variant<reg64, mem64>;

   enum mem_reg_index {
      reg_index = 0,
      mem_index = 1
   };

   template <typename T>
   inline constexpr bool is_mem_reg_type() {
      return std::is_same_v<std::decay_t<T>, mem_reg8>  ||
             std::is_same_v<std::decay_t<T>, mem_reg16> ||
             std::is_same_v<std::decay_t<T>, mem_reg32> ||
             std::is_same_v<std::decay_t<T>, mem_reg64>;
   }

   template <typename T>
   inline constexpr bool is_mem_type() {
      return std::is_same_v<std::decay_t<T>, mem8> ||
             std::is_same_v<std::decay_t<T>, mem16> ||
             std::is_same_v<std::decay_t<T>, mem32> ||
             std::is_same_v<std::decay_t<T>, mem64>;
   }

   template <typename T>
   inline constexpr bool is_reg_type() {
      return std::is_same_v<std::decay_t<T>, reg8>  ||
             std::is_same_v<std::decay_t<T>, reg16> ||
             std::is_same_v<std::decay_t<T>, reg32> ||
             std::is_same_v<std::decay_t<T>, reg64>;
   }

   template <typename T>
   inline constexpr bool is_mem_value(T&& op) {
      if constexpr (!is_mem_reg_type<T>()) {
         return is_mem_type<T>();
      } else {
         return op.template is_a<mem8>()  ||
                op.template is_a<mem16>() ||
                op.template is_a<mem32>() ||
                op.template is_a<mem64>() ;
      }
   }

   template <typename T>
   inline constexpr bool is_reg_value(T&& op) {
      if constexpr (!is_mem_reg_type<T>()) {
         return is_reg_type<T>();
      } else {
         return op.template is_a<reg8>()  ||
                op.template is_a<reg16>() ||
                op.template is_a<reg32>() ||
                op.template is_a<reg64>();
      }
   }

   template <typename T>
   inline constexpr auto get_reg(T&& op) {
      if constexpr (is_mem_reg_type<T>())
         return op.template get<mem_reg_index::reg_index>().value;
      else
         return op.value;
   }

   template <typename T>
   inline constexpr auto get_mem(T&& op) {
      if constexpr (is_mem_reg_type<T>())
         return op.template get<mem_reg_index::mem_index>().value;
      else
         return op.value;
   }

   template <typename T>
   inline constexpr uint8_t get_value(T&& op) {
      if constexpr(is_mem_reg_type<T>()) {
         if (is_mem_value(op))
            return (uint8_t)op.template get<mem_reg_index::mem_index>();
         else
            return (uint8_t)op.template get<mem_reg_index::reg_index>();
      } else {
         return (uint8_t)op;
      }
   }

   struct modrm {
      static constexpr uint8_t register_indirect_mode = 0b00;
      static constexpr uint8_t register_indirect_disp8_mode = 0b01;
      static constexpr uint8_t register_indirect_disp32_mode = 0b10;
      static constexpr uint8_t register_direct_mode = 0b11;

      template <typename T, typename U>
      modrm(T operand1, U operand2) {
         mod = register_indirect_mode; // default
         bool op1_is_reg = is_reg(operand1);
         bool op2_is_reg = is_reg(operand2);
         reg = op1_is_reg ? get_reg(operand1) : 0;
         rm  = op2_is_reg ? get_reg(operand2) : 0;

         if (op1_is_reg && op2_is_reg)
            mod = register_direct_mode;

         bool op1_is_mem = is_mem(operand1);
         bool op2_is_mem = is_mem(operand2);
         const auto& check_displacement = [&](const auto& m) {
            if (!m.has_displacement())
               mod = register_indirect_mode;
            else {
               if (m.displacement > 0xff)
                  mod = register_indirect_disp8_mode;
               else
                  mod = register_indirect_disp32_mode;
            }
         };
         if (op1_is_mem) {
            check_displacement(get_mem(operand1));
         } else if (op2_is_mem) {
            check_displacement(get_mem(operand2));
         }
      }

      template <typename T, uint8_t Extension=0xFF>
      modrm(T operand1) {
         mod = register_indirect_mode; // default
         bool op1_is_reg = is_reg(operand1);
         if constexpr (Extension == 0xFF) {
            reg = op1_is_reg ? get_reg(operand1) : 0;
         } else {
            //EOS_VM_ASSERT(Extension < 8, x86_64_encoder_exception, "ModRM extension out of range");
            reg = Extension;
         }

         if (op1_is_reg)
            mod = register_direct_mode;

         bool op1_is_mem = is_mem(operand1);
         const auto& check_displacement = [&](const auto& m) {
            if (!m.has_displacement()) {
               mod = register_indirect_mode;;
            } else {
               if (m.displacement > 0xff)
                  mod = register_indirect_disp8_mode;
               else
                  mod = register_indirect_disp32_mode;
            }
         };
         if (op1_is_mem) {
            check_displacement(get_mem(operand1));
         }
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


   template <typename T>
   constexpr uint8_t get_type_bitwidth() {
      if constexpr (is_mem_reg_type<T>()) {
         using ret_type = std::decay_t<decltype(std::declval<T>().template get<mem_reg_index::mem_index>())>;
         return ret_type::size;
      } else if constexpr (is_reg_type<T>() || is_mem_type<T>()) {
         return std::decay_t<T>::size;
      } else {
         return 0;
      }
   }

   template <typename T>
   static constexpr uint8_t bitwidth_v = get_type_bitwidth<T>();
}}} // namespace eosio::vm::x86_64
