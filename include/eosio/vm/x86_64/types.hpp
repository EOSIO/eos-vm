#pragma once

#include <eosio/vm/exceptions.hpp>
#include <eosio/vm/utils.hpp>
#include <eosio/vm/variant.hpp>
#include <eosio/vm/x86_64/registers.hpp>

#include <limits>
#include <optional>

namespace eosio { namespace vm { namespace x86_64 {
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

   struct optional_ty {};

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
   struct mem {
      template <typename Reg1, typename Reg2>
      constexpr mem(Reg1 base, Reg2 index, uint8_t scale = 0, uint32_t disp = 0)
         : value(scale, index.reg, base.reg), displacement(disp),
           is_32_bit(base.size == 32) {
         EOS_VM_ASSERT(base.size == 32 || base.size == 64, x86_64_encoder_exception, "base is not 32 or 64 bits");
      }

      constexpr mem(Reg1 base, Reg2 index, uint32_t disp = 0)
         : value(0, index.reg, base.reg), displacement(disp),
           is_32_bit(base.size == 32) {
         EOS_VM_ASSERT(base.size == 32 || base.size == 64, x86_64_encoder_exception, "base is not 32 or 64 bits");
      }

      constexpr mem(Reg1 base, uint32_t disp = 0)
         : value(0, 0, base.reg), displacement(disp), skip_sib(true),
           is_32_bit(base.size == 32) {
         EOS_VM_ASSERT(base.size == 32 || base.size == 64, x86_64_encoder_exception, "base is not 32 or 64 bits");
         }

      constexpr mem() = default;

      constexpr bool is_displacement_8()const {
         return displacement <= std::numeric_limits<uint8_t>::max();
      }

      constexpr bool has_displacement()const { return displacement > 0; }

      sib      value        = sib{0};
      uint32_t displacement = 0;
      // used for memory operands with only a base and displacement
      bool     skip_sib     = false;
      bool     is_32_bit    = false;
   };

   using mem_reg8  = variant<reg8, mem>;
   using mem_reg16 = variant<reg8, mem>;
   using mem_reg32 = variant<reg8, mem>;
   using mem_reg64 = variant<reg8, mem>;

   enum mem_reg_index {
      reg = 0;
      mem = 1;
   };

   template <typename T>
   inline constexpr bool is_mem_reg(T&& op) { return
         (std::is_same_v<std::decay_t<T>, mem_reg8>  ||
          std::is_same_v<std::decay_t<T>, mem_reg16> ||
          std::is_same_v<std::decay_t<T>, mem_reg32> ||
          std::is_same_v<std::decay_t<T>, mem_reg64>); }

   template <typename T>
   inline constexpr bool is_mem(T&& op) { return
         (is_mem_reg(op) && op.is_a<mem>()) ||
         (std::is_same_v<std::decay_t<T>, mem); }

   template <typename T>
   inline constexpr bool is_reg(T&& op) { return
         (is_mem_reg(op) && (op.is_a<reg8>()  ||
                             op.is_a<reg16>() ||
                             op.is_a<reg32>() ||
                             op.is_a<reg64>())) ||
         (std::is_same_v<std::decay_t<T>, reg8> ||
         (std::is_same_v<std::decay_t<T>, reg16> ||
         (std::is_same_v<std::decay_t<T>, reg32> ||
         (std::is_same_v<std::decay_t<T>, reg64>); }

   template <typename T, typename U>
   inline constexpr auto get_reg(const variant<T, U>& op) {
      return op.get<mem_reg_index::reg>().reg;
   }

   template <typename T>
   inline constexpr auto get_reg(T&& op) {
      return op.reg;
   }

   template <typename T, typename U>
   inline constexpr auto get_mem(const variant<T, U>& op) {
      return op.get<mem_reg_index::mem>().value;
   }

   template <typename T>
   inline constexpr auto get_mem(T&& op) {
      return op.value;
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
            EOS_VM_ASSERT(Extension < 8, x86_64_encoder_exception, "ModRM extension out of range");
            reg = Extension;
            rm  = op2_is_reg ? get_reg(operand2) : 0;
         }

         if (op1_is_reg && op2_is_reg)
            mod = register_direct_mode;

         bool op1_is_mem = is_mem(operand1);
         bool op2_is_mem = is_mem(operand2);
         const auto& check_displacement = [&](const auto& m) {
            if (!m.has_displacement())
               mod = register_indirect_mode;;
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

}}} // namespace eosio::vm::x86_64
