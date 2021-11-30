#pragma once

#include <eosio/vm/allocator.hpp>
#include <eosio/vm/execution_interface.hpp>
#include <eosio/vm/function_traits.hpp>
#include <eosio/vm/argument_proxy.hpp>
#include <eosio/vm/span.hpp>
#include <eosio/vm/utils.hpp>
#include <eosio/vm/wasm_stack.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace eosio { namespace vm {
   // types for host functions to use
   typedef std::nullptr_t standalone_function_t;
   struct no_match_t {};
   struct invoke_on_all_t {};

   template <typename Host_Type=standalone_function_t, typename Execution_Interface=execution_interface>
   struct running_context {
      using running_context_t = running_context<Execution_Interface>;
      inline explicit running_context(Host_Type* host, const Execution_Interface& ei) : host(host), interface(ei) {}
      inline explicit running_context(Host_Type* host, Execution_Interface&& ei) : host(host), interface(ei) {}

      inline void* access(wasm_ptr_t addr=0) const { return (char*)interface.get_memory() + addr; }

      inline Execution_Interface& get_interface() { return interface; }
      inline const Execution_Interface& get_interface() const { return interface; }

      inline decltype(auto) get_host() { return *host; }

      template <typename T, typename U>
      inline auto validate_pointer(U ptr, wasm_size_t len) const {
         return get_interface().template validate_pointer<T>(ptr, len);
      }

      template<typename T>
      inline auto validate_null_terminated_pointer(T ptr) const {
         return get_interface().validate_null_terminated_pointer(ptr);
      }

      Host_Type* host;
      Execution_Interface interface;
   };

   // Used to prevent base class overloads of from_wasm from being hidden.
   template<typename T>
   struct tag {};

#define EOS_VM_FROM_WASM_ADD_TAG(...) (__VA_ARGS__, ::eosio::vm::tag<T> = {})

#define EOS_VM_FROM_WASM(TYPE, PARAMS) \
   template <typename T>                    \
   auto from_wasm EOS_VM_FROM_WASM_ADD_TAG PARAMS const -> std::enable_if_t<std::is_same_v<T, TYPE>, TYPE>

   template <typename Host, typename Execution_Interface=execution_interface>
   struct type_converter : public running_context<Host, Execution_Interface> {
      using base_type = running_context<Host, Execution_Interface>;
      using base_type::running_context;
      using base_type::get_host;

      // TODO clean this up and figure out a more elegant way to get this for the macro
      using elem_type = operand_stack_elem;

      EOS_VM_FROM_WASM(bool, (uint32_t value)) { return value ? 1 : 0; }
      uint32_t to_wasm(bool&& value) { return value ? 1 : 0; }
      template<typename T>
      no_match_t to_wasm(T&&);

      template <typename T>
      auto from_wasm(wasm_ptr_t ptr, wasm_size_t len, tag<T> = {}) const
         -> std::enable_if_t<is_span_type_v<T>, T> {
         auto p = this->template validate_pointer<typename T::value_type>(ptr, len);
         return {static_cast<typename T::pointer>(p), len};
      }

      template <typename T>
      auto from_wasm(wasm_ptr_t ptr, wasm_size_t len, tag<T> = {}) const
         -> std::enable_if_t< is_argument_proxy_type_v<T> &&
                              is_span_type_v<typename T::proxy_type>, T> {
         auto p = this->template validate_pointer<typename T::pointee_type>(ptr, len);
         return {p, len};
      }

      template <typename T>
      auto from_wasm(wasm_ptr_t ptr, tag<T> = {}) const
         -> std::enable_if_t< is_argument_proxy_type_v<T> &&
                              std::is_pointer_v<typename T::proxy_type>, T> {
         auto p = this->template validate_pointer<typename T::pointee_type>(ptr, 1);
         return {p};
      }

      template<typename T>
      inline decltype(auto) as_value(const elem_type& val) const {
         if constexpr (std::is_integral_v<T> && sizeof(T) == 4)
            return static_cast<T>(val.template get<i32_const_t>().data.ui);
         else if constexpr (std::is_integral_v<T> && sizeof(T) == 8)
            return static_cast<T>(val.template get<i64_const_t>().data.ui);
         else if constexpr (std::is_floating_point_v<T> && sizeof(T) == 4)
            return static_cast<T>(val.template get<f32_const_t>().data.f);
         else if constexpr (std::is_floating_point_v<T> && sizeof(T) == 8)
            return static_cast<T>(val.template get<f64_const_t>().data.f);
         else if constexpr (std::is_void_v<std::decay_t<std::remove_pointer_t<T>>>)
            return base_type::access(val.template get<i32_const_t>().data.ui);
         else
            return no_match_t{};
      }

      template <typename T>
      inline constexpr auto as_result(T&& val) const {
         if constexpr (std::is_integral_v<T> && sizeof(T) == 4)
            return i32_const_t{ static_cast<uint32_t>(val) };
         else if constexpr (std::is_integral_v<T> && sizeof(T) == 8)
            return i64_const_t{ static_cast<uint64_t>(val) };
         else if constexpr (std::is_floating_point_v<T> && sizeof(T) == 4)
            return f32_const_t{ static_cast<float>(val) };
         else if constexpr (std::is_floating_point_v<T> && sizeof(T) == 8)
            return f64_const_t{ static_cast<double>(val) };
         else if constexpr (std::is_void_v<std::decay_t<std::remove_pointer_t<T>>>)
            return i32_const_t{ static_cast<uint32_t>(reinterpret_cast<uintptr_t>(val) -
                                                      reinterpret_cast<uintptr_t>(this->access())) };
         else
            return no_match_t{};
      }
   };

   namespace detail {
      template<typename T>
      constexpr bool is_tag_v = false;
      template<typename T>
      constexpr bool is_tag_v<tag<T>> = true;

      template<typename Tuple, std::size_t... N>
      std::tuple<std::tuple_element_t<N, Tuple>...> tuple_select(std::index_sequence<N...>);

      template<typename T>
      using strip_tag = std::conditional_t<is_tag_v<std::tuple_element_t<std::tuple_size_v<T> - 1, T>>,
                                           decltype(tuple_select<T>(std::make_index_sequence<std::tuple_size_v<T> - 1>())),
                                           T>;

      template <class TC, typename T>
      using from_wasm_type_deducer_t = strip_tag<flatten_parameters_t<&TC::template from_wasm<T>>>;
      template <class TC, typename T>
      using to_wasm_type_deducer_t = decltype(std::declval<TC>().to_wasm(std::declval<T>()));

      template <std::size_t N, typename Type_Converter>
      inline constexpr const auto& pop_value(Type_Converter& tc) { return tc.get_interface().operand_from_back(N); }

      template <typename S, typename Type_Converter>
      constexpr bool has_as_value() {
         return !std::is_same_v<no_match_t, std::decay_t<decltype(
               std::declval<Type_Converter>().template as_value<S>(pop_value<0>(std::declval<Type_Converter&>())))>>;
      }

      template <typename S, typename Type_Converter>
      constexpr bool has_as_result() {
         return !std::is_same_v<no_match_t, std::decay_t<decltype(
               std::declval<Type_Converter>().template as_result<S>(std::declval<S&&>()))>>;
      }

      template <typename T, class Type_Converter>
      inline constexpr std::size_t value_operand_size() {
         if constexpr (has_as_value<T, Type_Converter>())
            return 1;
         else
            return std::tuple_size_v<from_wasm_type_deducer_t<Type_Converter, T>>;
      }

      template <typename T, class Type_Converter>
      static inline constexpr std::size_t value_operand_size_v = value_operand_size<T, Type_Converter>();

      template <typename Args, std::size_t I, class Type_Converter>
      inline constexpr std::size_t total_operands() {
         if constexpr (I >= std::tuple_size_v<Args>)
            return 0;
         else {
            constexpr std::size_t sz = value_operand_size_v<std::tuple_element_t<I, Args>, Type_Converter>;
            return sz + total_operands<Args, I+1, Type_Converter>();
         }
      }

      template <typename Args, class Type_Converter>
      static inline constexpr std::size_t total_operands_v = total_operands<Args, 0, Type_Converter>();

      template <typename S, typename Type_Converter>
      constexpr inline static bool has_from_wasm_v = EOS_VM_HAS_TEMPLATE_MEMBER_TY(Type_Converter, from_wasm<S>);

      template <typename S, typename Type_Converter>
      constexpr inline static bool has_to_wasm_v =
         !std::is_same_v<no_match_t, to_wasm_type_deducer_t<Type_Converter, S>>;

      template <typename Args, typename S, std::size_t At, class Type_Converter, std::size_t... Is>
      inline constexpr decltype(auto) create_value(Type_Converter& tc, std::index_sequence<Is...>) {
         constexpr std::size_t offset = total_operands_v<Args, Type_Converter> - 1;
         if constexpr (has_from_wasm_v<S, Type_Converter>) {
            using arg_types = from_wasm_type_deducer_t<Type_Converter, S>;
            return tc.template from_wasm<S>(tc.template as_value<std::tuple_element_t<Is, arg_types>>(pop_value<offset - (At + Is)>(tc))...);
         } else {
            static_assert(has_as_value<S, Type_Converter>(), "no type conversion found for type, define a from_wasm for this type");
            return tc.template as_value<S>(pop_value<offset - At>(tc));
         }
      }

      template <typename S, typename Type_Converter>
      inline constexpr std::size_t skip_amount() {
         if constexpr (has_as_value<S, Type_Converter>()) {
            return 1;
         } else {
            return std::tuple_size_v<from_wasm_type_deducer_t<Type_Converter, S>>;
         }
      }

      template <typename Args, std::size_t At, std::size_t Skip_Amt, class Type_Converter>
      inline constexpr auto get_values(Type_Converter& tc) {
         if constexpr (At >= std::tuple_size_v<Args>)
            return std::tuple<>{};
         else {
            using source_t = std::tuple_element_t<At, Args>;
            constexpr std::size_t skip_amt = skip_amount<source_t, Type_Converter>();
            using converted_t = decltype(create_value<Args, source_t, Skip_Amt>(tc, std::make_index_sequence<skip_amt>{}));
            auto tail = get_values<Args, At+1, Skip_Amt + skip_amt>(tc);
            return std::tuple_cat(std::tuple<converted_t>(create_value<Args, source_t, Skip_Amt>(tc, std::make_index_sequence<skip_amt>{})),
                                  std::move(tail));
         }
      }

      template <typename Type_Converter, typename T>
      constexpr auto resolve_result(Type_Converter& tc, T&& val) {
         if constexpr (has_to_wasm_v<T, Type_Converter>) {
            return tc.as_result(tc.to_wasm(static_cast<T&&>(val)));
         } else {
            return tc.as_result(static_cast<T&&>(val));
         }
      }

      template <bool Once, std::size_t Cnt, typename T, typename F>
      inline constexpr void invoke_on_impl(F&& fn) {
         if constexpr (Once && Cnt == 0) {
            std::invoke(fn);
         }
      }

      template <bool Once, std::size_t Cnt, typename T, typename F, typename Arg, typename... Args>
      inline constexpr void invoke_on_impl(F&& fn, const Arg& arg, const Args&... args) {
         if constexpr (Once) {
            if constexpr (Cnt == 0)
               std::invoke(fn, arg, args...);
         } else {
            if constexpr (std::is_same_v<T, Arg> || std::is_same_v<T, invoke_on_all_t>)
               std::invoke(fn, arg, args...);
            invoke_on_impl<Once, Cnt+1, T>(std::forward<F>(fn), args...);
         }
      }

      template <typename Precondition, typename Type_Converter, typename Args, std::size_t... Is>
      inline static void precondition_runner(Type_Converter& ctx, const Args& args, std::index_sequence<Is...>) {
         Precondition::condition(ctx, std::get<Is>(args)...);
      }

      template <std::size_t I, typename Preconditions, typename Type_Converter, typename Args>
      inline static void preconditions_runner(Type_Converter& ctx, const Args& args) {
         constexpr std::size_t args_size = std::tuple_size_v<Args>;
         constexpr std::size_t preconds_size = std::tuple_size_v<Preconditions>;
         if constexpr (I < preconds_size) {
            precondition_runner<std::tuple_element_t<I, Preconditions>>(ctx, args, std::make_index_sequence<args_size>{});
            preconditions_runner<I+1, Preconditions>(ctx, args);
         }
      }
   } //ns detail

   template <bool Once, typename T, typename F, typename... Args>
   void invoke_on(F&& func, const Args&... args) {
      detail::invoke_on_impl<Once, 0, T>(static_cast<F&&>(func), args...);
   }

#define EOS_VM_INVOKE_ON(TYPE, CONDITION) \
   eosio::vm::invoke_on<false, TYPE>(CONDITION, args...);

#define EOS_VM_INVOKE_ON_ALL(CONDITION) \
   eosio::vm::invoke_on<false, eosio::vm::invoke_on_all_t>(CONDITION, args...);

#define EOS_VM_INVOKE_ONCE(CONDITION) \
   eosio::vm::invoke_on<true, eosio::vm::invoke_on_all_t>(CONDITION, args...);

#define EOS_VM_PRECONDITION(NAME, ...)                                       \
   struct NAME {                                                             \
      template <typename Type_Converter, typename... Args>                   \
      inline static decltype(auto) condition(Type_Converter& ctx, const Args&... args) { \
        __VA_ARGS__;                                                         \
      }                                                                      \
   };

   template <auto F, typename Preconditions, typename Type_Converter, typename Host, typename... Args>
   decltype(auto) invoke_impl(Type_Converter& tc, Host* host, Args&&... args) {
      if constexpr (std::is_same_v<Host, standalone_function_t>)
         return std::invoke(F, static_cast<Args&&>(args)...);
      else
         return std::invoke(F, host, static_cast<Args&&>(args)...);
   }

   template <auto F, typename Preconditions, typename Host, typename Args, typename Type_Converter, std::size_t... Is>
   decltype(auto) invoke_with_host_impl(Type_Converter& tc, Host* host, Args&& args, std::index_sequence<Is...>) {
      detail::preconditions_runner<0, Preconditions>(tc, args);
      return invoke_impl<F, Preconditions>(tc, host, std::get<Is>(static_cast<Args&&>(args))...);
   }

   template <auto F, typename Preconditions, typename Args, typename Type_Converter, typename Host, std::size_t... Is>
   decltype(auto) invoke_with_host(Type_Converter& tc, Host* host, std::index_sequence<Is...>) {
      constexpr std::size_t args_size = std::tuple_size_v<decltype(detail::get_values<Args, 0, 0>(tc))>;
      return invoke_with_host_impl<F, Preconditions>(tc, host, detail::get_values<Args, 0, 0>(tc), std::make_index_sequence<args_size>{});
   }

   template<typename Type_Converter, typename T>
   void maybe_push_result(Type_Converter& tc, T&& res, std::size_t trim_amt) {
      if constexpr (!std::is_same_v<std::decay_t<T>, maybe_void_t>) {
         tc.get_interface().trim_operands(trim_amt);
         tc.get_interface().push_operand(detail::resolve_result(tc, static_cast<T&&>(res)));
      } else {
         tc.get_interface().trim_operands(trim_amt);
      }
   }

   template <typename Cls, auto F, typename Preconditions, typename R, typename Args, typename Type_Converter, size_t... Is>
   auto create_function(std::index_sequence<Is...>) {
      return std::function<void(Cls*, Type_Converter& )>{ [](Cls* self, Type_Converter& tc) {
            maybe_push_result(tc, (invoke_with_host<F, Preconditions, Args>(tc, self, std::index_sequence<Is...>{}), maybe_void),
                              detail::total_operands_v<Args, Type_Converter>);
         }
      };
   }

   template<typename T>
   auto to_wasm_type();
   template<>
   constexpr auto to_wasm_type<i32_const_t>() { return types::i32; }
   template<>
   constexpr auto to_wasm_type<i64_const_t>() { return types::i64; }
   template<>
   constexpr auto to_wasm_type<f32_const_t>() { return types::f32; }
   template<>
   constexpr auto to_wasm_type<f64_const_t>() { return types::f64; }

   template <typename TC, typename T>
   constexpr auto to_wasm_type_v = to_wasm_type<decltype(detail::resolve_result(std::declval<TC&>(), std::declval<T>()))>();
   template <typename TC>
   constexpr auto to_wasm_type_v<TC, void> = types::ret_void;

   struct host_function {
      std::vector<value_type> params;
      std::vector<value_type> ret;
   };

   inline bool operator==(const host_function& lhs, const func_type& rhs) {
      return lhs.params.size() == rhs.param_types.size() &&
         std::equal(lhs.params.begin(), lhs.params.end(), rhs.param_types.raw()) &&
         lhs.ret.size() == rhs.return_count &&
         (lhs.ret.size() == 0 || lhs.ret[0] == rhs.return_type);
   }
   inline bool operator==(const func_type& lhs, const host_function& rhs) {
      return rhs == lhs;
   }

   template<typename TC, typename Args, std::size_t... Is>
   void get_args(value_type*& out, std::index_sequence<Is...>) {
      ((*out++ = to_wasm_type_v<TC, std::tuple_element_t<Is, Args>>), ...);
   }

   template<typename Type_Converter, typename T>
   void get_args(value_type*& out) {
      if constexpr (detail::has_from_wasm_v<T, Type_Converter>) {
         using args_tuple = detail::from_wasm_type_deducer_t<Type_Converter, T>;
         get_args<Type_Converter, args_tuple>(out, std::make_index_sequence<std::tuple_size_v<args_tuple>>());
      } else {
         *out++ = to_wasm_type_v<Type_Converter, T>;
      }
   }

   template <typename Type_Converter, typename Ret, typename Args, std::size_t... Is>
   host_function function_types_provider(std::index_sequence<Is...>) {
      host_function hf;
      hf.params.resize(detail::total_operands_v<Args, Type_Converter>);
#if (defined(__GNUC__) && !defined(__clang__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"      
      value_type* iter = hf.params.data();
      (get_args<Type_Converter, std::tuple_element_t<Is, Args>>(iter), ...);
#   pragma GCC diagnostic pop
#else
      value_type* iter = hf.params.data();
      (get_args<Type_Converter, std::tuple_element_t<Is, Args>>(iter), ...);
#endif
      if constexpr (to_wasm_type_v<Type_Converter, Ret> != types::ret_void) {
         hf.ret = { to_wasm_type_v<Type_Converter, Ret> };
      }
      return hf;
   }

   using host_func_pair = std::pair<std::string, std::string>;

   struct host_func_pair_hash {
      template <class T, class U>
      std::size_t operator()(const std::pair<T, U>& p) const {
         return std::hash<T>()(p.first) ^ std::hash<U>()(p.second);
      }
   };

   template <typename Cls, typename Execution_Interface=execution_interface, typename Type_Converter=type_converter<Cls, Execution_Interface>>
   struct registered_host_functions {
      using host_type_t           = Cls;
      using execution_interface_t = Execution_Interface;
      using type_converter_t      = Type_Converter;

      struct mappings {
         std::unordered_map<host_func_pair, uint32_t, host_func_pair_hash> named_mapping;
         std::vector<std::function<void(Cls*, Type_Converter&)>>           functions;
         std::vector<host_function>                                        host_functions;
         size_t                                                            current_index = 0;

         template <auto F, typename R, typename Args, typename Preconditions>
         void add_mapping(const std::string& mod, const std::string& name) {
            named_mapping[{mod, name}] = current_index++;
            functions.push_back(
                  create_function<Cls, F, Preconditions, R, Args, Type_Converter>(
                     std::make_index_sequence<std::tuple_size_v<Args>>()));
            host_functions.push_back(
                  function_types_provider<Type_Converter, R, Args>(
                     std::make_index_sequence<std::tuple_size_v<Args>>()));
         }

         static mappings& get() {
            static mappings instance;
            return instance;
         }
      };

      template <auto Func, typename... Preconditions>
      static void add(const std::string& mod, const std::string& name) {
         using args          = flatten_parameters_t<AUTO_PARAM_WORKAROUND(Func)>;
         using res           = return_type_t<AUTO_PARAM_WORKAROUND(Func)>;
         using preconditions = std::tuple<Preconditions...>;
         mappings::get().template add_mapping<Func, res, args, preconditions>(mod, name);
      }

      template <typename Module>
      static void resolve(Module& mod) {
         auto& imports          = mod.import_functions;
         auto& current_mappings = mappings::get();
         for (std::size_t i = 0; i < mod.imports.size(); i++) {
            std::string mod_name =
                  std::string((char*)mod.imports[i].module_str.raw(), mod.imports[i].module_str.size());
            std::string fn_name = std::string((char*)mod.imports[i].field_str.raw(), mod.imports[i].field_str.size());
            EOS_VM_ASSERT(current_mappings.named_mapping.count({ mod_name, fn_name }), wasm_link_exception,
                          std::string("no mapping for imported function ") + fn_name);
            imports[i] = current_mappings.named_mapping[{ mod_name, fn_name }];
            const import_entry& entry = mod.imports[i];
            EOS_VM_ASSERT(entry.kind == Function, wasm_link_exception, std::string("importing non-function ") + fn_name);
            EOS_VM_ASSERT(current_mappings.host_functions[imports[i]] == mod.types[entry.type.func_t], wasm_link_exception, std::string("wrong type for imported function ") + fn_name);
         }
      }

      void operator()(Cls* host, Execution_Interface ei, uint32_t index) {
         const auto& _func = mappings::get().functions[index];
         auto tc = Type_Converter{host, std::move(ei)};
         std::invoke(_func, host, tc);
      }
   };
}} // namespace eosio::vm
