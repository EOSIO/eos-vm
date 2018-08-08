#pragma once

#include <iostream>
#include <cstdlib>
#include <eosio/wasm_backend/exceptions.hpp>
#include <eosio/wasm_backend/utils.hpp>

namespace eosio { namespace wasm_backend {
   
   template <size_t N>
   struct zero_extended_size {
      static constexpr int bits = -1;
      static constexpr int bytes = -1;
   };

   template <>
   struct zero_extended_size<1> {
      static constexpr size_t bits = 7;
      static constexpr size_t bytes = 1;
   };

   template <>
   struct zero_extended_size<7> {
      static constexpr size_t bits = 7;
      static constexpr size_t bytes = 1;
   };

   template <>
   struct zero_extended_size<32> {
      static constexpr size_t bits = 35;
      static constexpr size_t bytes = 5;
   };

   template <>
   struct zero_extended_size<64> {
      static constexpr size_t bits = 70;
      static constexpr size_t bytes = 9;
   };

   template <size_t N>
   struct varuint {
      static_assert(zero_extended_size<N>::bits >= 7, 
            "varuint bit width not defined, use 1,7,32, or 64");
      uint8_t raw[zero_extended_size<N>::bytes] = {0};

      uint8_t size = 0;
      varuint() = default;
      varuint(uint64_t n) {
         set(n);
      }
      varuint( const std::vector<uint8_t>& code, size_t index ) {
         set(code, index);
      }
      varuint( const varuint<N>& n ) {
         uint64_t* tmp = (uint64_t*)raw;
         *tmp = *(uint64_t*)n.raw;
         size = n.size;
      }
      varuint& operator=( const varuint& n ) {
         uint64_t* tmp = (uint64_t*)raw;
         *tmp = *(uint64_t*)n.raw;
         size = n.size;
         return *this;
      }

      inline void set( const std::vector<uint8_t>& code, size_t index ) {
         uint8_t cnt = 0;
         for (; cnt < zero_extended_size<N>::bytes; cnt++ ) {
            EOS_WB_ASSERT( index+cnt < code.size(), wasm_interpreter_exception, "varuint not terminated before end of code" );
            raw[cnt] = code[index+cnt];
            if ((raw[cnt] & 0x80) == 0)
               break;
         }
         size = cnt+1;
      }

      inline void set( guarded_ptr<uint8_t>& code ) {
         for (uint8_t cnt=0; cnt < zero_extended_size<N>::bytes; cnt++ ) {
            EOS_WB_ASSERT( code.offset()+cnt < code.bounds(), wasm_interpreter_exception, "pointer out of bounds" );
            raw[cnt] = code[cnt];
            if ((raw[cnt] & 0x80) == 0) {
               size = cnt+1;
               code += size;
               break;
            }
         }
      }

      inline void set(uint64_t n) {
         uint8_t cnt = 1;
         guarded_ptr<uint8_t> data( raw, zero_extended_size<N>::bytes );
         EOS_WB_ASSERT( n < ((unsigned __int128)1 << N), wasm_interpreter_exception, 
               "value too large for bit width specified" );
         for (; cnt < sizeof(n); cnt++) {
            uint8_t byte = n & 0x7F;
            n >>= 7;
            if (n != 0)
               byte |= 0x80;
            *data++ = byte;
            if (n == 0)
               break;
         }
         size = cnt; 
      }

      inline uint64_t get() {
         uint64_t n = 0;
         uint8_t shift = 0;
         const uint8_t* end = raw + size;
         uint8_t* data = raw;
         bool quit = false;
         for (int i=0; i < size; i++) {
            EOS_WB_ASSERT( end && data != end, wasm_interpreter_exception, "malformed varuint");
            uint64_t byte = *data & 0x7F;
            EOS_WB_ASSERT( !(shift >= 64 || byte << shift >> shift != byte), 
                  wasm_interpreter_exception, "varuint too big for uint64");
            n += byte << shift;
            shift += 7;
            if ( quit )
               break;
            if ( *data++ <= 0x80 )
               quit = true;

         };
         return n;
      }
   };

   template <size_t N>
   struct varint {
      static_assert(zero_extended_size<N>::bits >= 7, 
            "varint bit width not defined, use 1,7,32, or 64");
      uint8_t raw[zero_extended_size<N>::bytes];
      uint8_t size;
      varint() = default;
      varint(int64_t n) {
         set(n);
      }
      varint( const std::vector<uint8_t>& code, size_t index ) {
         set(code, index);
      }
      varint( const varint<N>& n ) {
         //TODO fix this
         memcpy(raw, n.raw, n.size);
         size = n.size;
      }
      varint& operator=( const varint<N>& n ) {
         //TODO fix this
         memcpy(raw, n.raw, n.size);
         size = n.size;
         return *this;
      }

      inline void set( const std::vector<uint8_t>& code, size_t index ) {
         uint8_t cnt = 0;
         for (; cnt < zero_extended_size<N>::bytes; cnt++ ) {
            EOS_WB_ASSERT( index+cnt < code.size(), wasm_interpreter_exception, "varuint not terminated before end of code" );
            raw[cnt] = code[index+cnt];
            if ((raw[cnt] & 0x80) == 0)
               break;
         }
         size = cnt+1;
      }

      inline void set(int64_t n) {
         uint8_t* data = raw;
         uint8_t cnt = 1;
         bool more = true;
         if ( n >= 0 )
            EOS_WB_ASSERT( n < ((uint64_t)1 << (N-1)), 
               wasm_interpreter_exception,
               "value too large for bit width specified" );
         else
            EOS_WB_ASSERT( n >= (int64_t)(((uint64_t)-1 << (N-1))), 
               wasm_interpreter_exception,
               "value too small for bit width specified" );

         for (; more && cnt < sizeof(n); cnt++) {
            uint8_t byte = n & 0x7F;
            n >>= 7;
            more = !((((n == 0) && ((byte & 0x40) == 0)) ||
                     ((n == -1) && ((byte & 0x40) != 0))));

            if (more)
               byte |= 0x80;
            *data++ = byte;
            if (!more)
               break;
         }
         size = cnt;
      }

      inline int64_t get() const {
         int64_t n = 0;
         uint8_t shift = 0;
         uint8_t byte;
         const uint8_t* end = raw + size;
         const uint8_t* data = raw;
         
         for (int i=0; i < size; i++) {
            EOS_WB_ASSERT( end && data != end, wasm_interpreter_exception, "malformed varint");
            byte = *data++;
            n |= (int64_t)(byte & 0x7F) << shift;
            shift += 7;
            if ( byte < 0x80 )
               break;
         };

         if (byte & 0x40)
            n |= (-1ull) << shift;
         return n;
      }
   };

   /*
   void str_bits(varuint<32> n) {
      for (int i=32; i >= 0; i--) {
         if (i % 8 == 0)
            std::cout << " ";
         std::cout << (uint32_t)((n.raw[i/8] >> i%8) & 1) << " ";
      }
      std::cout << "\n";
   }
   */

}} // namespace eosio::wasm_backend
