#pragma once

#include <eosio/vm/allocator.hpp>
#include <eosio/vm/exceptions.hpp>
#include <eosio/vm/signals.hpp>
#include <eosio/vm/types.hpp>
#include <eosio/vm/utils.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <variant>
#include <vector>
#include <cpuid.h>

#define EOS_VM_ERROR_HANDLER(Name) \
   decltype(eosio::vm::x86_64::error_handler(#Name ## _cts))

namespace eosio { namespace vm { namespace x86_64 {

   template <typename Cts>
   struct error_handler {
      error_handler(Cts&&){}
      using type = Cts;
      static constexpr auto value = [](){ vm:throw_<wasm_interpreter_exception>( Cts::value ); };
      void* emitted_loc = nullptr;
   };

   template <typename...Ts>
   struct error_handlers {
      constexpr error_handlers() {}

      template <typename T, std::size_t Index>
      constexpr auto get_impl(T&& handler) const {
         if constexpr (std::is_same_v<std::tuple_element_t<Index, decltype(handlers)>, std::decay_t<T>>)
            return std::get<Index>(handlers);
         else
            return get_impl<T, Index+1>(std::move(handler));
      }

      template <typename T>
      constexpr auto get(T&& handler) const { return get_impl<T, 0>(std::move(handler)); }

      template <typename T>
      constexpr auto get_handler(T&& handler) const { return get(std::move(handler)).value; }

      template <typename T>
      constexpr auto get_emitted_handler(T&& handler) const { return get(std::move(handler)).emitted_loc; }

      template <typename T, typename Encoder>
      constexpr void emit(T&& handler, Encoder& enc) {
         auto& err_handler = get(std::move(handler));
         err_handler.emitted_loc = enc.current_loc();
         // emit a mask for the low 4 bits
         enc.emit_and(registers::rsp, uint8_t(0xF0)); // 0xF sign extended to 0xFFFFFFFFF0
         enc.emit_mov(registers::rax, uint64_t(static_cast<uint64_t>(reinterpret_cast<uintptr_t>(get(std::move(handler)).value))));
         enc.emit_call(registers::rax);
      }

      static constexpr std::size_t size() { return std::tuple_size_v<decltype(handlers)>; }

      std::tuple<error_handler<Ts>...> handlers;
   };

   // Notes
   // - branch instructions return the address that will need to be updated
   // - label instructions return the address of the target
   // - fix_branch will be called when the branch target is resolved
   // - memory base is stored in register rsi
   template <typename Context>
   class writer {
      private:
         struct relocations {
            constexpr relocations() = default;
            constexpr relocations(const std::vector<void*>& relocs) : relocs(relocs) {}
            constexpr relocations(std::vector<void*>&& relocs) : relocs(std::move(relocs)) {}
            constexpr inline void add(void* ptr) { relocs.push_back(ptr); }
            constexpr inline void set(void* ptr) { base = ptr; }

            template <bool Near=false>
            static constexpr inline void fixup(void* branch, void* target) {
               if constexpr (Near) {
                  auto b = static_cast<uint8_t*>(branch);
                  auto t = static_cast<uint8_t*>(target);
                  auto r = static_cast<uint32_t>(t - (b+4)); // relative offset target - (branch + a uint32)
                  EOS_VM_ASSERT( !(r > 0x7fffffffll || r < -0x80000000ll), wasm_interpreter_exception, "far jumps are not allowed with Near=true" );
                  memcpy(branch, &r, 4);
               } else {
                  memcpy(branch, target, 8);
               }
            }

            constexpr bool maybe_fixup(void* branch) {
               if (base) {
                  fixup(branch, base);
                  return true;
               } else {
                  return false;
               }
            }

            constexpr void apply() {
               for (void* br : relocs)
                  fixup(br, base);
            }

            std::vector<void*> relocs = {};
            void*              base   = nullptr;
         };

         struct relocations_table {
            struct host_function_t {};
            struct wasm_function_t {};

            constexpr relocations_table(std::size_t size, encoder& enc) : function_relocs(size), enc(enc) {}
            //constexpr relocations_table(
            void emit_host_function_call(uint32_t fn_num) {
               enc.emit_mov(registers::edx, fn_num);
               //enc.emit_push
            }

            // register a WASM function
            template <typename Tag>
            constexpr inline auto register_function(uint32_t fn_num, Tag&&) -> std::enable_if_t<std::is_same_v<std::decay_t<Tag>, wasm_function_t> ||
                                                                                                std::is_same_v<std::decay_t<Tag>, host_function_t>, void> {
               auto& reloc = function_relocs[fn_num];
               reloc.set(enc.current_loc());
               reloc.apply();
               if constexpr (std::is_same_v<std::decay_t<Tag>, host_function_t>)
                  emit_host_function_call(fn_num);
               //else
                  //emit something
            }

            constexpr inline void register_call(void* ptr, uint32_t fn_num) {
               const auto& reloc = function_relocs[fn_num];
               if (!reloc.maybe_fixup(ptr))
                  reloc.push_back(ptr);
            }

            static inline constexpr host_function_t host_function = {};
            static inline constexpr wasm_function_t wasm_function = {};

            std::vector<relocations> function_relocs = {};
            encoder& enc;
         };

      public:
         writer(growable_allocator& alloc, std::size_t source_bytes, module& mod) :
            mod(mod), enc(mod.allocator.alloc<uint8_t>(0), 0), relocs(mod.get_functions_total(), enc) {
            constexpr std::size_t code_size = 4 * 16; // 4 error handlers, each 16 bytes in size
            enc = encoder(mod.allocator.alloc<uint8_t>(code_size), code_size);

            emit_error_handlers();

            enc.assert_block();

            // emit host functions
            const uint32_t num_imported           = mod.get_imported_functions_size();
            const std::size_t host_functions_size = 40 * num_imported;

            // allocate for host functions
            add_new_section(host_functions_size);

            for (uint32_t i=0; i < num_imported; ++i) {
               relocs.register_function(i, relocations_table::host_function);
               relocs.emit_host_function_call(i);
            }

            enc.assert_block();

            construct_function_table();
         }
         //2153
         //2073 fix_branch

         void construct_function_table() {
            function_table = enc.current_loc();

            if (mod.tables.size() > 0) {
               const std::size_t table_size = table_element_size * mod.tables[0].table.size();
               // allocate for jmp table
               add_new_section(table_size);

               for (uint32_t i=0; i < mod.tables[0].table.size(); ++i) {
                  uint32_t fn = mod.tables[0].table[i];
                  if (fn < mod.fast_functions.size()) {
                     enc.emit_cmp(registers::edx, mod.fast_functions[fn]);
                     register_function(emit_branch_target(&encoder::emit_je), fn);
                  }
               }
            }
         }

         inline void add_new_section(std::size_t sz) { enc.set(mod.allocator.alloc<uint8_t>(sz), sz); }

         //void register_function(
         template <typename Jcc>
         inline void* emit_branch_target(Jcc&& emit_func) {
            void* res = enc.current_loc();
            //enc.emit_func(static_cast<uint32_t>(0xDEADBEEF - static_cast<uint32_t>(reinterpret_cast<uintptr_t>(code))));
            return res;
         }

         void emit_error_handlers() {
            constexpr uint32_t handler_code_size = required_error_handlers::size() * 16; // four handlers * 16 bytes
            enc = encoder(mod.allocator.alloc<uint8_t>(handler_code_size), handler_code_size);
            err_handlers.emit("unreachable_error"_cts, enc);
            err_handlers.emit("fp_error"_cts, enc);
            err_handlers.emit("call_indirect_error"_cts, enc);
            err_handlers.emit("type_error"_cts, enc);
            err_handlers.emit("stack_overflow_error"_cts, enc);
            enc.assert_block();
         }

         static constexpr uint8_t table_element_size = 17; // each function table element should consume 17 bytes
         module&  mod;
         encoder  enc;
         relocations_table relocs;
         std::vector<void*>       registered_error_handlers;
         void*                    function_table;

         using required_error_handlers = error_handlers < EOS_VM_ERROR_HANDLER(unreachable_error),
                                                          EOS_VM_ERROR_HANDLER(fp_error),
                                                          EOS_VM_ERROR_HANDLER(call_indirect_error),
                                                          EOS_VM_ERROR_HANDLER(types_error),
                                                          EOS_VM_ERROR_HANDLER(stack_overflow_error) >;
         required_error_handlers err_handlers;
   };
}}} // ns eosio::vm::x86

#undef EOS_VM_ERROR_HANDLER
