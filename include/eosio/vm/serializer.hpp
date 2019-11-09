#pragma once

#include <eosio/vm/types.hpp>

namespace eosio { namespace vm {
   // class to serialize a WASM module
   class serializer {
      public:
         serializer(const module& mod) : _mod(mod) {}
         void write(std::vector<uint8_t>& vec) {
            write_header();
            vec = _data;
         }
      private:
         void write_header() {
            write_bytes("\0asm");
            write_bytes("\0\0\0\1");
         }
         void write_bytes(const char* bytes, std::size_t len) {
            char* _back = (char*)(&_data.back())+1;
            _data.resize(_data.size()+len);
            memcpy(_back, bytes, len);
         }
         template <std::size_t N>
         void write_bytes(const char (&bytes)[N]) {
            _data.reserve(_data.size()+(N-1));
            char* _back = (char*)(&_data.back())+1;
            _data.resize(_data.size()+(N-1));
            memcpy(_back, bytes, (N-1));
         }

         const module& _mod;
         std::vector<uint8_t> _data;
   };
}} // namespace eosio::vm
