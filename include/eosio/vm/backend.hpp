#pragma once

#include <eosio/vm/allocator.hpp>
#include <eosio/vm/bitcode_writer.hpp>
#include <eosio/vm/config.hpp>
#include <eosio/vm/debug_visitor.hpp>
#include <eosio/vm/execution_context.hpp>
#include <eosio/vm/interpret_visitor.hpp>
#include <eosio/vm/parser.hpp>
#include <eosio/vm/types.hpp>
#include <eosio/vm/x86_64.hpp>

#include <atomic>
#include <exception>
#include <iostream>
#include <optional>
#include <string_view>
#include <system_error>
#include <vector>

namespace eosio { namespace vm {

   namespace detail {
      template <typename HostFunctions>
      struct host_type {
         using type = typename HostFunctions::host_type_t;
      };
      template <>
      struct host_type<std::nullptr_t> {
         using type = std::nullptr_t;
      };

      template <typename HF>
      using host_type_t = typename host_type<HF>::type;
   }

   struct jit {
      template<typename Host>
      using context = jit_execution_context<Host>;
      template<typename Host>
      using parser = binary_parser<machine_code_writer<jit_execution_context<Host>>>;
      static constexpr bool is_jit = true;
   };

   struct interpreter {
      template<typename Host>
      using context = execution_context<Host>;
      template<typename Host>
      using parser = binary_parser<bitcode_writer>;
      static constexpr bool is_jit = false;
   };

   template <typename HostFunctions = std::nullptr_t, typename Impl = interpreter>
   class backend {
      using host_t     = detail::host_type_t<HostFunctions>;
      using context_t  = typename Impl::template context<host_t>;
      using parser_t   = typename Impl::template parser<host_t>;
      void construct(host_t* host=nullptr) {
         mod.finalize();
         ctx.set_wasm_allocator(memory_alloc);
         if constexpr (!std::is_same_v<HostFunctions, std::nullptr_t>)
            HostFunctions::resolve(mod);
         initialize(host);
      }
    public:
      backend(wasm_code&& code, host_t& host, wasm_allocator* alloc)
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator }.parse_module(code, mod)) {
         construct(&host);
      }
      backend(wasm_code&& code, wasm_allocator* alloc)
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator }.parse_module(code, mod)) {
         construct();
      }
      backend(wasm_code& code, host_t& host, wasm_allocator* alloc)
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator }.parse_module(code, mod)) {
         construct(&host);
      }
      backend(wasm_code& code, wasm_allocator* alloc)
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator }.parse_module(code, mod)) {
         construct();
      }
      backend(wasm_code_ptr& ptr, size_t sz, host_t& host, wasm_allocator* alloc)
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator }.parse_module2(ptr, sz, mod)) {
         construct(&host);
      }
      backend(wasm_code_ptr& ptr, size_t sz, wasm_allocator* alloc)
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator }.parse_module2(ptr, sz, mod)) {
         construct();
      }

      template <typename... Args>
      inline auto operator()(host_t& host, const std::string_view& mod, const std::string_view& func, Args... args) {
         return call(host, mod, func, args...);
      }

      template <typename... Args>
      inline bool operator()(const std::string_view& mod, const std::string_view& func, Args... args) {
         return call(mod, func, args...);
      }

      inline backend& initialize(host_t* host=nullptr) {
         if (memory_alloc) {
            if (mod.memories.size())
               memory_alloc->reset(mod.memories[0].limits.initial);
            else
               memory_alloc->reset();
            ctx.reset();
            ctx.execute_start(host, interpret_visitor(ctx));
         }
         return *this;
      }

      inline backend& initialize(host_t& host) {
         return initialize(&host);
      }

      template <typename... Args>
      inline bool call_indirect(host_t* host, uint32_t func_index, Args... args) {
         if constexpr (eos_vm_debug) {
            ctx.execute_func_table(host, debug_visitor(ctx), func_index, args...);
         } else {
            ctx.execute_func_table(host, interpret_visitor(ctx), func_index, args...);
         }
         return true;
      }

      template <typename... Args>
      inline bool call(host_t* host, uint32_t func_index, Args... args) {
         if constexpr (eos_vm_debug) {
            ctx.execute(host, debug_visitor(ctx), func_index, args...);
         } else {
            ctx.execute(host, interpret_visitor(ctx), func_index, args...);
         }
         return true;
      }

      template <typename... Args>
      inline bool call(host_t& host, const std::string_view& mod, const std::string_view& func, Args... args) {
         if constexpr (eos_vm_debug) {
            ctx.execute(&host, debug_visitor(ctx), func, args...);
         } else {
            ctx.execute(&host, interpret_visitor(ctx), func, args...);
         }
         return true;
      }

      template <typename... Args>
      inline bool call(const std::string_view& mod, const std::string_view& func, Args... args) {
         if constexpr (eos_vm_debug) {
            ctx.execute(nullptr, debug_visitor(ctx), func, args...);
         } else {
            ctx.execute(nullptr, interpret_visitor(ctx), func, args...);
         }
         return true;
      }

      template <typename... Args>
      inline auto call_with_return(host_t& host, const std::string_view& mod, const std::string_view& func, Args... args ) {
         if constexpr (eos_vm_debug) {
            return ctx.execute(&host, debug_visitor(ctx), func, args...);
         } else {
            return ctx.execute(&host, interpret_visitor(ctx), func, args...);
         }
      }

      template <typename... Args>
      inline auto call_with_return(const std::string_view& mod, const std::string_view& func, Args... args) {
         if constexpr (eos_vm_debug) {
            return ctx.execute(nullptr, debug_visitor(ctx), func, args...);
         } else {
            return ctx.execute(nullptr, interpret_visitor(ctx), func, args...);
         }
      }

      template<typename Watchdog, typename F>
      inline void timed_run(Watchdog&& wd, F&& f) {
         std::atomic<bool>       _timed_out = false;
         auto reenable_code = scope_guard{[&](){
            if (_timed_out) {
               mod.allocator.enable_code(Impl::is_jit);
            }
         }};
         try {
            auto wd_guard = wd.scoped_run([this,&_timed_out]() {
               _timed_out = true;
               mod.allocator.disable_code();
            });
            static_cast<F&&>(f)();
         } catch(wasm_memory_exception&) {
            if (_timed_out) {
               throw timeout_exception{ "execution timed out" };
            } else {
               throw;
            }
         }
      }

      template <typename Watchdog>
      inline void execute_all(Watchdog&& wd, host_t& host) {
         timed_run(static_cast<Watchdog&&>(wd), [&]() {
            for (int i = 0; i < mod.exports.size(); i++) {
               if (mod.exports[i].kind == external_kind::Function) {
                  std::string s{ (const char*)mod.exports[i].field_str.raw(), mod.exports[i].field_str.size() };
                  ctx.execute(host, interpret_visitor(ctx), s);
               }
            }
         });
      }

      template <typename Watchdog>
      inline void execute_all(Watchdog&& wd) {
         timed_run(static_cast<Watchdog&&>(wd), [&]() {
            for (int i = 0; i < mod.exports.size(); i++) {
               if (mod.exports[i].kind == external_kind::Function) {
                  std::string s{ (const char*)mod.exports[i].field_str.raw(), mod.exports[i].field_str.size() };
                  ctx.execute(nullptr, interpret_visitor(ctx), s);
               }
            }
         });
      }

      inline void set_wasm_allocator(wasm_allocator* alloc) {
         memory_alloc = alloc;
         ctx.set_wasm_allocator(memory_alloc);
      }

      inline wasm_allocator* get_wasm_allocator() { return memory_alloc; }
      inline module&         get_module() { return mod; }
      inline void            exit(const std::error_code& ec) { ctx.exit(ec); }
      inline auto&           get_context() { return ctx; }

    private:
      wasm_allocator* memory_alloc = nullptr; // non owning pointer
      module          mod;
      context_t       ctx;
   };
}} // namespace eosio::vm
