#pragma once

#include <cstddef>
#include <unordered_map>
#include <string_view>
#include <optional>
#include <functional>
#include <type_traits>
#include <eosio/wasm_backend/wasm_stack.hpp>

#define __BACKEND_GET_ARG(ARG, X, EXPECTED)                                \
   std::visit( overloaded {                                                \
      [&](const EXPECTED& v) {                                             \
         ARG = v.data.ui;                                                  \
      }, [&](auto) {                                                       \
         throw wasm_interpreter_exception{"invalid host function arg"};    \
      }                                                                    \
   }, X)

namespace eosio { namespace wasm_backend {

   namespace detail {

      template <typename... Args, size_t... Is>
      auto get_args_full(std::index_sequence<Is...>) {
         std::tuple<std::decay_t<Args>...> tup;
         return std::tuple<Args...>{std::get<Is>(tup)...};
      }

      template <typename R, typename... Args>
      auto get_args_full(R(Args...)) {
         return get_args_full<Args...>(std::index_sequence_for<Args...>{});
      }

      template <typename R, typename Cls, typename... Args>
      auto get_args_full(R (Cls::*)(Args...)) {
         return get_args_full<Args...>(std::index_sequence_for<Args...>{});
      }

      template <typename T>
      struct return_type_wrapper {
         using type = T;
      };

      template <typename R, typename... Args>
      auto get_return_t(R(Args...)) {
         return return_type_wrapper<R>{};
      }

      template <typename R, typename Cls, typename... Args>
      auto get_return_t(R (Cls::*)(Args...)) {
         return return_type_wrapper<R>{};
      }

      template <typename R, typename... Args>
      auto get_args(R(Args...)) {
         return std::tuple<std::decay_t<Args>...>{};
      }

      template <typename R, typename Cls, typename... Args>
      auto get_args(R (Cls::*)(Args...)) {
         return std::tuple<std::decay_t<Args>...>{};
      }

      template <typename T>
      struct traits {
         static constexpr uint8_t is_integral_offset = 0;
         static constexpr uint8_t is_lval_ref_offset = 1;
         static constexpr uint8_t is_ptr_offset      = 2;
         static constexpr uint8_t is_float_offset    = 3;
         static constexpr uint8_t is_4_bytes_offset  = 4;
         static constexpr uint8_t value = (std::is_integral<T>::value << is_integral_offset) |
                                          (std::is_lvalue_reference<T>::value << is_lval_ref_offset) |
                                          (std::is_pointer<T>::value << is_ptr_offset) |
                                          (std::is_floating_point<T>::value << is_float_offset) |
                                          ((sizeof(T) == 4) << is_4_bytes_offset);
         static constexpr uint8_t i32_value_i  = (1 << is_integral_offset) | (1 << is_4_bytes_offset);
         static constexpr uint8_t i32_value_lv = (1 << is_lval_ref_offset) | (1 << is_4_bytes_offset);
         static constexpr uint8_t i32_value_p  = (1 << is_ptr_offset) | (1 << is_4_bytes_offset);
         static constexpr uint8_t i64_value_i  = (1 << is_integral_offset);
         static constexpr uint8_t i64_value_lv = (1 << is_lval_ref_offset);
         static constexpr uint8_t i64_value_p  = (1 << is_ptr_offset);
         static constexpr uint8_t f32_value = (1 << is_float_offset) | (1 << is_4_bytes_offset);
         static constexpr uint8_t f64_value = (1 << is_float_offset);
      };

      template <uint8_t Traits>
      struct _to_wasm_t;

      template <>
      struct _to_wasm_t<traits<int>::i32_value_i> {
         typedef i32_const_t type;
      };

      template <>
      struct _to_wasm_t<traits<int>::i32_value_lv> {
         typedef i32_const_t type;
      };

      template <>
      struct _to_wasm_t<traits<int>::i32_value_p> {
         typedef i32_const_t type;
      };

      template <>
      struct _to_wasm_t<traits<int>::i64_value_i> {
         typedef i64_const_t type;
      };

      template <>
      struct _to_wasm_t<traits<int>::i64_value_lv> {
         typedef i32_const_t type;
      };

      template <>
      struct _to_wasm_t<traits<int>::i64_value_p> {
         typedef i32_const_t type;
      };

      template <>
      struct _to_wasm_t<traits<int>::f32_value> {
         typedef f32_const_t type;
      };

      template <>
      struct _to_wasm_t<traits<int>::f64_value> {
         typedef f64_const_t type;
      };

      template <typename T>
      using to_wasm_t = typename _to_wasm_t<traits<T>::value>::type;

      template <typename S, typename T, typename Backend>
      constexpr auto get_value(Backend& backend, T&& val) -> std::enable_if_t<std::is_same_v<i32_const_t, T> && std::is_lvalue_reference_v<S>, S> {
         return (*((std::remove_reference_t<S>*)(backend.get_wasm_allocator().template get_base_ptr<uint8_t>()+val.data.ui)));
      }

      template <typename S, typename T, typename Backend>
      constexpr auto get_value(Backend& backend, T&& val) -> std::enable_if_t<std::is_same_v<i32_const_t, T> && std::is_pointer_v<S>, S> {
         return (S)(backend.get_wasm_allocator().template get_base_ptr<uint8_t>()+val.data.ui);
      }

      template <typename S, typename T, typename Backend>
      constexpr auto get_value(Backend& backend, T&& val) -> std::enable_if_t<std::is_same_v<i32_const_t, T> && 
                                           (!std::is_lvalue_reference_v<S> && !std::is_pointer_v<S>), S> {
         return val.data.ui;
      }

      template <typename S, typename T, typename Backend>
      constexpr auto get_value(Backend& backend, T&& val) -> std::enable_if_t<std::is_same_v<i64_const_t, T>, S> {
         return val.data.ui;
      }

      template <typename S, typename T, typename Backend>
      constexpr auto get_value(Backend& backend, T&& val) -> std::enable_if_t<std::is_same_v<f32_const_t, T>, S> {
         return val.data.f;
      }

      template <typename S, typename T, typename Backend>
      constexpr auto get_value(Backend& backend, T&& val) -> std::enable_if_t<std::is_same_v<f64_const_t, T>, S> {
         return val.data.f;
      }

      template <typename Backend, typename Cls, auto F, typename R, typename Args, size_t... Is>
      auto create_function(std::index_sequence<Is...>) {
         return std::function<void(Cls*, Backend&, eosio::wasm_backend::operand_stack<Backend>&)>{
            [](Cls* self, Backend& backend, eosio::wasm_backend::operand_stack<Backend>& os) {
               size_t i = sizeof...(Is)-1;
               if constexpr (!std::is_same_v<R, void>) {
                  if constexpr (std::is_same_v<Cls, std::nullptr_t>) {
                     os.push(to_wasm_t<R>{std::invoke(F, get_value<typename std::tuple_element<Is, Args>::type>(
                        backend, std::get<to_wasm_t<typename std::tuple_element<Is, Args>::type>>(
                           os.get_back(i - Is)))...)});
                  } else {
                     os.push(to_wasm_t<R>{std::invoke(F, self, get_value<typename std::tuple_element<Is, Args>::type>(
                        backend, std::get<to_wasm_t<typename std::tuple_element<Is, Args>::type>>(
                           os.get_back(i - Is)))...)});
                  }
               }
               else {
                  if constexpr (std::is_same_v<Cls, std::nullptr_t>) {
                     std::invoke(F, get_value<typename std::tuple_element<Is, Args>::type>(
                        backend, std::get<to_wasm_t<typename std::tuple_element<Is, Args>::type>>(
                           os.get_back(i - Is)))...);
                  } else {
                     std::invoke(F, self, get_value<typename std::tuple_element<Is, Args>::type>(
                        backend, std::get<to_wasm_t<typename std::tuple_element<Is, Args>::type>>(
                           os.get_back(i - Is)))...);
                  }
               }
               os.trim(sizeof...(Is));
            }
         };
      }

      template <typename T>
      constexpr auto to_wasm_type() -> std::enable_if_t<std::is_integral<T>::value, uint8_t> {
         if constexpr (sizeof(T) == 4)
            return types::i32;
         else if constexpr (sizeof(T) == 8)
            return types::i64;
         else
            throw wasm_parse_exception {"incompatible type"};
      }
      template <typename T>
      constexpr auto to_wasm_type() -> std::enable_if_t<std::is_floating_point<T>::value, uint8_t> {
         if constexpr (sizeof(T) == 4)
            return types::f32;
         else if constexpr (sizeof(T) == 8)
            return types::f64;
         else
            throw wasm_parse_exception {"incompatible type"};
      }
      template <typename T>
      constexpr auto to_wasm_type() -> std::enable_if_t<std::is_lvalue_reference<T>::value, uint8_t> {
         return types::i32;
      }
      template <typename T>
      constexpr auto to_wasm_type() -> std::enable_if_t<std::is_rvalue_reference<T>::value, uint8_t> {
         if constexpr (sizeof(std::decay_t<T>) == 4)
            return types::i32;
         else if constexpr (sizeof(std::decay_t<T>) == 8)
            return types::i64;
         else
            throw wasm_parse_exception {"incompatible type"};
      }
      template <typename T>
      constexpr auto to_wasm_type() -> std::enable_if_t<std::is_pointer<T>::value, uint8_t> {
         return types::i32;
      }
      template <typename T>
      constexpr auto to_wasm_type() -> std::enable_if_t<std::is_same<T, void>::value, uint8_t> {
         return types::ret_void;
      }

      template <typename T>
      constexpr bool is_return_void() {
         if constexpr (std::is_same<T, void>::value)
            return true;
         return false;
      }

      template <typename... Args>
      struct to_wasm_type_array {
         static constexpr uint8_t value[] = {to_wasm_type<Args>()...};
      };
   }

   template <typename T>
   constexpr auto to_wasm_type_v = detail::to_wasm_type<T>();

   template <typename T>
   constexpr auto is_return_void_v = detail::is_return_void<T>();

   struct host_function {
      void*                   ptr;
      std::vector<value_type> params;
      std::vector<value_type> ret;
   };

   template <typename Ret, typename... Args>
   host_function function_types_provider() {
      host_function hf;
      hf.ptr = (void*)func;
      hf.params = {to_wasm_type_v<Args>...};
      if constexpr (to_wasm_type_v<Ret> != types::ret_void) {
         hf.ret = {to_wasm_type_v<Ret>};
      }
      return hf;
   }

   template <typename Ret, typename... Args>
   host_function function_types_provider( Ret(*func)(Args...) ) {
      return function_types_provider<Ret, Args...>();
   }

   template <typename Ret, typename Cls, typename... Args>
   host_function function_types_provider( Ret(*func)(Args...) ) {
      return function_types_provider<Ret, Args...>();
   }

   template <char... Str>
   struct host_function_name {
      static constexpr const char value[] = {Str...};
      static constexpr size_t len = sizeof...(Str);
      static constexpr bool is_same(const char* nm, size_t l) {
         if (len == l) {
            bool is_not_same = false;
            for (int i=0; i < len; i++) {
               is_not_same |= nm[i] != value[i];
            }
            return !is_not_same;
         }
         return false;
      }
   };

   #if defined __clang__
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
   #elif defined __GNUC__
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"
   #endif
   template <typename T, T... Str>
   static constexpr host_function_name<Str...> operator ""_hfn() {
      constexpr auto hfn = host_function_name<Str...>{};
      return hfn;
   }
   #if defined __clang__
   #pragma clang diagnostic pop
   #elif defined __GNUC__
   #pragma GCC diagnostic pop
   #endif

   template <typename C, auto C::*MP, typename Name>
   struct registered_member_function {
      static constexpr auto function = MP;
      static constexpr auto name = Name{};
      using name_t = Name;
      static constexpr bool is_member = true;
   };

   template <typename Cls>
   struct registered_host_functions {
      template <typename Backend>
      struct mappings {
         std::unordered_map<std::string, uint32_t> named_mapping;
         std::vector<host_function>                host_functions;
         std::vector<std::function<void(Cls*, Backend&, operand_stack<Backend>&)>> functions;
         size_t                                    current_index = 0;
      };

      template <typename Backend>
      static mappings<Backend>& get_mappings() {
         thread_local mappings<Backend> _mappings;
         return _mappings;
      }

      template <auto Func, typename Backend>
      static void add(const std::string mod, const std::string name) {
         using deduced_full_ts = decltype(detail::get_args_full(Func));
         using deduced_ts      = decltype(detail::get_args(Func));
         using res_t           = typename decltype(detail::get_return_t(Func))::type;
         static constexpr auto is = std::make_index_sequence<std::tuple_size<deduced_ts>::value>();

         auto& current_mappings = get_mappings<Backend>();
         current_mappings.named_mapping[name] = current_mappings.current_index++;
         //current_mappings.host_functions.push_back( function_types_provider(Func) );
         current_mappings.functions.push_back( detail::create_function<Backend, Cls, Func, res_t, deduced_full_ts>(is) );
      }

      template <typename Module>
      static void resolve( Module& mod ) {
         mod.import_functions.resize(mod.get_imported_functions_size());
         auto& current_mappings = get_mappings<typename Module::backend_type>();
         for (int i=0; i < mod.imports.size(); i++) {
            std::string mod_name{ (char*)mod.imports[i].module_str.raw(), mod.imports[i].module_len };
            std::string fn_name{ (char*)mod.imports[i].field_str.raw(), mod.imports[i].field_len };
            EOS_WB_ASSERT(current_mappings.named_mapping.count(fn_name), wasm_link_exception, "no mapping for imported function");
            mod.import_functions[i] = current_mappings.named_mapping[fn_name];
            std::cout << "fn " << fn_name << "\n";
         }
      }

      template <typename Execution_Context>
      void operator()(Cls* host, Execution_Context& ctx, uint32_t index) {
         const auto& _func = get_mappings<typename Execution_Context::backend_type>().functions[index];
         std::invoke(_func, host, ctx.get_backend(), ctx.get_operand_stack());
         return;
         static constexpr size_t calling_conv_arg_cnt = 6;
#if not defined __x86_64__ and (__APPLE__ or __linux__)
         static_assert(false, "currently only supporting x86_64 on Linux and Apple");
#endif
         const auto& func = get_mappings<typename Execution_Context::backend_type>().host_functions[index];
         uint64_t args[calling_conv_arg_cnt] = {0};

         int i=0;
         for (;i < func.params.size() && i < calling_conv_arg_cnt; i++) {
            const auto& op = ctx.pop_operand();
            switch (func.params[i]) {
                  case types::i32:
                  {
                     __BACKEND_GET_ARG(args[i], op, i32_const_t);
                     break;
                  }
                  case types::i64:
                  {
                     __BACKEND_GET_ARG(args[i], op, i64_const_t);
                     break;
                  }
                  case types::f32:
                  {
                     __BACKEND_GET_ARG(args[i], op, f32_const_t);
                     break;
                  }
                  case types::f64:
                  {
                     __BACKEND_GET_ARG(args[i], op, f64_const_t);
                     break;
                  }
            }
         }

         const size_t  param_cnt = func.params.size(); // must be volatile to ensure that it is a memory operand
         const bool    has_ret   = func.ret.size();
         const uint8_t ret_type  = has_ret ? func.ret[0] : 0;

         uint64_t return_val = 0;
         switch (func.params.size()) {
            case 0:
               asm( "callq *%1\n\t"
                    "movq %%rax, %0"
                  : "=r"(return_val)
                  : "a"(func.ptr));
               break;
            case 1:
               asm("movq %2, %%rdi\n\t"
                   "callq *%1\n\t"
                   "movq %%rax, %0"
                  : "=r"(return_val)
                  : "a"(func.ptr), "g"(args[0])
                  : "rdi");
               break;
            case 2:
               asm("movq %2, %%rdi\n\t"
                   "movq %3, %%rsi\n\t"
                   "callq *%1\n\t"
                   "movq %%rax, %0"
                  : "=r"(return_val)
                  : "a"(func.ptr), "g"(args[1]), "g"(args[0])
                  : "rdi", "rsi");
               break;
            case 3:
               asm("movq %2, %%rdi\n\t"
                   "movq %3, %%rsi\n\t"
                   "movq %4, %%rdx\n\t"
                   "callq *%1\n\t"
                   "movq %%rax, %0"
                  : "=r"(return_val)
                  : "a"(func.ptr), "g"(args[2]), "g"(args[1]), "g"(args[0])
                  : "rdi", "rsi", "rdx");
               break;
            case 4:
               asm("movq %2, %%rdi\n\t"
                   "movq %3, %%rsi\n\t"
                   "movq %4, %%rdx\n\t"
                   "movq %5, %%rcx\n\t"
                   "callq *%1\n\t"
                   "movq %%rax, %0"
                  : "=r"(return_val)
                  : "a"(func.ptr), "g"(args[3]), "g"(args[2]), "g"(args[1]), "g"(args[0])
                  : "rdi", "rsi", "rdx", "rcx");
               break;
            case 5:
               asm("movq %2, %%rdi\n\t"
                   "movq %3, %%rsi\n\t"
                   "movq %4, %%rdx\n\t"
                   "movq %5, %%rcx\n\t"
                   "movq %6, %%r8\n\t"
                   "callq *%1\n\t"
                   "movq %%rax, %0"
                  : "=r"(return_val)
                  : "a"(func.ptr), "g"(args[4]), "g"(args[3]), "g"(args[2]), "g"(args[1]), "g"(args[0])
                  : "rdi", "rsi", "rdx", "rcx", "r8");
               break;
            case 6:
               asm("movq %2, %%rdi\n\t"
                   "movq %3, %%rsi\n\t"
                   "movq %4, %%rdx\n\t"
                   "movq %5, %%rcx\n\t"
                   "movq %6, %%r8\n\t"
                   "movq %7, %%r9\n\t"
                   "callq *%1\n\t"
                   "movq %%rax, %0"
                  : "=r"(return_val)
                  : "a"(func.ptr), "g"(args[5]), "g"(args[4]), "g"(args[3]), "g"(args[2]), "g"(args[1]), "g"(args[0])
                  : "rdi", "rsi", "rdx", "rcx", "r8", "r9");
               break;
            default:
               for (int j=i; j < param_cnt; j++) {
                  const auto& stack_op = args[j];
                  asm( "pushq %0"
                       :
                       : "a"(stack_op));
               }
               asm("movq %2, %%rdi\n\t"
                   "movq %3, %%rsi\n\t"
                   "movq %4, %%rdx\n\t"
                   "movq %5, %%rcx\n\t"
                   "movq %6, %%r8\n\t"
                   "movq %7, %%r9\n\t"
                   "callq *%1\n\t"
                   "movq %%rax, %0"
                  : "=r"(return_val)

                  : "a"(func.ptr), "g"(args[5]), "g"(args[4]), "g"(args[3]), "g"(args[2]), "g"(args[1]), "g"(args[0])
                  : "rdi", "rsi", "rdx", "rcx", "r8", "r9");
               for (int j=i; j < param_cnt; j++) {
                  asm( "popq %%rdi\n\t"
                       :
                       :
                       : "rdi");
               }
         }

         if (has_ret) {
            switch (ret_type) {
               case types::i32:
                  ctx.push_operand(i32_const_t{static_cast<uint32_t>(return_val)});
                  break;
               case types::i64:
                  ctx.push_operand(i64_const_t{return_val});
                  break;
               case types::f32:
                  ctx.push_operand(f32_const_t{static_cast<uint32_t>(return_val)});
                  break;
               case types::f64:
                  ctx.push_operand(f64_const_t{return_val});
                  break;
            }
         }
      }

   };

   template <typename Cls, auto F, typename Mod, typename Name, typename Backend >
   struct registered_function {
      registered_function() {
         registered_host_functions<Cls>::template add<Cls, F, Backend>(std::string{Mod::value, Mod::len}, std::string{Name::value, Name::len});
      }

      static constexpr auto function = F;
      static constexpr auto name = Name{};
      using name_t = Name;
      static constexpr bool is_member = false;
   };

}} // ns eosio::wasm_backend
