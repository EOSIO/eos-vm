#pragma once

#include <eosio/vm/allocator.hpp>
#include <eosio/vm/exceptions.hpp>
#include <eosio/vm/signals.hpp>
#include <eosio/vm/types.hpp>
#include <eosio/vm/util.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <variant>
#include <vector>
#include <cpuid.h>

namespace eosio { namespace vm { namespace x86_64 {
   // Notes
   // - branch instructions return the address that will need to be updated
   // - label instructions return the address of the target
   // - fix_branch will be called when the branch target is resolved
   // - memory base is stored in register rsi
   template <typename Context>
   class writer {
      public:
         writer(growable_allocator& alloc, std::size_t source_bytes, module& mod) :
            _mod(mod), _code_segment_base(alloc.start_code()) {
            constexpr std::size_t code_size = 4 * 16; // 4 error handlers, each 16 bytes in size
            _code_start = _mod.allocator.alloc<uint8_t>(code_size);
            _code_end   = _code_start + code_size;
            code        = _code_start;

            emit_required_handlers();

            assert(code == _code_end);

            // emit host functions
            const uint32_t num_imported           = mod.get_imported_functions_size();
            const std::size_t host_functions_size = 40 * num_imported;
            _code_start = mod.allocator.alloc<uint8_t>(host_functions_size);
            _code_end   = _code_start + host_functions_size;

            for (uint32_t i=0; i < num_imported; ++i) {
               start_function(code, i);
               emit_host_call(i);
            }

            assert(code == _code_end);

            jmp_table = code;

            if (_mod.tables.size() > 0) {
               _table_element_size = 17; // each element consumes 17 bytes
               const std::size_t table_size = _table_element_size * _mod.tables[0].table.size();
               _code_start = _mod.allocator.alloc<uint8_t>(table_size);
               _code_end   = _code_start + table_size;

               for (uint32_t i=0; i < _mod.tables[0].table.size(); ++i) {
                  uint32_t fn = _mod.tables[0].table[i];
                  if (fn < _mod.fast_functions.size()) {
                     emit_cmp
                  }
               }
            }
         }

      private:
         void emit_required_handlers() {
            fpe_handler            = emit_error_handler(&on_fp_error);
            call_indirect_handler  = emit_error_handler(&on_call_indirect_error);
            type_error_handler     = emit_error_handler(&on_type_error);
            stack_overflow_handler = emit_error_handler(&on_stack_overflow);
         }
   };
}}} // ns eosio::vm::x86
