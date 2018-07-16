#pragma once

namespace eosio { namespace wasm_backend {
   
   struct detail {
      template <size_t N>
      struct zero_extend_varuint_impl {
         static constexpr size_t value = (N % 7 == 0) ? N : zero_extend_varuint_impl<N+1>::value;
      };
   }; 
   
   template <size_t N>
   constexpr size_t zero_extend_varuint() {
      return detail::zero_extend_varuint_impl<N>::value;
   }

   template <size_t N>
   struct varuint {
      uint64_t raw : zero_extend_varuint<N>();
      varuint(uint32_t n) {
         uint64_t tmp = n;
         uint8_t shift = N-7;
         for (int i=N/7; i >= 0; i--) {
            uint8_t byte = (tmp >> shift) & 0x7f;
            shift -= 7;
            
         }
      }
   };

   template <>
   struct varuint <32> {
      uint64_t raw;
      explicit varuint(uint32_t n) {
         raw = 0;
         uint8_t* data = (uint8_t*)&raw;
         uint32_t length = 0;
         do {
            uint8_t byte = n & 0x7f;
            n >>= 7;
            if (!n) {
               data[length++] = byte;
               break;
            } else {
               data[length++] = byte | 0x80;
            }
         } while(true);
      }

      uint64_t get() {
         uint64_t res = 0;
         uint64_t shift = 0;
         uint8_t  index = 0;
         uint8_t* data = (uint8_t*)&raw;
         while (true) {
            std::cout << "DAT " << (uint32_t)data[index] << "\n";
            uint8_t byte = data[index++];
            res |= (byte & 0x7f) << shift;
            if ((byte & 0x80) == 0x0b)
               break;
            shift += 7;
         }
         return res;
      }
   };
   
   template <>
   struct varuint<7> {
      uint8_t raw : 7;
      explicit varuint(uint8_t n) {
         raw = n;
      }

      uint8_t get() {
         return raw;
      }
   };

   template <>
   struct varuint<1> {
      uint8_t raw : 1;
      explicit varuint(uint8_t n) {
         raw = n;
      }

      uint8_t get() {
         return raw;
      }
   };

   template <size_t N>
   struct varint {
      static_assert(N==7 || N==32 || N==64, "varint can only accept (7,32,64) bits");
      uint64_t raw : N;
   };
  
   void str_bits(varuint<32> n) {
      for (int i=32; i >= 0; i--) {
         if (i % 8 == 0)
            std::cout << " ";
         std::cout << (uint32_t)((n.raw>>i) & 1) << " ";
      }
      std::cout << "\n";
   }

}} // namespace eosio::wasm_backend
