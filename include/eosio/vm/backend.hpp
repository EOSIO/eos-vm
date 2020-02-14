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
      struct host_function_resolver {
         constexpr host_function_resolver() = default;
         template <typename Module>
         inline constexpr explicit host_function_resolver(Module& mod) {
            mod.finalize();
            HostFunctions::resolve(mod);
         }
      };
      template <>
      struct host_function_resolver<nullptr_t> {
         constexpr host_function_resolver() = default;
         template <typename Module>
         inline constexpr explicit host_function_resolver(Module& mod){}
      };

      template <typename HostFunctions>
      struct host_type {
         using type = typename HostFunctions::host_type_t;
      };
      template <>
      struct host_type<nullptr_t> {
         using type = nullptr_t;
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

   template <typename HostFunctions = nullptr_t, typename Impl = interpreter>
   class backend : public detail::host_function_resolver<HostFunctions> {
      using host_t     = detail::host_type_t<HostFunctions>;
    public:
      template <typename Host = detail::host_type_t<HostFunctions>>
      backend(wasm_code& code, wasm_allocator* alloc=nullptr, Host* host=nullptr)
         : memory_alloc(alloc), ctx(typename Impl::template parser<Host>{ mod.allocator }.parse_module(code, mod)) {
         mod.finalize();
         ctx.set_wasm_allocator(alloc);
         if constexpr (!std::is_same_v<HostFunctions, nullptr_t>)
            HostFunctions::resolve(mod);
         if (alloc)
            initialize(host);
      }

      template <typename Host = detail::host_type_t<HostFunctions>>
      backend(wasm_code_ptr& ptr, size_t sz, wasm_allocator* alloc=nullptr, Host* host=nullptr) : ctx(typename Impl::template parser<Host>{ mod.allocator }.parse_module2(ptr, sz, mod)) {
         mod.finalize();
         ctx.set_wasm_allocator(alloc);
         if constexpr (!std::is_same_v<HostFunctions, nullptr_t>)
            HostFunctions::resolve(mod);
         if (alloc)
            initialize(host);
      }

      template <typename Host, typename... Args>
      inline bool operator()(Host* host, const std::string_view& mod, const std::string_view& func, Args... args) {
         return call(host, mod, func, args...);
      }

      template <typename Host = detail::host_type_t<HostFunctions>>
      inline backend& initialize(Host* host=nullptr) {
         if(mod.memories.size())
            memory_alloc->reset(mod.memories[0].limits.initial);
         else
            memory_alloc->reset();
         ctx.reset();
         if (host)
            ctx.execute_start(host, interpret_visitor(ctx));
         return *this;
      }

      template <typename Host, typename... Args>
      inline bool call_indirect(Host* host, uint32_t func_index, Args... args) {
         if constexpr (eos_vm_debug) {
            ctx.execute_func_table(host, debug_visitor(ctx), func_index, args...);
         } else {
            ctx.execute_func_table(host, interpret_visitor(ctx), func_index, args...);
         }
         return true;
      }

      template <typename Host, typename... Args>
      inline bool call(Host* host, uint32_t func_index, Args... args) {
         if constexpr (eos_vm_debug) {
            ctx.execute(host, debug_visitor(ctx), func_index, args...);
         } else {
            ctx.execute(host, interpret_visitor(ctx), func_index, args...);
         }
         return true;
      }

      template <typename Host, typename... Args>
      inline bool call(Host* host, const std::string_view& mod, const std::string_view& func, Args... args) {
         if constexpr (eos_vm_debug) {
            ctx.execute(host, debug_visitor(ctx), func, args...);
         } else {
            ctx.execute(host, interpret_visitor(ctx), func, args...);
         }
         return true;
      }

      template <typename Host, typename... Args>
      inline auto call_with_return(Host* host, const std::string_view& mod, const std::string_view& func,
                                   Args... args) {
         if constexpr (eos_vm_debug) {
            return ctx.execute(host, debug_visitor(ctx), func, args...);
         } else {
            return ctx.execute(host, interpret_visitor(ctx), func, args...);
         }
      }

      void print_result(const std::optional<operand_stack_elem>& result) {
         if(result) {
            std::cout << "result: ";
            if (result->is_a<i32_const_t>())
               std::cout << "i32:" << result->to_ui32();
            else if (result->is_a<i64_const_t>())
               std::cout << "i64:" << result->to_ui64();
            else if (result->is_a<f32_const_t>())
               std::cout << "f32:" << result->to_f32();
            else if (result->is_a<f64_const_t>())
              std::cout << "f64:" << result->to_f64();
            std::cout << std::endl;
        }
      }

      template<typename Watchdog, typename F>
      void timed_run(Watchdog&& wd, F&& f) {
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

      template <typename Watchdog, typename Host=nullptr_t>
      inline void execute_all(Watchdog&& wd, Host* host = nullptr) {
         timed_run(static_cast<Watchdog&&>(wd), [&]() {
            for (int i = 0; i < mod.exports.size(); i++) {
               if (mod.exports[i].kind == external_kind::Function) {
                  std::string s{ (const char*)mod.exports[i].field_str.raw(), mod.exports[i].field_str.size() };
	          if constexpr (eos_vm_debug) {
                     print_result(ctx.execute(host, debug_visitor(ctx), s));
	          } else {
	             ctx.execute(host, interpret_visitor(ctx), s);
	          }
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
      wasm_allocator*                       memory_alloc = nullptr; // non owning pointer
      module                                mod;
      typename Impl::template context<host_t> ctx;
   };
}} // namespace eosio::vm
