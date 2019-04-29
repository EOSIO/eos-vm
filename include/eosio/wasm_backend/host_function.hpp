#pragma once

#include <cstddef>
#include <utility>
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

   template <typename T>
   struct reduce_type {
      using type = T;
   };
	
   template <>
   struct reduce_type<bool> {
      using type = uint32_t;
   };

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

   template <typename R, typename Cls, typename... Args>
   auto get_args_full(R (Cls::*)(Args...)const) {
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

   template <typename R, typename Cls, typename... Args>
   auto get_return_t(R (Cls::*)(Args...)const) {
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

   template <typename R, typename Cls, typename... Args>
   auto get_args(R (Cls::*)(Args...)const) {
      return std::tuple<std::decay_t<Args>...>{};
   }

   template <typename T>
   struct traits {
      static constexpr uint8_t is_integral_offset = 0;
      static constexpr uint8_t is_lval_ref_offset = 1;
      static constexpr uint8_t is_ptr_offset      = 2;
      static constexpr uint8_t is_float_offset    = 3;
      static constexpr uint8_t is_4_bytes_offset  = 4;
      static constexpr uint8_t value = (std::is_integral<typename reduce_type<T>::type>::value << is_integral_offset) |
                                       (std::is_lvalue_reference<typename reduce_type<T>::type>::value << is_lval_ref_offset) |
                                       (std::is_pointer<typename reduce_type<T>::type>::value << is_ptr_offset) |
                                       (std::is_floating_point<typename reduce_type<T>::type>::value << is_float_offset) |
                                       ((sizeof(typename reduce_type<T>::type) == 4) << is_4_bytes_offset);
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
      return (*((std::remove_reference_t<S>*)(backend.get_wasm_allocator()->template get_base_ptr<uint8_t>()+val.data.ui)));
   }

   template <typename S, typename T, typename Backend>
   constexpr auto get_value(Backend& backend, T&& val) -> std::enable_if_t<std::is_same_v<i32_const_t, T> && std::is_pointer_v<S>, S> {
      return (S)(backend.get_wasm_allocator()->template get_base_ptr<uint8_t>()+val.data.ui);
   }

   template <typename S, typename T, typename Backend>
   constexpr auto get_value(Backend& backend, T&& val) -> std::enable_if_t<std::is_same_v<i32_const_t, T> && 
                                          std::is_fundamental_v<S> &&
                                          (!std::is_lvalue_reference_v<S> && !std::is_pointer_v<S>), S> {
      return val.data.ui;
   }

   template <typename S, typename T, typename Backend>
   constexpr auto get_value(Backend& backend, T&& val) -> std::enable_if_t<std::is_same_v<i64_const_t, T> &&
                                                                           std::is_fundamental_v<S>, S> {
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

   template <typename Backend, typename Cls, typename Cls2, auto F, typename R, typename Args, size_t... Is>
   auto create_function(std::index_sequence<Is...>) {
      return std::function<void(Cls*, Backend&, eosio::wasm_backend::operand_stack<Backend>&)>{
         [](Cls* self, Backend& backend, eosio::wasm_backend::operand_stack<Backend>& os) {
            size_t i = sizeof...(Is)-1;
            if constexpr (!std::is_same_v<R, void>) {
               if constexpr (std::is_same_v<Cls, std::nullptr_t>) {
                  auto res = std::invoke(F, get_value<typename std::tuple_element<Is, Args>::type>(
                             backend, std::get<to_wasm_t<typename std::tuple_element<Is, Args>::type>>(os.get_back(i - Is)))...);
                  os.trim(sizeof...(Is));
                  os.push(*(to_wasm_t<R>*)&res);
               } else {
                  auto res = std::invoke(F, (Cls2*)self, get_value<typename std::tuple_element<Is, Args>::type>(
                             backend, std::get<to_wasm_t<typename std::tuple_element<Is, Args>::type>>(os.get_back(i - Is)))...);
                  os.trim(sizeof...(Is));
                  os.push(*(to_wasm_t<R>*)&res);
               }
            }
            else {
               if constexpr (std::is_same_v<Cls, std::nullptr_t>) {
                  std::invoke(F, get_value<typename std::tuple_element<Is, Args>::type>(
                     backend, std::get<to_wasm_t<typename std::tuple_element<Is, Args>::type>>(
                        os.get_back(i - Is)))...);
               } else {
                  std::invoke(F, (Cls2*)self, get_value<typename std::tuple_element<Is, Args>::type>(
                     backend, std::get<to_wasm_t<typename std::tuple_element<Is, Args>::type>>(
                        os.get_back(i - Is)))...);
               }
               os.trim(sizeof...(Is));
            }
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

   template <typename T>
   constexpr auto to_wasm_type_v = to_wasm_type<T>();

   template <typename T>
   constexpr auto is_return_void_v = is_return_void<T>();

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
  
   using host_func_pair = std::pair<std::string, std::string>; 
   struct host_func_pair_hash {
      template <class T, class U>
      std::size_t operator() (const std::pair<T,U>& p) const {
         return std::hash<T>()(p.first) ^ std::hash<U>()(p.second);
      }
   };

   template <typename Cls>
   struct registered_host_functions {
      template <typename Backend>
      struct mappings {
         std::unordered_map<std::pair<std::string, std::string>, 
		            uint32_t, host_func_pair_hash>  named_mapping;
         std::vector<host_function>                         host_functions;
         std::vector<std::function<
            void(Cls*, Backend&, operand_stack<Backend>&)>> functions;
         size_t                                             current_index = 0;
      };

      template <typename Backend>
      static mappings<Backend>& get_mappings() {
         static mappings<Backend> _mappings;
         return _mappings;
      }

      template <typename Cls2, auto Func, typename Backend>
      static void add(const std::string& mod, const std::string& name) {
         using deduced_full_ts = decltype(get_args_full(Func));
         using deduced_ts      = decltype(get_args(Func));
         using res_t           = typename decltype(get_return_t(Func))::type;
         static constexpr auto is = std::make_index_sequence<std::tuple_size<deduced_ts>::value>();

         auto& current_mappings = get_mappings<Backend>();
         current_mappings.named_mapping[{mod, name}] = current_mappings.current_index++;
         current_mappings.functions.push_back( create_function<Backend, Cls, Cls2, Func, res_t, deduced_full_ts>(is) );
      }

      template <typename Module>
      static void resolve( Module& mod ) {
         mod.import_functions.resize(mod.get_imported_functions_size());
         auto& current_mappings = get_mappings<typename Module::backend_type>();
         for (int i=0; i < mod.imports.size(); i++) {
            std::string mod_name{ (char*)mod.imports[i].module_str.raw(), mod.imports[i].module_len };
            std::string fn_name{ (char*)mod.imports[i].field_str.raw(), mod.imports[i].field_len };
            EOS_WB_ASSERT(current_mappings.named_mapping.count({mod_name, fn_name}), wasm_link_exception, "no mapping for imported function");
            mod.import_functions[i] = current_mappings.named_mapping[{mod_name, fn_name}];
         }
      }

      template <typename Execution_Context>
      void operator()(Cls* host, Execution_Context& ctx, uint32_t index) {
         const auto& _func = get_mappings<typename Execution_Context::backend_type>().functions[index];
         std::invoke(_func, host, ctx.get_backend(), ctx.get_operand_stack());
      }

   };

   template <typename Cls, typename Cls2, auto F, typename Backend >
   struct registered_function {
      registered_function(std::string mod, std::string name) {
         registered_host_functions<Cls>::template add<Cls2, F, Backend>(mod, name);
      }
   };

}} // ns eosio::wasm_backend
