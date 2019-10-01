#pragma once

#include <eosio/vm/base_visitor.hpp>
#include <eosio/vm/types.hpp>

namespace eosio { namespace vm {
class wasm_serializer : public base_visitor {
   public:
      wasm_serializer(const module& mod) : _module(mod) {}
      template <typename Stream>
      void write(Stream&& stream) {
         write_magic(std::forward<Stream>(stream))
            .write_version(std::forward<Stream>(stream))
            .write_section<section_id::type_section>(std::forward<Stream>(stream));
      }
      template <typename Stream, typename T>
      inline auto& write_value(Stream&& stream, T&& val) {
         stream.write((char*)&val, sizeof(val));
         return *this;
      }
      template <typename Stream, size_t N>
      inline auto& write_varuint(Stream&& stream, const varuint<N>& val) {
         const auto& bytes = val.bytes();
         stream.write((char*)bytes.data(), bytes.size());
         return *this;
      }
      template <typename Stream, size_t N>
      inline auto& write_varuint(Stream&& stream, const varint<N>& val) {
         const auto& bytes = val.bytes();
         stream.write((char*)&bytes.data(), bytes.size());
         return *this;
      }

      template <typename Stream>
      inline auto& write_magic(Stream&& stream) { write_value(std::forward<Stream>(stream), static_cast<uint32_t>(constants::magic)); return *this; }
      template <typename Stream>
      inline auto& write_version(Stream&& stream) { write_value(std::forward<Stream>(stream), static_cast<uint32_t>(constants::version)); return *this; }
      inline auto& write_version(Stream&& stream) { 
      template <typename Stream, section_id SectionID>
      inline auto& write_section(Stream&& stream) {
         write_value(std::forward<Stream>(stream), static_cast<uint8_t>(SectionID));
         if constexpr (SectionID == section_id::type_section)
            write_value(std::forward<Stream>(stream), write_type_section(std::forward<Stream>(stream));
         //if constexpr (SectionID == section_id::type_section)
         //   write_type_section(std::forward<Stream>(stream));
         return *this;
      }
      template <typename Stream>
      inline std::size_t write_type_section(Stream&& stream) { 
         write_varuint(std::forward<Stream>(stream), varuint<32>(_module.types.size()));
         for (uint32_t i; i < _module.types.size(); i++) {
            write_value(std::forward<Stream>(stream), static_cast<uint8_t>(_module.types[i].form));
            write_varuint(std::forward<Stream>(stream)
         }
      }
   private:
      const module& _module;
};

}} // namespace eosio::vm
