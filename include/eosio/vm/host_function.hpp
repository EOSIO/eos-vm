#pragma once

#include <eosio/vm/allocator.hpp>
#include <eosio/vm/execution_interface.hpp>
#include <eosio/vm/function_traits.hpp>
#include <eosio/vm/reference_proxy.hpp>
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

      template <typename T=char>
      inline T* access_as(wasm_ptr_t addr) const { return reinterpret_cast<T*>(access(addr)); }

      inline Execution_Interface& get_interface() { return interface; }
      inline const Execution_Interface& get_interface() const { return interface; }

      inline decltype(auto) get_host() { return *host; }

      template <typename T>
      inline void validate_pointer(const T* ptr, wasm_size_t len) {
         EOS_VM_ASSERT( len <= std::numeric_limits<wasm_size_t>::max() / (wasm_size_t)sizeof(T), wasm_interpreter_exception, "length will overflow" );
         volatile auto check_addr = *(reinterpret_cast<const char*>(ptr) + (len * sizeof(T)) - 1);
         ignore_unused_variable_warning(check_addr);
      }

      inline void validate_null_terminated_pointer(const char* ptr) {
         volatile auto check_addr = std::strlen(ptr);
         ignore_unused_variable_warning(check_addr);
      }
      Host_Type* host;
      Execution_Interface interface;
   };


   namespace detail {
      template <template<typename> class T, typename U>
      auto get_dependent_type(T<U>) -> U;
      template <typename T>
      auto get_dependent_type(T) -> T;
   } // eosio::vm::detail

   template <typename T>
   using dependent_type_t = decltype(detail::get_dependent_type(std::declval<T>()));

#define EOS_VM_GET_MACRO(_1, _2, _3, NAME, ...) NAME

#define EOS_VM_FROM_WASM_ERROR(...) \
   static_assert(false, "EOS_VM_FROM_WASM supplied with the wrong number of arguments");

#define EOS_VM_FROM_WASM_T_IMPL(T, TYPE, PARAMS)                     \
   template <typename T ## _T, typename T=dependent_type_t<T ## _T>> \
   auto from_wasm PARAMS const -> std::enable_if_t<std::is_same_v<T ## _T, TYPE>, TYPE>

#define EOS_VM_FROM_WASM_IMPL(TYPE, PARAMS) \
   template <typename T>                    \
   auto from_wasm PARAMS const -> std::enable_if_t<std::is_same_v<T, TYPE>, TYPE>

#define EOS_VM_FROM_WASM(...) EOS_VM_GET_MACRO(__VA_ARGS__, EOS_VM_FROM_WASM_T_IMPL, \
                                                            EOS_VM_FROM_WASM_IMPL,   \
                                                            EOS_VM_FROM_WASM_ERROR)(__VA_ARGS__)
#define EOS_VM_TO_WASM_ERROR(...) \
   static_assert(false, "EOS_VM_TO_WASM supplied with the wrong number of arguments");

#define EOS_VM_TO_WASM_T_IMPL(TEMPLATE_T, TYPE, PARAMS)              \
   template <typename T ## _T, typename T=dependent_type_t<T ## _T>> \
   auto to_wasm PARAMS const -> std::enable_if_t<std::is_same_v<T ## _T, TYPE>, elem_type>

#define EOS_VM_TO_WASM_IMPL(TYPE, PARAMS) \
   template <typename T>                  \
   auto to_wasm PARAMS const -> std::enable_if_t<std::is_same_v<T, TYPE>, elem_type>

#define EOS_VM_TO_WASM(...) EOS_VM_GET_MACRO(__VA_ARGS__, EOS_VM_TO_WASM_T_IMPL, \
                                                          EOS_VM_TO_WASM_IMPL,   \
                                                          EOS_VM_TO_WASM_ERROR)(__VA_ARGS__)

#define EOS_VM_TYPE(...) decltype(std::declval<__VA_ARGS__>())

   template <typename T>
   struct reference {
      constexpr reference(void* ptr) : value(reinterpret_cast<T*>(ptr)) {}

      operator T&() { return *value; }
      operator const T&() { return *value; }
      T* value;
   };

   template <typename T>
   constexpr auto is_reference_type(reference<T>) { return std::true_type{}; }

   template <typename T>
   constexpr auto is_reference_type(T&&) { return std::false_type{}; }

   template <typename T>
   inline constexpr bool is_reference_type_v = std::is_same_v<decltype(is_reference_type(std::declval<T>())), std::true_type>;

   template <typename Host, typename Execution_Interface=execution_interface>
   struct type_converter : public running_context<Host, Execution_Interface> {
      using base_type = running_context<Host, Execution_Interface>;
      using base_type::running_context;
      using base_type::get_host;

      // TODO clean this up and figure out a more elegant way to get this for the macro
      using elem_type = operand_stack_elem;

      EOS_VM_FROM_WASM(bool, (const elem_type& value)) { return as_value<uint32_t>(value) ? 1 : 0; }
      EOS_VM_TO_WASM(bool, (bool value)) { return as_result<uint32_t>(value ? 1 : 0); }

      template <typename T>
      auto from_wasm(const elem_type& ptr, const elem_type& len) const
         -> std::enable_if_t<is_span_type_v<T>, T> {
         return {as_value<typename T::pointer>(std::move(ptr)), as_value<wasm_size_t>(std::move(len))};
      }

      template <typename T>
      auto from_wasm(const elem_type& ptr, const elem_type& len) const
         -> std::enable_if_t< is_reference_proxy_type_v<T> &&
                              !is_reference_proxy_legacy_v<T> &&
                              is_span_type_v<dependent_type_t<T>>, T> {
         return {as_value<reference_proxy_dependent_type_t<T>*>(std::move(ptr)), as_value<wasm_size_t>(std::move(len))};
      }

      template <typename T>
      auto from_wasm(const elem_type& ptr, const elem_type& len) const
         -> std::enable_if_t< is_reference_proxy_type_v<T> &&
                              is_reference_proxy_legacy_v<T> &&
                              !is_span_type_v<dependent_type_t<T>>, T> {
         return {as_value<reference_proxy_dependent_type_t<T>*>(std::move(ptr))};
      }

      template <typename T>
      auto from_wasm(const elem_type& ptr, const elem_type& len) const
         -> std::enable_if_t< is_reference_proxy_type_v<T> &&
                              !is_reference_proxy_legacy_v<T> &&
                              !is_span_type_v<dependent_type_t<T>>, T> {
         return {as_value<reference_proxy_dependent_type_t<T>*>(std::move(ptr))};
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
         else if constexpr (std::is_pointer_v<T>)
            return base_type::template access_as<std::remove_pointer_t<T>>(val.template get<i32_const_t>().data.ui);
         else if constexpr (std::is_lvalue_reference_v<T>)
            return reference<std::decay_t<T>>(base_type::template access_as<std::decay_t<T>>(val.template get<i32_const_t>().data.ui));
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
         else if constexpr (std::is_pointer_v<T>)
            return i32_const_t{ static_cast<uint32_t>(reinterpret_cast<uintptr_t>(val) -
                                                      reinterpret_cast<uintptr_t>(this->access())) };
         else if constexpr (std::is_lvalue_reference_v<T>)
            return i32_const_t{ static_cast<uint32_t>(reinterpret_cast<uintptr_t>(std::addressof(val)) -
                                                      reinterpret_cast<uintptr_t>(this->access())) };
         else
            return no_match_t{};
      }
   };

   namespace detail {
      template <class TC, typename T>
      using from_wasm_type_deducer_t = flatten_parameters_t<&TC::template from_wasm<T>>;
      template <class TC, typename T>
      using to_wasm_type_deducer_t = return_type_t<&TC::template to_wasm<T>>;

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
      constexpr inline static bool has_to_wasm_v = EOS_VM_HAS_TEMPLATE_MEMBER_TY(Type_Converter, to_wasm<S>);

      template <typename Args, typename S, std::size_t At, class Type_Converter, std::size_t... Is>
      inline constexpr auto create_value(Type_Converter& tc, std::index_sequence<Is...>) {
         constexpr std::size_t offset = total_operands_v<Args, Type_Converter> - 1;
         if constexpr (has_as_value<S, Type_Converter>()) {
            if constexpr (has_from_wasm_v<S, Type_Converter>) {
               return tc.template from_wasm<S>( tc.template as_value<S>(pop_value<offset - At>(tc)) );
            } else {
               return tc.template as_value<S>(pop_value<offset - At>(tc));
            }
         } else {
            static_assert(has_from_wasm_v<S, Type_Converter>, "no type conversion found for type, define a from_wasm for this type");
            return tc.template from_wasm<S>(pop_value<offset - (At + Is)>(tc)...);
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
            using tuple_t  = std::conditional_t<std::is_lvalue_reference_v<source_t>, reference<std::decay_t<source_t>>, source_t>;
            constexpr std::size_t skip_amt = skip_amount<source_t, Type_Converter>();
            return std::tuple_cat(std::tuple<tuple_t>(create_value<Args, source_t, Skip_Amt>(tc, std::make_index_sequence<skip_amt>{})),
                  get_values<Args, At+1, Skip_Amt + skip_amt>(tc));
         }
      }

      template <typename Type_Converter, typename T>
      constexpr auto resolve_result(Type_Converter& tc, T&& val) {
         if constexpr (has_as_result<T, Type_Converter>()) {
            if constexpr (has_to_wasm_v<T, Type_Converter>) {
               return tc.template to_wasm<T>(tc.template as_result<T>(static_cast<T&&>(val)));
            } else {
               return tc.template as_result<T>(static_cast<T&&>(val));
            }
         } else {
            return tc.template to_wasm<T>(static_cast<T&&>(val));
         }
      }

      template <bool Once, std::size_t Cnt, typename T, typename F>
      inline constexpr void invoke_on_impl(F&& fn) {
         if constexpr (Once && Cnt == 0) {
            std::invoke(fn);
         }
      }

      template <bool Once, std::size_t Cnt, typename T, typename F, typename Arg, typename... Args>
      inline constexpr void invoke_on_impl(F&& fn, Arg&& arg, Args&&... args) {
         if constexpr (Once) {
            if constexpr (Cnt == 0)
               std::invoke(fn, std::forward<Arg>(arg), std::forward<Args>(args)...);
         } else {
            if constexpr (std::is_same_v<T, Arg> || std::is_same_v<T, invoke_on_all_t>)
               std::invoke(fn, std::forward<Arg>(arg), std::forward<Args>(args)...);
            invoke_on_impl<Once, Cnt+1, T>(std::forward<F>(fn), std::forward<Args>(args)...);
         }
      }

      template <typename T>
      decltype(auto) convert_type(T&& t) { return static_cast<T&&>(t); }
      template <typename T>
      decltype(auto) convert_type(reference<T> t) { return static_cast<T&>(t); }

      template <typename Precondition, typename Type_Converter, typename Args, std::size_t... Is>
      inline static auto precondition_runner(Type_Converter& ctx, Args&& args, std::index_sequence<Is...>) {
         return Precondition::condition(ctx, std::get<Is>(std::forward<Args>(args))...);
      }

      template <std::size_t I, typename Preconditions, typename Type_Converter, typename Args>
      inline static auto preconditions_runner(Type_Converter& ctx, Args&& args) {
         constexpr std::size_t args_size = std::tuple_size_v<Args>;
         constexpr std::size_t preconds_size = std::tuple_size_v<Preconditions>;
         if constexpr (I < preconds_size)
            return preconditions_runner<I+1, Preconditions>(ctx,
                   precondition_runner<std::tuple_element_t<I, Preconditions>>(ctx,
                      std::forward<Args>(args), std::make_index_sequence<args_size>{}));
         else
            return std::move(args);
      }
   } //ns detail

   template <bool Once, typename T, typename F, typename... Args>
   void invoke_on(F&& func, Args&&... args) {
      detail::invoke_on_impl<Once, 0, T>(static_cast<F&&>(func), std::forward<Args>(args)...);
   }

#define EOS_VM_INVOKE_ON(TYPE, CONDITION) \
   eosio::vm::invoke_on<false, TYPE>(CONDITION, std::forward<Args>(args)...);

#define EOS_VM_INVOKE_ON_ALL(CONDITION) \
   eosio::vm::invoke_on<false, eosio::vm::invoke_on_all_t>(CONDITION, std::forward<Args>(args)...);

#define EOS_VM_INVOKE_ONCE(CONDITION) \
   eosio::vm::invoke_on<true, eosio::vm::invoke_on_all_t>(CONDITION, std::forward<Args>(args)...);

#define EOS_VM_PRECONDITION(NAME, ...)                                       \
   struct NAME {                                                             \
      template <typename Type_Converter, typename... Args>                   \
      inline static decltype(auto) condition(Type_Converter& ctx, Args&&... args) { \
        __VA_ARGS__;                                                         \
        return std::tuple<Args...>(std::move(args)...);                      \
      }                                                                      \
   };

   template <auto F, typename Preconditions, typename Type_Converter, typename Host, typename... Args>
   decltype(auto) invoke_impl(Type_Converter& tc, Host* host, Args&&... args) {
      if constexpr (std::is_same_v<Host, standalone_function_t>)
         return std::invoke(F, detail::convert_type(static_cast<Args&&>(args))...);
      else
         return std::invoke(F, host, detail::convert_type(static_cast<Args&&>(args))...);
   }

   template <auto F, typename Preconditions, typename Host, typename Args, typename Type_Converter, std::size_t... Is>
   decltype(auto) invoke_with_host_impl(Type_Converter& tc, Host* host, Args&& args, std::index_sequence<Is...>) {
      auto&& forwarded_args = detail::preconditions_runner<0, Preconditions>(tc, std::forward<Args>(args));
      return invoke_impl<F, Preconditions>(tc, host, std::get<Is>(static_cast<Args&&>(forwarded_args))...);
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
            maybe_push_result(tc, (invoke_with_host<F, Preconditions, Args>(tc, self, std::index_sequence<Is...>{}), maybe_void), sizeof...(Is));
         }
      };
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
         size_t                                                            current_index = 0;

         template <auto F, typename R, typename Args, typename Preconditions>
         void add_mapping(const std::string& mod, const std::string& name) {
            named_mapping[{mod, name}] = current_index++;
            functions.push_back(
                  create_function<Cls, F, Preconditions, R, Args, Type_Converter>(
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
         for (int i = 0; i < mod.imports.size(); i++) {
            std::string mod_name =
                  std::string((char*)mod.imports[i].module_str.raw(), mod.imports[i].module_str.size());
            std::string fn_name = std::string((char*)mod.imports[i].field_str.raw(), mod.imports[i].field_str.size());
            EOS_VM_ASSERT(current_mappings.named_mapping.count({ mod_name, fn_name }), wasm_link_exception,
                          "no mapping for imported function");
            imports[i] = current_mappings.named_mapping[{ mod_name, fn_name }];
         }
      }

      void operator()(Cls* host, Execution_Interface ei, uint32_t index) {
         const auto& _func = mappings::get().functions[index];
         auto tc = Type_Converter{host, std::move(ei)};
         std::invoke(_func, host, tc);
      }
   };
}} // namespace eosio::vm
