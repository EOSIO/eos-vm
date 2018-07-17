#pragma once

#include <eosio/wasm_backend/exceptions.hpp>

namespace eosio { namespace wasm_backend {
   
   template <size_t N>
   struct zero_extended_size {
      static constexpr int value = -1;
   };

   template <>
   struct zero_extended_size<1> {
      static constexpr size_t value = 7;
   };

   template <>
   struct zero_extended_size<7> {
      static constexpr size_t value = 7;
   };

   template <>
   struct zero_extended_size<32> {
      static constexpr size_t value = 35;
   };

   template <>
   struct zero_extended_size<64> {
      static constexpr size_t value = 70;
   };

   template <size_t N>
   struct varuint {
      static_assert(zero_extended_size<N>::value >= 7, 
            "varuint bit width not defined, use 1,7,32, or 64");
      uint8_t raw[zero_extended_size<N>::value];
      uint8_t size;
      varuint(uint64_t n, uint8_t pad=0) {
         set(n, pad);
      }

      inline void set(uint64_t n, uint8_t pad=0) {
         uint8_t cnt = 0;
         uint8_t* data = raw;
         EOS_WB_ASSERT( n < ((uint64_t)1 << N), wasm_interpreter_exception, 
               "value too large for bit width specified" );
         do {
            uint8_t byte = n & 0x7F;
            n >>= 7;
            cnt++;
            if (n != 0 || cnt < pad)
               byte |= 0x80;
            *data++ = byte;
         } while (n != 0);
         size = cnt; 
         // pad if needed
         if (cnt < pad) {
            for (; cnt < pad - 1; ++cnt)
               *data++ = 0x80;
         }
      }

      inline uint64_t get() {
         uint64_t n = 0;
         uint8_t shift = 0;
         const uint8_t* end = raw + size;
         uint8_t* data = raw;
         do {
            EOS_WB_ASSERT( end && data != end, wasm_interpreter_exception, "malformed varuint");
            uint64_t byte = *data & 0x7F;
            EOS_WB_ASSERT( !(shift >= 64 || byte << shift >> shift != byte), 
                  wasm_interpreter_exception, "varuint too big for uint64");
            
            n += uint64_t(*data & 0x7F) << shift;
            shift += 7;
         } while (*data++ >= 128);
         return n;
      }
   };
   
   template <size_t N>
   struct varint {
      static_assert(zero_extended_size<N>::value >= 7, 
            "varint bit width not defined, use 1,7,32, or 64");
      uint8_t raw[zero_extended_size<N>::value];
      uint8_t size;
      varint(int64_t n, uint8_t pad=0) {
         set(n, pad);
      }

      inline void set(int64_t n, uint8_t pad=0) {
         uint8_t* data = raw;
         uint8_t cnt = 0;
         bool more;
         if ( n >= 0 )
            EOS_WB_ASSERT( n < ((uint64_t)1 << (N-1)), 
               wasm_interpreter_exception,
               "value too large for bit width specified" );
         else
            EOS_WB_ASSERT( n >= (int64_t)(((uint64_t)-1 << (N-1))), 
               wasm_interpreter_exception,
               "value too small for bit width specified" );

         do {
            uint8_t byte = n & 0x7F;
            n >>= 7;
            more = !((((n == 0) && ((byte & 0x40) == 0)) ||
                     ((n == -1) && ((byte & 0x40) != 0))));
            cnt++;

            if (more || cnt < pad)
               byte |= 0x80;
            *data++ = byte;
         } while (more);
         size = cnt;
         if (cnt < pad) {
            uint8_t padded = n < 0 ? 0x7F : 0x00;
            for (; cnt < pad - 1; ++cnt)
               *data++ = (padded | 0x80);
            *data++ = padded;
         }
      }

      inline int64_t get() {
         int64_t n = 0;
         uint8_t shift = 0;
         uint8_t byte;
         const uint8_t* end = raw + size;
         uint8_t* data = raw;
         
         do {
            EOS_WB_ASSERT( end && data != end, wasm_interpreter_exception, "malformed varint");
            byte = *data++;
            n |= (int64_t)(byte & 0x7F) << shift;
            shift += 7;
         } while ( byte >= 128);

         if (byte & 0x40)
            n |= (-1ull) << shift;
         return n;
      }
   };

   void str_bits(varuint<32> n) {
      for (int i=32; i >= 0; i--) {
         if (i % 8 == 0)
            std::cout << " ";
         std::cout << (uint32_t)((n.raw[i/8] >> i%8) & 1) << " ";
      }
      std::cout << "\n";
   }

}} // namespace eosio::wasm_backend
