#pragma once

#include <eosio/vm/execution_interface.hpp>
#include <eosio/vm/argument_proxy.hpp>
#include <eosio/vm/span.hpp>
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

#include <boost/hana/ext/std/tuple.hpp>
#include <boost/hana/ext/std/array.hpp>
#include <boost/hana/integral_constant.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/zip_with.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/scan_left.hpp>
#include <boost/hana/drop_back.hpp>
#include <boost/hana/drop_front.hpp>
#include <boost/hana/take_front.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/fuse.hpp>
#include <boost/hana/plus.hpp>
#include <boost/hana/traits.hpp>

namespace eosio { namespace vm {
   // types for host functions to use
   typedef std::nullptr_t standalone_function_t;
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

      template <typename T>
      inline void validate_pointer(const void* ptr, wasm_size_t len) const {
         EOS_VM_ASSERT( len <= std::numeric_limits<wasm_size_t>::max() / (wasm_size_t)sizeof(T), wasm_interpreter_exception, "length will overflow" );
         volatile auto check_addr = *(reinterpret_cast<const char*>(ptr) + (len * sizeof(T)) - 1);
         ignore_unused_variable_warning(check_addr);
      }

      inline void validate_null_terminated_pointer(const void* ptr) const {
         volatile auto check_addr = std::strlen(static_cast<const char*>(ptr));
         ignore_unused_variable_warning(check_addr);
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
      using elem_type = operand_stack_elem;

      // TODO clean this up and figure out a more elegant way to get this for the macro
      EOS_VM_FROM_WASM(bool, (uint32_t value)) { return value ? 1 : 0; }
      uint32_t to_wasm(bool&& value) { return value ? 1 : 0; }

      template<typename T>
      void to_wasm(T&&) = delete;

      template <typename T>
      auto from_wasm(void* ptr, wasm_size_t len, tag<T> = {}) const
         -> std::enable_if_t<is_span_type_v<T>, T> {
         this->template validate_pointer<typename T::value_type>(ptr, len);
         return {static_cast<typename T::pointer>(ptr), len};
      }

      template <typename T>
      auto from_wasm(void* ptr, wasm_size_t len, tag<T> = {}) const
         -> std::enable_if_t< is_argument_proxy_type_v<T> &&
                              is_span_type_v<typename T::proxy_type>, T> {
         this->template validate_pointer<typename T::pointee_type>(ptr, len);
         return {ptr, len};
      }

      template <typename T>
      auto from_wasm(void* ptr, tag<T> = {}) const
         -> std::enable_if_t< is_argument_proxy_type_v<T> &&
                              std::is_pointer_v<typename T::proxy_type>, T> {
         this->template validate_pointer<typename T::pointee_type>(ptr, 1);
         return {ptr};
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
            static_assert(! std::is_same_v<T,T>, "no type conversion found for type, define a from_wasm for this type");
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
            static_assert(! std::is_same_v<T,T>, "no type conversion found for type, define a to_wasm for this type");
      }
   };

   namespace detail {
      template <typename T>
      struct is_tag : std::integral_constant<bool, false> {};

      template <typename T>
      struct is_tag<tag<T>> : std::integral_constant<bool, true> {};

      template <typename T>
      struct callable_traits;

      namespace hana = boost::hana;

      template <typename Cls, typename Ret, typename... Args>
      struct callable_traits<Ret (Cls::*)(Args...)> {
         constexpr static auto args = hana::to<hana::basic_tuple_tag>(hana::tuple_t<std::decay_t<Args>...>);
         constexpr static auto ret  = hana::type_c<Ret>;
      };

      template <typename Cls, typename Ret, typename... Args>
      struct callable_traits<Ret (Cls::*)(Args...) const> {
         constexpr static auto args = hana::to<hana::basic_tuple_tag>(hana::tuple_t<std::decay_t<Args>...>);
         constexpr static auto ret  = hana::type_c<Ret>;
      };

      template <typename Ret, typename... Args>
      struct callable_traits<Ret(Args...)> {
         constexpr static auto args = hana::to<hana::basic_tuple_tag>(hana::tuple_t<std::decay_t<Args>...>);
         constexpr static auto ret  = hana::type_c<Ret>;
      };

      constexpr auto get_arg_types = [](auto fun) { return callable_traits<decltype(fun)>::args; };

      inline auto group_wasm_args = [](auto args, auto sizes) {
         if constexpr (hana::size(args).value > 0) {
            constexpr auto indices = hana::scan_left(hana::drop_back(sizes), hana::size_c<0>, hana::plus);
            return hana::zip_with([&args](auto index, auto sz) { 
                     return hana::take_front(hana::drop_front(args, index), sz); 
                  }, indices, sizes);
         }
         else return args;
      };

      template <typename TC>
      class type_converter_ext {
         TC* converter;
      public:
         type_converter_ext(TC& tc) : converter(&tc) {}

         constexpr static auto has_from_wasm = boost::hana::is_valid([](auto x)->decltype((void)&TC::template from_wasm< typename decltype(x)::type > ){});
         constexpr static auto has_to_wasm = boost::hana::is_valid([](auto x)->decltype((void)std::declval<TC&>().to_wasm(std::declval<typename decltype(x)::type>())){});

         inline auto as_value() {
            return [this](auto type, const auto& elem) {
                  return converter->template as_value<typename decltype(type)::type>(elem);
            };
         }

         inline auto maybe_from_wasm() {
            return [this](auto type, auto wasm_arg_group) {
               using T = typename decltype(type)::type;
               if constexpr (has_from_wasm(type)) 
                  return boost::hana::unpack(wasm_arg_group, [this](auto ...xs) { return converter->template from_wasm<T>(xs...); } );
               else 
                  return boost::hana::front(wasm_arg_group);
            };
         }

         inline auto resolve_result() {
            return [this](auto&& val) {
               if constexpr (has_to_wasm(boost::hana::type_c<std::decay_t<decltype(val)>>)) 
                  return converter->as_result(converter->to_wasm(std::move(val)));
               else
                  return converter->as_result(std::move(val));
            };
         }

         template <typename Preconditions, typename Args>
         inline void check_preconditions(Preconditions preconditions, Args&& args) {
            boost::hana::for_each(preconditions, [this, &args](auto pre) {
               boost::hana::unpack(args, [this](const auto& ...arg) { decltype(pre)::condition(*converter, arg...); });
            }); 
         }
      };

      template <typename TC, typename T>
      inline auto resolve_result(TC& tc, T&& t) {
         return type_converter_ext<TC>(tc).resolve_result()(t);
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

   template <typename F, typename Type_Converter, typename... Precondition>
   struct host_function_traits {
      using base_traits        = detail::callable_traits<F>;
      using converter          = Type_Converter;
      using type_convert_ext_t = detail::type_converter_ext<Type_Converter>;

      constexpr static auto converter_type      = boost::hana::type_c<Type_Converter>;
      ///
      /// <code>
      ///  int32_t get_context_free_data(uint32_t index, legacy_span<char> buffer);
      ///  using traits = host_function_traits<get_context_free_data, type_converter<standalone_function_t>>
      ///
      ///  static_assert( traits::from_wasm_arg_types(hana::type_c<legacy_span<char>>) ==
      ///                  hana::make_basic_tuple(hana::type_t<void*, wasm_size_t>), "" );
      /// </code>
      constexpr static auto from_wasm_arg_types = [](auto t) {
         using T = typename decltype(t)::type;
         return boost::hana::remove_if(detail::get_arg_types(&Type_Converter::template from_wasm<T>),
                                       boost::hana::trait<detail::is_tag>);
      };

      //
      /// <code>
      ///  int32_t get_context_free_data(uint32_t index, legacy_span<char> buffer);
      ///  using traits = host_function_traits<get_context_free_data, type_converter<standalone_function_t>>
      ///
      ///  static_assert( traits::interface_arg_types ==
      ///                  hana::make_basic_tuple(hana::type_t<uint32_t, legacy_span<char>>), "" );
      /// </code>
      constexpr static auto interface_arg_types     = base_traits::args;

      //
      /// <code>
      ///  int32_t get_context_free_data(uint32_t index, legacy_span<char> buffer);
      ///  using traits = host_function_traits<get_context_free_data, type_converter<standalone_function_t>>
      ///
      ///  static_assert( traits::deconstructed_arg_types ==
      ///                  hana::make_basic_tuple( 
      ///                      hana::make_basic_tuple(hana::type_c<uint32_t>), 
      ///                      hana::make_basic_tuple(hana::type_t<void*, wasm_size_t>>)), "" );
      /// </code>
      constexpr static auto deconstructed_arg_types = []() {
         return boost::hana::transform(interface_arg_types, [](auto x) {
            if constexpr (type_convert_ext_t::has_from_wasm(x))
               return from_wasm_arg_types(x);
            else
               return boost::hana::make_basic_tuple(x);
         });
      }();

      //
      /// <code>
      ///  int32_t get_context_free_data(uint32_t index, legacy_span<char> buffer);
      ///  using traits = host_function_traits<get_context_free_data, type_converter<standalone_function_t>>
      ///
      ///  static_assert( traits::wasm_arg_types ==
      ///                 hana::make_basic_tuple(hana::type_t<uint32_t, void*, wasm_size_t>>), "" );
      /// </code>
      constexpr static auto wasm_arg_types = []() {
         namespace hana = boost::hana;
         auto x         = hana::flatten(deconstructed_arg_types);
         return hana::transform(x, [](auto type) {
            using T = decltype(std::declval<type_convert_ext_t&>().as_value()(
                  type, std::declval<typename Type_Converter::elem_type>()));
            if constexpr (std::is_integral_v<T>)
               return hana::type_c<std::make_unsigned_t<T>>;
            else
               return hana::type_c<T>;
         });
      }();

      ///
      /// <code>
      ///  int32_t get_context_free_data(uint32_t index, legacy_span<char> buffer);
      ///  using traits = host_function_traits<get_context_free_data, type_converter<standalone_function_t>>
      ///  using namespace hana::literals;
      ///  static_assert( traits::operand_group_sizes == hana::make_basic_tuple(1_c, 2_c), "" );
      /// <code>
      constexpr static auto operand_group_sizes = boost::hana::transform(deconstructed_arg_types, boost::hana::size);
      //
      /// <code>
      ///  int32_t get_context_free_data(uint32_t index, legacy_span<char> buffer);
      ///  using traits = host_function_traits<get_context_free_data, type_converter<standalone_function_t>>
      ///  using namespace hana::literals;
      ///  static_assert( traits::num_operands == 3, "" );
      /// <code>
      constexpr static auto num_operands  = boost::hana::value(boost::hana::size(wasm_arg_types));

      /// <code>
      ///  int32_t get_context_free_data(uint32_t index, legacy_span<char> buffer);
      ///  using traits = host_function_traits<get_context_free_data, type_converter<standalone_function_t>>
      ///  static_assert( traits::ret_type == hana::type_c<int32_t>, "" );
      /// <code>
      constexpr static auto ret_type      = base_traits::ret;
      constexpr static auto preconditions = boost::hana::basic_tuple<Precondition...>();
   };

   template <typename F>
   inline F bind_host(F f, eosio::vm::standalone_function_t* host) {
      return f;
   }

   template <typename F, typename T>
   inline auto bind_host(F f, T* host) {
      return [f, host](auto&&... arg) { return (host->*f)(std::forward<decltype(arg)>(arg)...); };
   }

   template <typename TC>
   detail::type_converter_ext<TC> make_tc_ext(TC& tc) {
      return detail::type_converter_ext<TC>(tc);
   }

   /// make a lambda which executes f and returns hana::nothing if the return type of f is void;
   /// otherwise it returns hana::just(f(...)).
   constexpr auto make_monadic = [](auto&& f) {
      return [&f](auto&&... xs) {
         if constexpr (std::is_same_v<void, decltype(f(std::forward<decltype(xs)>(xs)...))>)
            return f(std::forward<decltype(xs)>(xs)...), boost::hana::nothing;
         else
            return boost::hana::just(f(std::forward<decltype(xs)>(xs)...));
      };
   };

   inline auto apply_with_wasm_args = [](auto f, auto traits, auto tc_ext, const auto& wasm_args) {
      namespace hana      = boost::hana;
      auto arg_groups     = detail::group_wasm_args(wasm_args, traits.operand_group_sizes);

      // convert each arg  group with from_wasm() if possible
      auto interface_args = hana::zip_with(tc_ext.maybe_from_wasm(), traits.interface_arg_types, arg_groups);
      tc_ext.check_preconditions(traits.preconditions, interface_args);

      // actual invoke f with the interface_args 
      auto r = hana::fuse(make_monadic(f))(std::move(interface_args));
      return hana::transform(r, tc_ext.resolve_result());
   };

   template <std::size_t N, typename TC>
   inline auto pop_operands(TC& tc) {
      auto exec_interface = tc.get_interface();
      auto r = boost::hana::to_tuple(reinterpret_cast<const std::array<typename TC::elem_type, N>&>(exec_interface.operand_from_back(N - 1)));
      exec_interface.trim_operands(N);
      return r;
   }

   template <typename TC>
   inline auto push_operand(TC& tc) {
      return [&tc](auto result) { 
            tc.get_interface().push_operand(result);
            return boost::hana::just(0);
      };
   };

   template <typename Cls, auto F, typename Traits>
   void fn(Cls* self, typename Traits::converter& tc) {
      namespace hana = boost::hana;
      auto tc_ext    = make_tc_ext(tc);
      auto operands  = pop_operands<Traits::num_operands>(tc);
      auto wasm_args = hana::zip_with(tc_ext.as_value(), Traits::wasm_arg_types, operands);
      apply_with_wasm_args(bind_host(F, self), Traits{}, tc_ext, wasm_args) | push_operand(tc);
   }

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

   template <typename T>
   struct to_wasm_type_code;

   template <>
   struct to_wasm_type_code<i32_const_t> :  std::integral_constant<value_type, types::i32>{};
   template <>
   struct to_wasm_type_code<i64_const_t> :  std::integral_constant<value_type, types::i64>{};
   template <>
   struct to_wasm_type_code<f32_const_t> :  std::integral_constant<value_type, types::f32>{};
   template <>
   struct to_wasm_type_code<f64_const_t> :  std::integral_constant<value_type, types::f64>{};
   template <>
   struct to_wasm_type_code<void> :  std::integral_constant<value_type, types::ret_void>{};

   template <typename TC, typename T>
   struct to_wasm_type : to_wasm_type_code<decltype( detail::resolve_result(std::declval<TC&>(), std::declval<T>()) )>{};
 
   template <typename TC, typename T>
   inline constexpr auto to_wasm_type_v = to_wasm_type<TC, T>::value;

   inline auto wasm_arg_codes = [](auto tc_type, auto arg_types) -> std::vector<value_type> {
       return boost::hana::unpack(arg_types, [tc_type](auto... type) { return std::vector<value_type>{ boost::hana::trait<to_wasm_type>(tc_type, type)... }; }); 
   };

   inline auto wasm_ret_codes = [](auto tc_type, auto ret_type) -> std::vector<value_type> {
      if constexpr ( boost::hana::traits::is_void(ret_type) ) 
         return {};
      else
         return std::vector<value_type>{ boost::hana::trait<to_wasm_type>(tc_type, ret_type) };
   };

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
         typedef void (*function_t)(Cls*, Type_Converter&);
         std::unordered_map<host_func_pair, uint32_t, host_func_pair_hash> named_mapping;
         std::vector<function_t>                          functions;
         std::vector<host_function>                                        host_functions;
         size_t                                                            current_index = 0;

         template <auto F, typename Traits>
         void add_mapping(const std::string& mod, const std::string& name) {
            named_mapping[{ mod, name }] = current_index++;
            functions.push_back(&fn<Cls, F, Traits>);
            host_functions.push_back(host_function{wasm_arg_codes(Traits::converter_type, Traits::wasm_arg_types), 
                                                   wasm_ret_codes(Traits::converter_type, Traits::ret_type)});
         }

         static mappings& get() {
            static mappings instance;
            return instance;
         }
      };

      template <auto Func, typename... Preconditions>
      static void add(const std::string& mod, const std::string& name) {
         using traits = host_function_traits<decltype(Func), Type_Converter, Preconditions...>;
         mappings::get().template add_mapping<Func, traits>(mod, name);
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
            const import_entry& entry = mod.imports[i];
            EOS_VM_ASSERT(entry.kind == Function, wasm_link_exception, "importing non-function");
            EOS_VM_ASSERT(current_mappings.host_functions[imports[i]] == mod.types[entry.type.func_t],
                          wasm_link_exception, "wrong type for imported function");
         }
      }

      void operator()(Cls* host, Execution_Interface ei, uint32_t index) {
         const auto& _func = mappings::get().functions[index];
         auto tc = Type_Converter{host, std::move(ei)};
         std::invoke(_func, host, tc);
      }
   };
}} // namespace eosio::vm
