#pragma once

#include <unordered_map>
#include <string_view>

namespace eosio { namespace wasm_backend {

   namespace detail {
      /*
      template <typename R, typename Act, typename... Args>
      auto get_args(R(Act::*p)(Args...)) { return std::tuple<std::decay_t<Args>...>{}; }

      typename <auto Func>
      using deduced_args = decltype(get_args(Func));
      */

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
      constexpr auto to_wasm_type() -> std::enable_if_t<std::is_lvalue_reference<T>::value, uint8_t> {
         return types::i32;
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

   template <typename Ret, typename... Args>
   auto function_type_provider( Ret(Args...) ) {
      func_type ft;
      ft.form = 0;
      ft.param_count = sizeof...(Args);
      ft.param_types.resize(sizeof...(Args));
      for (int i=0; i < sizeof...(Args); i++)
         ft.param_types.at(i) = detail::to_wasm_type_array<Args...>::value[i];
      ft.return_count = is_return_void_v<Ret>;
      ft.return_type  = to_wasm_type_v<Ret>;
      return ft;
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
   #pragma clang diagnostic "-Wgnu-string-literal-operator-template"
   #elif defined __GNUC__
   #pragma GCC diagnostic push
   #pragma GCC diagnostic "-Wgnu-string-literal-operator-template"
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

   template <auto F, typename Name >
   struct registered_function {
      static constexpr auto function = F;
      static constexpr auto name = Name{};
      using name_t = Name;
      static constexpr bool is_member = false; 
   };

   struct host_function {
      void*                ptr;
      std::vector<uint8_t> arg_types;
      std::vector<uint8_t> ret_types;
   };

   struct registered_host_functions {
      template <typename... Registered_Funcs>
      registered_host_functions() {
         //host_functions = { {(void*)Registered_Funcs::function, function_type_provider<Registered_Funcs>::type()}... }; 
      }
      registered_host_functions() {
      }

      template <auto Func>
      constexpr void add(std::string_view name) {
         host_functions.emplace_back( (void*)Func, function_type_provider(Func) ); 
      }
      /*
      template <size_t Index, typename RF, typename... RFs>
      static void _resolve( module& mod ) {
         size_t name_size = RF::name_t::len;
         if (Index >= mod.import_functions.size())
            return;
         else {
            bool found_import = false;
            for (int i=0; i < mod.imports.size(); i++) {
               if (mod.imports[i].kind != external_kind::Function)
                  continue;
               if (RF::name_t::is_same((const char*)mod.imports[i].field_str.raw(), mod.imports[i].field_len)) {
                  mod.import_functions[i] = Index;
                  found_import = true;
                  std::cout << "Found it " << RF::name_t::value << "\n";
               }
            }
            //EOS_WB_ASSERT(found_import, wasm_interpreter_exception, "unresolved import");
            if constexpr (sizeof...(RFs) >= 1)
               _resolve<Index+1, RFs...>(mod);
         }
      }
      */

      static void resolve( module& mod ) {
         mod.import_functions.resize(mod.get_imported_functions_size());
         //_resolve<0, Registered_Funcs...>(mod);
      }

      /*
      template <size_t N>
      static constexpr void _call(uint32_t index) {
         if constexpr(index == N)
            std::invoke(std::get<N>(registered));
         else
            _call<N+1>(index);
      }
      */

      std::unordered_map<std::string, uint32_t> named_mapping;
      std::vector<host_function>                host_functions;
      //static constexpr const std::tuple<Registered_Funcs...> registered;
   };

      /*
   template <typename... Funcs> 
   struct native_invoker {
      std::array<void*, sizeof...(Funcs)> functions = {(void*)Funcs...}; 
   };
      */
}} // ns eosio::wasm_backend
