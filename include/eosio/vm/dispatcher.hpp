#pragma once

#include <eosio/vm/variant.hpp>
#include <eosio/vm/opcodes.hpp>

#define DISPATCH(VISITOR, VARIANT) \
   {  \
      decltype(VISITOR)& __eos_vm_visitor = VISITOR; \
      decltype(VARIANT)& __eos_vm_variant = VARIANT ; \
      static void* dispatch_table[] = { \
      CONTROL_FLOW_OPS(CREATE_TABLE_ENTRY)               \
      BR_TABLE_OP(CREATE_TABLE_ENTRY)                    \
      RETURN_OP(CREATE_TABLE_ENTRY)                      \
      CALL_OPS(CREATE_TABLE_ENTRY)                       \
      PARAMETRIC_OPS(CREATE_TABLE_ENTRY)                 \
      VARIABLE_ACCESS_OPS(CREATE_TABLE_ENTRY)            \
      MEMORY_OPS(CREATE_TABLE_ENTRY)                     \
      I32_CONSTANT_OPS(CREATE_TABLE_ENTRY)               \
      I64_CONSTANT_OPS(CREATE_TABLE_ENTRY)               \
      F32_CONSTANT_OPS(CREATE_TABLE_ENTRY)               \
      F64_CONSTANT_OPS(CREATE_TABLE_ENTRY)               \
      COMPARISON_OPS(CREATE_TABLE_ENTRY)                 \
      NUMERIC_OPS(CREATE_TABLE_ENTRY)                    \
      CONVERSION_OPS(CREATE_TABLE_ENTRY)                 \
      SYNTHETIC_OPS(CREATE_TABLE_ENTRY)                  \
      ERROR_OPS(CREATE_TABLE_ENTRY)                      \
      }; \
      CONTROL_FLOW_OPS(CREATE_LABEL) \
      BR_TABLE_OP(CREATE_LABEL) \
      RETURN_OP(CREATE_LABEL) \
      CALL_OPS(CREATE_LABEL) \
      PARAMETRIC_OPS(CREATE_LABEL) \
      VARIABLE_ACCESS_OPS(CREATE_LABEL) \
      MEMORY_OPS(CREATE_LABEL) \
      I32_CONSTANT_OPS(CREATE_LABEL) \
      I64_CONSTANT_OPS(CREATE_LABEL) \
      F32_CONSTANT_OPS(CREATE_LABEL) \
      F64_CONSTANT_OPS(CREATE_LABEL) \
      COMPARISON_OPS(CREATE_LABEL) \
      NUMERIC_OPS(CREATE_LABEL) \
      CONVERSION_OPS(CREATE_LABEL) \
      SYNTHETIC_OPS(CREATE_LABEL) \
      ERROR_OPS(CREATE_LABEL) \
   }
