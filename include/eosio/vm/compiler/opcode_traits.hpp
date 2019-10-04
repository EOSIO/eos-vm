#pragma once

namespace eosio { namespace vm {
   struct opcode_traits {
      static inline is_a_control(opcode* op) {
         return op->is_a<if_t>() || op->is_a<else_t>() || op->is_a<br_t>() || op->is_a<br_if_t>();
      }

      static inline is_a_leader(opcode* op) {
         return op->is_a<block_t>() || op->is_a<if_t>() || op->is_a<else_t>();
      }
   };
}} // namespace eosio::vm
