#pragma once

#include <eosio/vm/types.hpp>

#include <string_view>

namespace eosio { namespace vm {

#define MEMORY_DUMP_OP_VISIT(name, code) \
   void operator()(const name##_t& op) { \
      stream << #name << "\n"; \
   }

#define MEMORY_DUMP_CONTROL_FLOW_VISIT(name, code) \
   void operator()(const name##_t& op) { \
      stream << #name << " : { " << op.data << ", " << op.pc << ", " << op.index << ", " << op.op_index << " }\n"; \
   }

#define MEMORY_DUMP_BR_TABLE_VISIT(name, code) \
   void operator()(const name##_t& op) { \
      stream << #name << " : { [ "; \
      for (uint32_t i=0; i < op.size; i++) { \
         stream << op.table[i]; \
         if (i < op.size-1) { \
            stream << ", "; \
         } \
      } \
      stream << " ] }\n"; \
   }

#define MEMORY_DUMP_CALL_VISIT(name, code) \
   void operator()(const name##_t& op) { \
      stream << #name << " : { " << op.index << " }\n"; \
   }

#define MEMORY_DUMP_VARIABLE_ACCESS_VISIT(name, code) \
   void operator()(const name##_t& op) { \
      stream << #name << " : { " << op.index << " }\n"; \
   }

#define MEMORY_DUMP_MEMORY_VISIT(name, code) \
   void operator()(const name##_t& op) { \
      stream << #name << " : { " << op.flags_align << ", " << op.offset << " }\n"; \
   }

#define MEMORY_DUMP_CONST_VISIT(name, code) \
   void operator()(const name##_t& op) { \
      stream << #name << " : { " << op.data.ui << " }\n"; \
   }
   
   template <typename Stream>
   struct memory_dump_visitor : public base_visitor {
      memory_dump_visitor(Stream&& stream) : stream(stream) {}
      CONTROL_FLOW_OPS(MEMORY_DUMP_CONTROL_FLOW_VISIT)
      BR_TABLE_OP(MEMORY_DUMP_BR_TABLE_VISIT)
      RETURN_OP(MEMORY_DUMP_OP_VISIT)
      CALL_OPS(MEMORY_DUMP_CALL_VISIT)
      PARAMETRIC_OPS(MEMORY_DUMP_OP_VISIT)
      VARIABLE_ACCESS_OPS(MEMORY_DUMP_VARIABLE_ACCESS_VISIT)
      I32_CONSTANT_OPS(MEMORY_DUMP_CONST_VISIT)
      I64_CONSTANT_OPS(MEMORY_DUMP_CONST_VISIT)
      F32_CONSTANT_OPS(MEMORY_DUMP_CONST_VISIT)
      F64_CONSTANT_OPS(MEMORY_DUMP_CONST_VISIT)
      COMPARISON_OPS(MEMORY_DUMP_OP_VISIT)
      NUMERIC_OPS(MEMORY_DUMP_OP_VISIT)
      CONVERSION_OPS(MEMORY_DUMP_OP_VISIT)
      SYNTHETIC_OPS(MEMORY_DUMP_OP_VISIT)
      ERROR_OPS(MEMORY_DUMP_OP_VISIT)
      Stream& stream;
   };

   template <typename Opcode>
   class memory_dump {
      public:
         memory_dump(Opcode* opcodes, size_t size) : _opcodes(opcodes), _size(size) {}

         template <typename Stream>
         void write(Stream&& stream) {
            memory_dump_visitor<Stream> md(std::forward<Stream>(stream));
            for (size_t i=0; i < _size; i++) {
               eosio::vm::visit(std::move(md), std::move(_opcodes[i]));
            }
         }
      private:
         Opcode* _opcodes;
         size_t  _size;
   };
}} // ns eosio::vm
