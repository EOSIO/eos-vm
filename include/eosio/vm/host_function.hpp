#pragma once

#include <eosio/vm/allocator.hpp>
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
   typedef std::uint32_t wasm_ptr_t;
   typedef std::uint32_t wasm_size_t;
   typedef nullptr_t     standalone_function_t;
   struct no_match_t {};

   template <typename Execution_Interface=execution_interface>
   struct running_context {
      using running_context_t = running_context<Execution_Interface>;
      inline explicit running_context(const Execution_Interface& ei) : interface(ei) {}
      inline explicit running_context(Execution_Interface&& ei) : interface(ei) {}

      inline void* access(wasm_ptr_t addr=0) const { return (char*)interface.get_memory() + addr; }

      template <typename T=char>
      inline T* access_as(wasm_ptr_t addr) const { return reinterpret_cast<T*>(access(addr)); }

      inline Execution_Interface& get_interface() { return interface; }

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
      Execution_Interface interface;
   };

   template <typename T>
   struct dependent_type;

   template <template<typename> class T, typename U>
   struct dependent_type<T<U>> {
      using type = U;
   };

   template <typename T>
   using dependent_type_t = typename dependent_type<T>::type;

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

#define EOS_VM_TYPE_CONVERTER_SETUP \
   using base_type = running_context<Execution_Interface>; \
   using base_type::running_context;                       \
   using elem_type = decltype(std::declval<type_converter>().get_interface().operand_from_back(0));

   template <typename Execution_Interface=execution_interface>
   struct type_converter : public running_context<Execution_Interface> {
      using base_type = running_context<Execution_Interface>;
      using base_type::running_context;

      // TODO clean this up and figure out a more elegant way to get this for the macro
      using elem_type = decltype(std::declval<type_converter>().get_interface().operand_from_back(0));

      EOS_VM_FROM_WASM(bool, (elem_type&& value)) { return as_value<uint32_t>(value) ? 1 : 0; }
      EOS_VM_TO_WASM(bool, (bool value)) { return as_result<uint32_t>(value ? 1 : 0); }

      EOS_VM_FROM_WASM(T, span<T>, (elem_type&& ptr, elem_type&& len)) { return {as_value<T*>(std::move(ptr)), as_value<wasm_size_t>(std::move(len))}; }

      EOS_VM_FROM_WASM(T, reference_proxy<span<T>>, (elem_type&& ptr, elem_type&& len)) { return {as_value<T*>(std::move(ptr)), as_value<wasm_size_t>(std::move(len))}; }
      EOS_VM_FROM_WASM(T, reference_proxy<T, true>, (elem_type&& ptr)) { return {as_value<T*>(std::move(ptr))}; }
      EOS_VM_FROM_WASM(T, reference_proxy<T>, (elem_type&& ptr)) { return {as_value<T*>(std::move(ptr))}; }

      // passthrough
      template <typename T>
      T from_wasm(T&& val) const { return val; }

      template<typename T>
      inline auto as_value(elem_type&& val) const {
         if constexpr (std::is_integral_v<T> && sizeof(T) == 4)
            return static_cast<const T&&>(val.template get<i32_const_t>().data.ui);
         else if constexpr (std::is_integral_v<T> && sizeof(T) == 8)
            return static_cast<const T&&>(val.template get<i64_const_t>().data.ui);
         else if constexpr (std::is_floating_point_v<T> && sizeof(T) == 4)
            return static_cast<const T&&>(val.template get<f32_const_t>().data.f);
         else if constexpr (std::is_floating_point_v<T> && sizeof(T) == 8)
            return static_cast<const T&&>(val.template get<f64_const_t>().data.f);
         else if constexpr (std::is_pointer_v<T>)
            return reinterpret_cast<T>(this->access(val.template get<i32_const_t>().data.ui));
         else if constexpr (std::is_lvalue_reference_v<T>)
            return static_cast<T>(*reinterpret_cast<std::remove_reference_t<T>*>(this->access(val.template get<i32_const_t>().data.ui)));
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
      template <typename T, typename Cls, typename... Args>
      auto from_wasm_overload(T(Cls::*)(Args...)) -> std::tuple<Args...>;
      template <typename T, typename Cls, typename... Args>
      auto from_wasm_overload(T(Cls::*)(Args...)const) ->std::tuple<Args...>;

      template <typename R, typename T, typename Cls>
      auto to_wasm_overload(R(Cls::*)(T)) -> R;
      template <typename R, typename T, typename Cls>
      auto to_wasm_overload(R(Cls::*)(T)const) -> R;

      template <class TC, typename T>
      using from_wasm_type_deducer_t = flatten_parameters_t<&TC::template from_wasm<T>>;
      template <class TC, typename T>
      using to_wasm_type_deducer_t = return_type_t<&TC::template to_wasm<T>>;

      template <std::size_t N, typename Type_Converter>
      inline constexpr auto& pop_value(Type_Converter& tc) { return tc.get_interface().operand_from_back(N); }

      template <std::size_t N>
      struct value_encoded_type { static constexpr std::size_t value = N; };

      template <typename T, class Type_Converter>
      inline constexpr std::size_t value_operand_size() {
         if constexpr (!std::is_same_v<no_match_t,
               std::decay_t<decltype(std::declval<Type_Converter>().template as_value<T>(std::declval<decltype(pop_value<0>(std::declval<Type_Converter&>()))>()))>>)
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

      template <typename Args, typename S, std::size_t At, class Type_Converter, std::size_t... Is>
      inline constexpr decltype(auto) create_value(Type_Converter& tc, std::index_sequence<Is...>) {
         constexpr std::size_t offset = total_operands_v<Args, Type_Converter> - 1;
         if constexpr (!std::is_same_v<no_match_t, std::decay_t<decltype(tc.template as_value<S>(pop_value<offset - At>(tc)))>>)
            return tc.template from_wasm<S>( tc.template as_value<S>(pop_value<offset - At>(tc)) );
         else {
            return tc.template from_wasm<S>(pop_value<offset - (At + Is)>(tc)...);
         }
      }

      template <typename S, typename Type_Converter>
      inline constexpr std::size_t skip_amount(Type_Converter& tc) {
         if constexpr (!std::is_same_v<no_match_t, std::decay_t<decltype(tc.template as_value<S>(pop_value<0>(tc)))>>)
            return 1;
         else
            return std::tuple_size_v<from_wasm_type_deducer_t<Type_Converter, S>>;
      }

      template <typename Args, std::size_t At, class Type_Converter>
      inline constexpr decltype(auto) get_values(Type_Converter& tc) {
         if constexpr (At >= std::tuple_size_v<Args>)
            return std::tuple<>{};
         else {
            using source_t      = std::tuple_element_t<At, Args>;
            constexpr size_t skip_amt = skip_amount<source_t>(tc);
            return std::tuple_cat(std::tuple<source_t>{create_value<Args, source_t, At>(tc, std::make_index_sequence<skip_amt>{})}, get_values<Args, At + skip_amt>(tc));
         }
      }

      template <typename Type_Converter, typename T>
      constexpr auto resolve_result(Type_Converter& tc, T&& val) {
         if constexpr (!std::is_same_v<decltype(tc.template as_result(val)), no_match_t>)
            return tc.as_result(static_cast<T&&>(val));
         else
            return tc.to_wasm(static_cast<T&&>(val));
      }

      template <typename F>
      inline constexpr void invoke_on_impl(F&& fn) {}

      template <typename F, typename Arg, typename... Args>
      inline constexpr void invoke_on_impl(F&& fn, Arg&& arg, Args&&... args) {
          // TODO need to fix this back up, somehow this has stopped working
          //if constexpr (std::is_same_v<decltype(parameter_at<0>(fn)), Arg>)
          //   std::invoke(fn, std::forward<Arg>(arg));
          std::invoke(fn, std::forward<Arg>(arg), std::forward<Args>(args)...);
          invoke_on_impl(static_cast<F&&>(fn), std::forward<Args>(args)...);
      }

      template <typename Precondition, typename Type_Converter, typename Args, std::size_t... Is>
      inline static auto precondition_runner(Type_Converter& ctx, Args&& args, std::index_sequence<Is...>) {
         return Precondition::condition(ctx, std::get<Is>(args)...);
      }

      template <std::size_t I, typename Preconditions, typename Type_Converter, typename... Args>
      inline static auto preconditions_runner(Type_Converter& ctx, std::tuple<Args...>&& tup) {
         if constexpr (I < std::tuple_size_v<Preconditions>)
            return preconditions_runner<I+1, Preconditions>(ctx,
                  precondition_runner<std::tuple_element_t<I, Preconditions>>(ctx, std::move(tup), std::make_index_sequence<sizeof...(Args)>{}));
         else
            return static_cast<std::tuple<Args...>&&>(tup);
      }
   } //ns detail

   template <typename... Args, typename F>
   void invoke_on(F&& func, Args&&... args) {
      detail::invoke_on_impl(static_cast<F&&>(func), std::forward<Args>(args)...);
   }

#define EOS_VM_INVOKE_ON(CONDITION) \
    invoke_on(CONDITION, std::forward<Args>(args)...);

#define EOS_VM_PRECONDITION(NAME, ...)                                       \
   struct NAME {                                                             \
      template <typename Type_Converter, typename... Args>                   \
      inline static decltype(auto) condition(Type_Converter& ctx, Args&&... args) { \
        __VA_ARGS__;                                                         \
        return std::forward_as_tuple(args...);                               \
      }                                                                      \
   };

   template <auto F, typename Preconditions, typename Args, typename Type_Converter>
   decltype(auto) invoke_with_preconditions(Type_Converter& tc, Args&& args) {
      return detail::preconditions_runner<0, Preconditions>(tc, std::move(args));
   }

   template <auto F, typename Preconditions, typename Host, typename Args, typename Type_Converter, std::size_t... Is>
   decltype(auto) invoke_with_host_impl(Type_Converter& tc, Host* host, Args&& args, std::index_sequence<Is...>) {
      if constexpr (std::is_same_v<Host, standalone_function_t>)
         return std::invoke(F, std::get<Is>(invoke_with_preconditions<F, Preconditions>(tc, args))...);
      else
         return std::invoke(F, host, std::get<Is>(invoke_with_preconditions<F, Preconditions>(tc, args))...);
   }

   template <auto F, typename Preconditions, typename Args, typename Type_Converter, typename Host, std::size_t... Is>
   decltype(auto) invoke_with_host(Type_Converter& tc, Host* host, std::index_sequence<Is...>) {
      constexpr std::size_t args_size = std::tuple_size_v<decltype(detail::get_values<Args, 0>(tc))>;
      return invoke_with_host_impl<F, Preconditions>(tc, host, detail::get_values<Args, 0>(tc), std::make_index_sequence<args_size>{});
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

   template <typename Cls, typename Execution_Interface=execution_interface, typename Type_Converter=type_converter<Execution_Interface>>
   struct registered_host_functions {
      using host_type_t           = Cls;
      using execution_interface_t = Execution_Interface;
      using type_converter_t      = Type_Converter;

      struct mappings {
         std::unordered_map<host_func_pair, uint32_t,                            host_func_pair_hash> named_mapping;
         std::vector<std::function<void(Cls*, Type_Converter&)>>                 functions;
         size_t                                                                  current_index = 0;
      };

      static mappings& get_mappings() {
         static mappings _mappings;
         return _mappings;
      }

      template <auto Func, typename... Preconditions>
      static void add(const std::string& mod, const std::string& name) {
         using deduced_full_ts                         = flatten_parameters_t<Func>;
         using res_t                                   = return_type_t<Func>;
         using preconditions                           = std::tuple<Preconditions...>;
         static constexpr auto is                      = std::make_index_sequence<std::tuple_size_v<deduced_full_ts>>();
         auto& current_mappings                        = get_mappings();
         current_mappings.named_mapping[{ mod, name }] = current_mappings.current_index++;
         current_mappings.functions.push_back(create_function<Cls, Func, preconditions, res_t, deduced_full_ts, Type_Converter>(is));
      }

      template <typename Module>
      static void resolve(Module& mod) {
         auto& imports          = mod.import_functions;
         auto& current_mappings = get_mappings();
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
         const auto& _func = get_mappings().functions[index];
         auto tc = Type_Converter{std::move(ei)};
         std::invoke(_func, host, tc);
      }
   };

   template <typename Cls, typename Cls2, auto F>
   struct registered_function {
      registered_function(std::string mod, std::string name) {
         registered_host_functions<Cls>::template add<Cls2, F>(mod, name);
      }
   };

#undef AUTO_PARAM_WORKAROUND

}} // namespace eosio::vm
