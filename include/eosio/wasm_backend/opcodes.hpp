#pragma once
#include <eosio/wasm_backend/opcodes_def.hpp>
#include <map>

namespace eosio { namespace wasm_backend {
   enum opcode {
      OPCODES(CREATE_ENUM)
   };
   
   struct opcode_utils {
      std::map<uint16_t, std::string> opcode_map{OPCODES(CREATE_MAP)};
   }; 

   enum imm_types {
      none,
      block_imm,
      varuint32_imm,
      br_table_imm,
   };
}} // namespace eosio::wasm_backend
