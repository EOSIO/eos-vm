#pragma once

#include <eosio/vm/allocator.hpp>
#include <eosio/vm/bitcode_writer.hpp>
#include <eosio/vm/config.hpp>
#include <eosio/vm/debug_visitor.hpp>
#include <eosio/vm/execution_context.hpp>
#include <eosio/vm/interpret_visitor.hpp>
#include <eosio/vm/null_writer.hpp>
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

   struct jit {
      template<typename Host, typename Allocator>
      using context = jit_execution_context<Host, Allocator>;
      template<typename Host, typename Allocator, typename Options>
      using parser = binary_parser<machine_code_writer<jit_execution_context<Host, Allocator>>, Options>;
      static constexpr bool is_jit = true;
   };

   struct interpreter {
      template<typename Host, typename Allocator>
      using context = execution_context<Host, Allocator>;
      template<typename Host, typename Allocator, typename Options>
      using parser = binary_parser<bitcode_writer, Options>;
      static constexpr bool is_jit = false;
   };

   struct null_backend {
      template<typename Host, typename Allocator>
      using context = null_execution_context<Host, Allocator>;
      template<typename Host, typename Allocator, typename Options>
      using parser = binary_parser<null_writer, Options>;
      static constexpr bool is_jit = false;
   };

   template <typename HostFunctions = std::nullptr_t, typename Impl = interpreter, typename Allocator = wasm_allocator, typename Options = default_options>
   class backend {
      using host_t     = detail::host_type_t<HostFunctions>;
      using context_t  = typename Impl::template context<HostFunctions, Allocator>;
      using parser_t   = typename Impl::template parser<HostFunctions, Allocator, Options>;
      void construct(host_t* host=nullptr) {
         mod.finalize();
         ctx.set_allocator(memory_alloc);
         if constexpr (!std::is_same_v<HostFunctions, std::nullptr_t>)
            HostFunctions::resolve(mod);
         // FIXME: should not hard code knowledge of null_backend here
         if constexpr (!std::is_same_v<Impl, null_backend>)
            initialize(host);
      }
    public:
      backend(wasm_code&& code, host_t& host, Allocator* alloc, const Options& options = Options{})
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator, options }.parse_module(code, mod), detail::get_max_call_depth(options)) {
         ctx.set_max_pages(detail::get_max_pages(options));
         construct(&host);
      }
      backend(wasm_code&& code, Allocator* alloc, const Options& options = Options{})
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator, options }.parse_module(code, mod), detail::get_max_call_depth(options)) {
         ctx.set_max_pages(detail::get_max_pages(options));
         construct();
      }
      backend(wasm_code& code, host_t& host, Allocator* alloc, const Options& options = Options{})
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator, options }.parse_module(code, mod), detail::get_max_call_depth(options)) {
         ctx.set_max_pages(detail::get_max_pages(options));
         construct(&host);
      }
      backend(wasm_code& code, Allocator* alloc, const Options& options = Options{})
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator, options }.parse_module(code, mod), detail::get_max_call_depth(options)) {
         ctx.set_max_pages(detail::get_max_pages(options));
         construct();
      }
      backend(wasm_code_ptr& ptr, size_t sz, host_t& host, Allocator* alloc, const Options& options = Options{})
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator, options }.parse_module2(ptr, sz, mod), detail::get_max_call_depth(options)) {
         ctx.set_max_pages(detail::get_max_pages(options));
         construct(&host);
      }
      backend(wasm_code_ptr& ptr, size_t sz, Allocator* alloc, const Options& options = Options{})
         : memory_alloc(alloc), ctx(parser_t{ mod.allocator, options }.parse_module2(ptr, sz, mod), detail::get_max_call_depth(options)) {
         ctx.set_max_pages(detail::get_max_pages(options));
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

      // Only dynamic options matter.  Parser options will be ignored.
      inline backend& initialize(host_t* host, const Options& new_options) {
         ctx.set_max_call_depth(detail::get_max_call_depth(new_options));
         ctx.set_max_pages(detail::get_max_pages(new_options));
         initialize(host);
         return *this;
      }

      inline backend& initialize(host_t* host=nullptr) {
         if (memory_alloc) {
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

      [[deprecated]]
      inline void set_wasm_allocator(wasm_allocator* alloc) {
         memory_alloc = alloc;
         ctx.set_allocator(memory_alloc);
      }

      inline void set_allocator(Allocator* alloc) {
         memory_alloc = alloc;
         ctx.set_allocator( memory_alloc );
      }

      [[deprecated]]
      inline wasm_allocator* get_wasm_allocator() { return memory_alloc; }
      inline auto            get_allocator() { return memory_alloc; }
      inline module&         get_module() { return mod; }
      inline void            exit(const std::error_code& ec) { ctx.exit(ec); }
      inline auto&           get_context() { return ctx; }

    private:
      Allocator* memory_alloc = nullptr; // non owning pointer
      module          mod;
      context_t       ctx;
   };
}} // namespace eosio::vm
