#pragma once

#include <unordered_map>
#include <string_view>
#include <optional>

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
   host_function function_types_provider( Ret(*func)(Args...) ) {
      host_function hf;
      hf.ptr = (void*)func;
      hf.params = {to_wasm_type_v<Args>...};
      if constexpr (to_wasm_type_v<Ret> != types::ret_void) {
         hf.ret = {to_wasm_type_v<Ret>};
      }
      return hf;
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

   struct registered_host_functions {
      struct mappings {
         std::unordered_map<std::string, uint32_t> named_mapping;
         std::vector<host_function>                host_functions;
         size_t                                    current_index = 0;
      };

      static mappings& get_mappings() {
         thread_local mappings _mappings;
         return _mappings;
      }

      template <auto Func>
      static void add(const std::string& mod, const std::string& name) {
         mappings& current_mappings = get_mappings();
         current_mappings.named_mapping[name] = current_mappings.current_index++;
         current_mappings.host_functions.push_back( function_types_provider(Func) ); 
      }

      template <typename Module>
      static void resolve( Module& mod ) {
         mod.import_functions.resize(mod.get_imported_functions_size());
         mappings& current_mappings = get_mappings();
         for (int i=0; i < mod.imports.size(); i++) {
            std::string mod_name{ (char*)mod.imports[i].module_str.raw(), mod.imports[i].module_len };
            std::string fn_name{ (char*)mod.imports[i].field_str.raw(), mod.imports[i].field_len };
            EOS_WB_ASSERT(current_mappings.named_mapping.count(fn_name), wasm_link_exception, "no mapping for imported function");
            mod.import_functions[i] = current_mappings.named_mapping[fn_name];
            std::cout << "fn " << fn_name << "\n";
         }
      }

      template <typename Execution_Context>
      void operator()(Execution_Context& ctx, uint32_t index) {
         static constexpr size_t calling_conv_arg_cnt = 6;
#if not defined __x86_64__ and (__APPLE__ or __linux__)
         static_assert(false, "currently only supporting x86_64 on Linux and Apple");
#endif
         const auto& func = get_mappings().host_functions[index];
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
                  : "a"(func.ptr), "g"(args[0]), "g"(args[1])
                  : "rdi", "rsi");
               break;
            case 3:
               asm("movq %2, %%rdi\n\t"
                   "movq %3, %%rsi\n\t"
                   "movq %4, %%rdx\n\t"
                   "callq *%1\n\t"
                   "movq %%rax, %0"
                  : "=r"(return_val)
                  : "a"(func.ptr), "g"(args[0]), "g"(args[1]), "g"(args[2])
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
                  : "a"(func.ptr), "g"(args[0]), "g"(args[1]), "g"(args[2]), "g"(args[3])
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
                  : "a"(func.ptr), "g"(args[0]), "g"(args[1]), "g"(args[2]), "g"(args[3]), "g"(args[4])
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
                  : "a"(func.ptr), "g"(args[0]), "g"(args[1]), "g"(args[2]), "g"(args[3]), "g"(args[4]), "g"(args[5])
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
                  : "a"(func.ptr), "g"(args[0]), "g"(args[1]), "g"(args[2]), "g"(args[3]), "g"(args[4]), "g"(args[5])
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

   template <auto F, typename Mod, typename Name >
   struct registered_function {
      registered_function() {
         registered_host_functions::add<F>(std::string{Mod::value, Mod::len}, std::string{Name::value, Name::len}); 
      }

      static constexpr auto function = F;
      static constexpr auto name = Name{};
      using name_t = Name;
      static constexpr bool is_member = false; 
   };

}} // ns eosio::wasm_backend
