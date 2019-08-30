#pragma once

#include <eosio/vm/wasm_stack.hpp>
#include <eosio/vm/utils.hpp>

#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <cxxabi.h>
#include <memory>
#include <string>

// forward declaration of array_ptr
namespace eosio { namespace chain {
   template <typename T>
   struct array_ptr;
}}

namespace eosio { namespace vm {

   template <typename Derived, typename Base>
   struct construct_derived {
      static auto value(Base& base) { return Derived(base); }
      typedef Base type;
   };

   template <typename Derived>
   struct construct_derived<Derived, nullptr_t> {
      static nullptr_t value(nullptr_t) { return nullptr; }
   };
   
   // Workaround for compiler bug handling C++g17 auto template parameters.
   // The parameter is not treated as being type-dependent in all contexts,
   // causing early evaluation of the containing expression.
   // Tested at Apple LLVM version 10.0.1 (clang-1001.0.46.4)
   template<class T, class U>
   inline constexpr U&& make_dependent(U&& u) { return static_cast<U&&>(u); }
#define AUTO_PARAM_WORKAROUND(X) make_dependent<decltype(X)>(X)

   template <typename... Args, size_t... Is>
   auto get_args_full(std::index_sequence<Is...>) {
      std::tuple<std::decay_t<Args>...> tup;
      return std::tuple<Args...>{ std::get<Is>(tup)... };
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
   auto get_args_full(R (Cls::*)(Args...) const) {
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
   auto get_return_t(R (Cls::*)(Args...) const) {
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
   auto get_args(R (Cls::*)(Args...) const) {
      return std::tuple<std::decay_t<Args>...>{};
   }

   template <typename T>
   struct aligned_ptr_wrapper {
      constexpr aligned_ptr_wrapper(T* ptr) : ptr(ptr) {
         if (ptr % alignof(T) != 0) {
            copy = T{};
            memcpy( (void*)&(*copy), (void*)ptr, sizeof(T) );
         }
      }
      ~aligned_ptr_wrapper() {
         if constexpr (!std::is_const_v<T>)
            if (copy)
               memcpy( (void*)ptr, (void*)&(*copy), sizeof(T) );
      }
      constexpr std::add_lvalue_reference_t<T> operator*() const { 
         if (copy)
            return *copy;
         else
            return *ptr;
      }
      constexpr T* operator->() const noexcept {
         if (copy)
            return &(*copy);
         else
            return ptr;
      }
      constexpr operator T*() const {
         if (copy)
            return &(*copy);
         else
            return ptr;
      }

      T* ptr;
      std::optional<T> copy;
   };

   // This class can be specialized to define a conversion to/from wasm.
   // generic pass through
   template <typename T>
   struct wasm_type_converter {
      static T&& from_wasm(T&& t) { return std::forward<T>(t); }
      static T&& to_wasm(T&& t) { return std::forward<T>(t); }
   };

   namespace detail {
      template<typename T, typename U, typename... Args>
      auto from_wasm_type_impl(T (*)(U, Args...)) -> U;
      template<typename T>
      using from_wasm_type_impl_t = decltype(detail::from_wasm_type_impl(&wasm_type_converter<T>::from_wasm));

      template<typename T, typename U, typename... Args>
      auto to_wasm_type_impl(T (*)(U, Args...)) -> T;
      template<typename T>
      using to_wasm_type_impl_t = decltype(detail::to_wasm_type_impl(&wasm_type_converter<T>::to_wasm));

      // Extract the wasm type from wasm_type_converter and verify
      // that if both from_wasm and to_wasm are defined, they use
      // the same type.
      template<typename T, typename HasFromWasm = void, typename HasToWasm = void>
      struct get_wasm_type;
      template<typename T, typename HasToWasm>
      struct get_wasm_type<T, std::void_t<from_wasm_type_impl_t<T>>, HasToWasm> {
         using type = from_wasm_type_impl_t<T>;
      };
      template<typename T, typename HasFromWasm>
      struct get_wasm_type<T, HasFromWasm, std::void_t<from_wasm_type_impl_t<T>>> {
         using type = to_wasm_type_impl_t<T>;
      };
      template<typename T>
      struct get_wasm_type<T, std::void_t<from_wasm_type_impl_t<T>>, std::void_t<to_wasm_type_impl_t<T>>> {
         static_assert(std::is_same_v<from_wasm_type_impl_t<T>, to_wasm_type_impl_t<T>>,
                       "wasm_type_converter must use the same type for both from_wasm and to_wasm.");
         using type = from_wasm_type_impl_t<T>;
      };
      
      // try the pointer to force segfault early
      template <typename WAlloc, typename T>
      constexpr T* validate_ptr( WAlloc&& alloc, uint32_t offset, uint32_t len ) {
         EOS_VM_ASSERT( len < INT_MAX / (uint32_t)sizeof(T), wasm_interpreter_exception, "length will overflow" );
         T* ret_ptr = reinterpret_cast<T>(alloc.template get_base_ptr<char>() + offset);
         // check the pointer
         volatile auto ret_val = *(ret_ptr + (len ? len-1 : 0));
         return ret_ptr;
      }

      // Allow from_wasm to return a wrapper that holds extra data.
      // StorageType must be implicitly convertible to T
      template<typename T, typename StorageType>
      struct cons_item {
         template<typename F>
         explicit cons_item(F&& f) : _value(f()) {}
         StorageType _value;
         T get() const { return value; }
      };

      template<typename Car, typename Cdr>
      struct cons {
         // Make sure that we get mandatory RVO, so that we can guarantee
         // that user provided destructors run exactly once in the right sequence.
         template<typename F, typename FTail>
         cons(F&& f, FTail&& ftail) : cdr(ftail()), car(f(cdr)) {}
         static constexpr size = Cdr::size + 1;
         // Reverse order to get the order of construction and destruction right
         // We want to construct in reverse order and destroy in forwards order.
         Cdr cdr;
         Car car;
         // We need mandatory RVO.  deleting the copy constructor should
         // cause an error if we messed that up.
         cons(const cons&) = delete;
         cons& operator=(const cons&) = delete;
      };

      template<>
      struct cons<void, void> { static constexpr std::size_t size = 0; };
      using nil_t = cons<void, void>;

      template<std::size_t I, typename Car, typename Cdr>
      decltype(auto) cons_get(const cons<Car, Cdr>& l) {
         if constexpr (I == 0) {
            return l.car.get();
         } else {
            return detail::cons_get<I-1>(l.cdr);
         }
      }

      // get the type of the Ith element of the cons list.
      template<std::size_t I, typename Cons>
      using cons_item_t = decltype(detail::cons_get<I>(std::declval<Cons>()));

      // Calls from_wasm with the correct arguments
      template<typename A, typename SourceType, typename Tail, typename T, std::size_t... Is>
      auto get_value_impl(std::index_sequence<Is...>, WAlloc* alloc, T&& val, const Tail& tail) {
         return wasm_type_converter<A>::from_wasm(get_value<SourceType>(alloc, val, tail), cons_get<Is>(tail)...);
      }

      // Matches a specific overload of a function and deduces the first argument
      template<typename... Rest>
      struct match_from_wasm {
         template<typename R, typename U>
         static U apply(R(U, Rest...));
      };

      template<std::size_t LookaheadCount, typename SourceType>
      struct value_getter {
         template<typename A, typename WAlloc, typename Tail>
         auto apply(WAlloc* alloc, T&& val, const Tail& tail) {
            return get_value_impl<A, SourceType>(std::make_index_sequence<LookaheadCount>{}, alloc, static_cast<T&&>(val), tail);
         }
      };

      // Detect whether there is a match of from_wasm with these arguments.
      // returns a value_getter or void if it didn't match.
      template<typename T, typename Cons, std::size_t... Is>
      auto try_value_getter(std::index_sequence<Is...>)
         -> value_getter<sizeof...(Is), decltype(match_from_wasm<cons_item_t<Is, Cons>...>::apply(&wasm_type_converter<T>::from_wasm))>;
      // Fallback
      auto try_value_getter(...) -> void;

      template<std::size_t N, typename T, typename Cons>
      using try_value_getter_t = decltype(try_value_getter<T, Cons>(std::make_index_sequence<N>{}));

      // Error type to hopefully make a somewhat understandable error message when
      // the user did not provide a correct overload of from_wasm.
      template<typename T, typename... Tail>
      struct no_viable_overload_of_from_wasm {};

      // Searches for a match of from_wasm following the principal of maximum munch.
      template<std::size_t N, typename T, typename Tail>
      constexpr auto make_value_getter_impl() {
         if constexpr(std::is_same_v<try_value_getter_t<N, T, Tail>, void>) {
            if constexpr (N == 0) {
               return no_viable_overload_of_from_wasm<T>{};
            } else {
               return make_value_getter_impl<N-1, T, Tail>(arg);
            }
         } else {
            return try_value_getter_t<N, T, Tail>{};
         }
      }

      template<typename T, typename Tail>
      constexpr auto make_value_getter() {
         return make_value_getter_impl<Tail::size, T, Tail>();
      }

      // args should be a tuple
      // Constructs a cons of the translated arguments given a list of
      // argument types, a wasm stack, and a wasm allocator.
      template<typename Args>
      struct pack_args;

      template<typename T, typename F>
      auto make_cons_item(F&& f) {
         return [=](auto& arg) { return cons_item<T, decltype(f(arg))>{[&]() { f(arg); }}; }
      }

      template<typename T0, typename... T>
      struct pack_args<std::tuple<T0, T...>> {
         using next = std::tuple<T...>;
         template<typename WAlloc, typename Os>
         static auto apply(WAlloc* alloc, Os& os) {
            using next_result = decltype(next::apply(alloc, os));
            using result_type = cons<cons_item<T0, decltype(get_value<T0>(alloc, os.get_back(sizeof...(T)))>, std::declval<next_result&>()), next_result>;
            return result_type(make_cons_item([&](auto& tail) {return get_value<T0>(alloc, os.get_back(sizeof...(T))), tail);}),
              [&](){ return next::apply(alloc, os); } );
         }
      };

      template<>
      struct pack_args<std::tuple<>> {
         template<typename WAlloc, typename Os>
         static auto apply(WAlloc* alloc, Os& os) { return nil_t{}; }
      };

      template<typename S, typename T, typename WAlloc, typename Cons>
      constexpr auto get_value(WAlloc* alloc, T&& val, Cons& tail) -> S {
         if constexpr (std::is_integral_v<S> && sizeof(S) == 4)
            return val.template get<i32_const_t>().data.ui;
         else if constexpr (std::is_integral_v<S> && sizeof(S) == 8)
            return val.template get<i64_const_t>().data.ui;
         else if constexpr (std::is_floating_point_v<S> && sizeof(S) == 4)
            return val.template get<f32_const_t>().data.f;
         else if constexpr (std::is_floating_point_v<S> && sizeof(S) == 8)
            return val.template get<f64_const_t>().data.f;
         else if constexpr (std::is_pointer_v<S>)
            return reinterpret_cast<S>(alloc->template get_base_ptr<char>() + val.template get<i32_const_t>().data.ui);
         else {
            return detail::make_value_getter<S, Cons>().template apply<S>(alloc, static_cast<T&&>(val), tail);
         }
      }

      template <typename T, typename WAlloc>
      constexpr auto resolve_result(T&& res, WAlloc* alloc) {
         if constexpr (std::is_integral_v<T> && sizeof(T) == 4)
            return i32_const_t{ static_cast<uint32_t>(res) };
         else if constexpr (std::is_integral_v<T> && sizeof(T) == 8)
            return i64_const_t{ static_cast<uint64_t>(res) };
         else if constexpr (std::is_floating_point_v<T> && sizeof(T) == 4)
            return f32_const_t{ static_cast<float>(res) };
         else if constexpr (std::is_floating_point_v<T> && sizeof(T) == 8)
            return f64_const_t{ static_cast<double>(res) };
         else if constexpr (std::is_pointer_v<T>)
            return i32_const_t{ static_cast<uint32_t>(reinterpret_cast<uintptr_t>(res) -
                                                      reinterpret_cast<uintptr_t>(alloc->template get_base_ptr<char>())) };
         else
            return detail::resolve_result(wasm_type_converter<T>::to_wasm(static_cast<T&&>(res)), alloc);
      }
   } //ns detail

   template <>
   struct wasm_type_converter<bool> {
      static bool from_wasm(uint32_t val) { return val != 0; }
      static uint32_t to_wasm(bool val) { return val? 1 : 0; }
   };

   template <typename T>
   struct wasm_type_converter<T&> {
      static T& from_wasm(T* ptr) { return *ptr; }
      static T* to_wasm(T& ref) { return std::addressof(ref); }
   };
  

   template <typename T>
   inline constexpr auto to_wasm_type() -> uint8_t {
      if constexpr (std::is_same_v<T, void>)
         return types::ret_void;
      else if constexpr (std::is_same_v<T, bool>)
         return types::i32;
      else if constexpr (std::is_integral_v<T> && sizeof(T) == 4)
         return types::i32;
      else if constexpr (std::is_integral_v<T> && sizeof(T) == 8)
         return types::i64;
      else if constexpr (std::is_floating_point_v<T> && sizeof(T) == 4)
         return types::f32;
      else if constexpr (std::is_floating_point_v<T> && sizeof(T) == 8)
         return types::f64;
      else if constexpr (std::is_pointer_v<T> || std::is_reference_v<T>)
         return types::i32;
      else
         return vm::to_wasm_type<typename detail::get_wasm_type<T>::type>();
   }

   template <uint8_t Type>
   struct _to_wasm_t;

   template <>
   struct _to_wasm_t<types::i32> {
      typedef i32_const_t type;
   };

   template <>
   struct _to_wasm_t<types::i64> {
      typedef i64_const_t type;
   };

   template <>
   struct _to_wasm_t<types::f32> {
      typedef f32_const_t type;
   };

   template <>
   struct _to_wasm_t<types::f64> {
      typedef f64_const_t type;
   };

   template <typename T>
   using to_wasm_t = typename _to_wasm_t<to_wasm_type<T>()>::type;

   template<auto F, typename Derived, typename... T>
   auto invoke_with_host(Host* host, T&&... args) {
      constexpr size_t args_end = sizeof...(Is)-1;
      if constexpr (std::is_same_v<Derived, nullptr_t>)
         return std::invoke(F, static_cast<T&&>(args)...);
      else
         return std::invoke(F, construct_derived<Derived, Host>::value(*host), static_cast<T&&>(args)...);
   }

   template<auto F, typename Derived, typename Cons>
   auto invoke_with_cons(Host* host, Cons&& args, std::index_sequence<Is...>) {
      return invoke_with_host<F, Derived>(host, detail::cons_get<Is>(args)...);
   }

   template<typename F, typename Os>
   auto wrap_invoke(F&& f, Os& os, std::size_t trim_amt) {
      if constexpr std::is_same_v<decltype(f()), void> {
         auto res = f();
         os.trim(trim_amt);
         os.push(res);
      } else {
         f();
         os.trim(trim_amt);
      }
   }

   template <typename Walloc, typename Cls, typename Cls2, auto F, typename R, typename Args, size_t... Is>
   auto create_function(std::index_sequence<Is...>) {
      return std::function<void(Cls*, Walloc*, operand_stack&)>{ [](Cls* self, Walloc* walloc, operand_stack& os) {
         wrap_invoke([]() {
               return invoke_with_cons<F, Cls2>(self, pack_args<Args>::apply(walloc, os), std::make_index_sequence<Is...>);
            },
            os,
            sizeof...(Is));
      } };
   }

   template <typename T>
   constexpr auto to_wasm_type_v = to_wasm_type<T>();

   struct host_function {
      void*                   ptr;
      std::vector<value_type> params;
      std::vector<value_type> ret;
   };

   template <typename Ret, typename... Args>
   host_function function_types_provider() {
      host_function hf;
      hf.ptr    = (void*)func;
      hf.params = { to_wasm_type_v<Args>... };
      if constexpr (to_wasm_type_v<Ret> != types::ret_void) {
         hf.ret = { to_wasm_type_v<Ret> };
      }
      return hf;
   }

   template <typename Ret, typename... Args>
   host_function function_types_provider(Ret (*func)(Args...)) {
      return function_types_provider<Ret, Args...>();
   }

   template <typename Ret, typename Cls, typename... Args>
   host_function function_types_provider(Ret (*func)(Args...)) {
      return function_types_provider<Ret, Args...>();
   }

   template <char... Str>
   struct host_function_name {
      static constexpr const char value[] = { Str... };
      static constexpr size_t     len     = sizeof...(Str);
      static constexpr bool       is_same(const char* nm, size_t l) {
         if (len == l) {
            bool is_not_same = false;
            for (int i = 0; i < len; i++) { is_not_same |= nm[i] != value[i]; }
            return !is_not_same;
         }
         return false;
      }
   };

#if defined __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
#endif
   template <typename T, T... Str>
   static constexpr host_function_name<Str...> operator""_hfn() {
      constexpr auto hfn = host_function_name<Str...>{};
      return hfn;
   }
#if defined __clang__
#   pragma clang diagnostic pop
#endif

   template <typename C, auto C::*MP, typename Name>
   struct registered_member_function {
      static constexpr auto function  = MP;
      static constexpr auto name      = Name{};
      using name_t                    = Name;
      static constexpr bool is_member = true;
   };

   using host_func_pair = std::pair<std::string, std::string>;
   struct host_func_pair_hash {
      template <class T, class U>
      std::size_t operator()(const std::pair<T, U>& p) const {
         return std::hash<T>()(p.first) ^ std::hash<U>()(p.second);
      }
   };

   template <typename Cls>
   struct registered_host_functions {
      template <typename WAlloc>
      struct mappings {
         std::unordered_map<std::pair<std::string, std::string>, uint32_t, host_func_pair_hash> named_mapping;
         std::vector<host_function>                                                             host_functions;
         std::vector<std::function<void(Cls*, WAlloc*, operand_stack&)>>                        functions;
         size_t                                                                                 current_index = 0;
      };

      template <typename WAlloc>
      static mappings<WAlloc>& get_mappings() {
         static mappings<WAlloc> _mappings;
         return _mappings;
      }

      template <typename Cls2, auto Func, typename WAlloc>
      static void add(const std::string& mod, const std::string& name) {
         using deduced_full_ts                         = decltype(get_args_full(AUTO_PARAM_WORKAROUND(Func)));
         using deduced_ts                              = decltype(get_args(AUTO_PARAM_WORKAROUND(Func)));
         using res_t                                   = typename decltype(get_return_t(AUTO_PARAM_WORKAROUND(Func)))::type;
         static constexpr auto is                      = std::make_index_sequence<std::tuple_size_v<deduced_ts>>();
         auto&                 current_mappings        = get_mappings<WAlloc>();
         current_mappings.named_mapping[{ mod, name }] = current_mappings.current_index++;
         current_mappings.functions.push_back(create_function<WAlloc, Cls, Cls2, Func, res_t, deduced_full_ts>(is));
      }

      template <typename Module>
      static void resolve(Module& mod) {
         decltype(mod.import_functions) imports          = { mod.allocator, mod.get_imported_functions_size() };
         auto&                          current_mappings = get_mappings<wasm_allocator>();
         for (int i = 0; i < mod.imports.size(); i++) {
            std::string mod_name =
                  std::string((char*)mod.imports[i].module_str.raw(), mod.imports[i].module_str.size());
            std::string fn_name = std::string((char*)mod.imports[i].field_str.raw(), mod.imports[i].field_str.size());
            EOS_VM_ASSERT(current_mappings.named_mapping.count({ mod_name, fn_name }), wasm_link_exception,
                          "no mapping for imported function");
            imports[i] = current_mappings.named_mapping[{ mod_name, fn_name }];
         }
         mod.import_functions = std::move(imports);
      }

      template <typename Execution_Context>
      void operator()(Cls* host, Execution_Context& ctx, uint32_t index) {
         const auto& _func = get_mappings<wasm_allocator>().functions[index];
         std::invoke(_func, host, ctx.get_wasm_allocator(), ctx.get_operand_stack());
      }
   };

   template <typename Cls, typename Cls2, auto F>
   struct registered_function {
      registered_function(std::string mod, std::string name) {
         registered_host_functions<Cls>::template add<Cls2, F, wasm_allocator>(mod, name);
      }
   };

#undef AUTO_PARAM_WORKAROUND

}} // namespace eosio::vm
