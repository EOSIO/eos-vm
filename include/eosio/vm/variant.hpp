#pragma once

// temporarily use exceptions
#include <eosio/vm/exceptions.hpp>
#include <eosio/vm/outcome.hpp>
#include <eosio/vm/utils.hpp>

#include <algorithm>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <variant>
#include <functional>

namespace eosio { namespace vm {

   // forward declaration
   template <typename... Alternatives>
   class variant;

   // implementation details
   namespace detail {

      template <typename... Ts>
      constexpr std::size_t max_layout_size_v = std::max({sizeof(Ts)...});

      template <typename... Ts>
      constexpr std::size_t max_alignof_v = std::max({alignof(Ts)...});

      template <typename T, typename... Alternatives>
      constexpr bool is_valid_alternative_v = (... + (std::is_same_v<T, Alternatives>?1:0)) != 0;

      template <typename T, typename Alternative, typename... Alternatives>
      constexpr std::size_t get_alternatives_index_v =
               std::is_same_v<T, Alternative> ? 0 : get_alternatives_index_v<T, Alternatives...> + 1;

      template <typename T, typename Alternative>
      constexpr std::size_t get_alternatives_index_v<T, Alternative> = 0;

      template <std::size_t I, typename... Alternatives>
      using get_alternative_t = std::tuple_element_t<I, std::tuple<Alternatives...>>;

      template <bool Valid, typename Ret>
      struct dispatcher;

      template <typename Ret>
      struct dispatcher<false, Ret> {
         template <std::size_t I, typename Vis, typename Var>
         static constexpr Ret _case(Vis&&, Var&&) {
            __builtin_unreachable();
         }
         template <std::size_t I, typename Vis, typename Var>
         static constexpr Ret _switch(Vis&&, Var&&) {
            __builtin_unreachable();
         }
      };

      template <typename Ret>
      struct dispatcher<true, Ret> {
         template <std::size_t I, typename Vis, typename Var>
         static constexpr Ret _case(Vis&& vis, Var&& var) {
            return std::invoke(std::forward<Vis>(vis), std::forward<Var>(var).template get<I>());
         }

         template <std::size_t I, typename Vis, typename Var>
         static constexpr Ret _switch(Vis&& vis, Var&& var) {
            constexpr std::size_t sz = std::decay_t<Var>::variant_size();
            switch (var.index()) {
               case I + 0: {
                  return dispatcher<I + 0 < sz, Ret>::template _case<I + 0>(std::forward<Vis>(vis),
                                                                            std::forward<Var>(var));
               }
               case I + 1: {
                  return dispatcher<I + 1 < sz, Ret>::template _case<I + 1>(std::forward<Vis>(vis),
                                                                            std::forward<Var>(var));
               }
               case I + 2: {
                  return dispatcher<I + 2 < sz, Ret>::template _case<I + 2>(std::forward<Vis>(vis),
                                                                            std::forward<Var>(var));
               }
               case I + 3: {
                  return dispatcher<I + 3 < sz, Ret>::template _case<I + 3>(std::forward<Vis>(vis),
                                                                            std::forward<Var>(var));
               }
               default: {
                  return dispatcher<I + 4 < sz, Ret>::template _switch<I + 4>(std::forward<Vis>(vis),
                                                                              std::forward<Var>(var));
               }
            }
         }
      };
   } // namespace detail

   template <class Visitor, typename Variant>
   constexpr auto visit(Visitor&& vis, Variant&& var) {
      using Ret = decltype(std::invoke(std::forward<Visitor>(vis), var.template get<0>()));
      return detail::dispatcher<true, Ret>::template _switch<0>(std::forward<Visitor>(vis), std::forward<Variant>(var));
   }

   template <typename... Alternatives>
   class variant {
      static_assert(sizeof...(Alternatives) <= std::numeric_limits<uint8_t>::max()+1,
                    "eosio::vm::variant can only accept 256 alternatives");

    public:
      variant() = default;
      variant(const variant& other) = default;
      variant(variant&& other) = default;

      variant& operator=(const variant& other) = default;
      variant& operator=(variant&& other) = default;

      template <typename T, typename = std::enable_if_t<detail::is_valid_alternative_v<std::decay_t<T>, Alternatives...>>>
      variant(T&& alt) {
         new (&_storage) std::decay_t<T>(std::forward<T>(alt));
         _which = detail::get_alternatives_index_v<std::decay_t<T>, Alternatives...>;
      }

      template <typename T, typename = std::enable_if_t<detail::is_valid_alternative_v<std::decay_t<T>, Alternatives...>>>
      variant& operator=(T&& alt) {
         new (&_storage) std::decay_t<T>(std::forward<T>(alt));
         _which = detail::get_alternatives_index_v<std::decay_t<T>, Alternatives...>;
         return *this;
      }

      static inline constexpr size_t variant_size() { return sizeof...(Alternatives); }
      inline constexpr uint16_t      index() const { return _which; }

      template <size_t Index>
      inline constexpr auto&& get_check() {
         // TODO add outcome stuff
         return 3;
      }

      template <size_t Index>
      inline constexpr const auto& get() const & {
         return reinterpret_cast<const typename detail::get_alternative_t<Index, Alternatives...>&>(_storage);
      }

      template <typename Alt>
      inline constexpr const Alt& get() const & {
         return reinterpret_cast<const Alt&>(_storage);
      }

      template <size_t Index>
      inline constexpr const auto&& get() const && {
         return reinterpret_cast<const typename detail::get_alternative_t<Index, Alternatives...>&&>(_storage);
      }

      template <typename Alt>
      inline constexpr const Alt&& get() const && {
         return reinterpret_cast<const Alt&&>(_storage);
      }

      template <size_t Index>
      inline constexpr auto&& get() && {
         return reinterpret_cast<typename detail::get_alternative_t<Index, Alternatives...>&&>(_storage);
      }

      template <typename Alt>
      inline constexpr Alt&& get() && {
         return reinterpret_cast<Alt&&>(_storage);
      }

      template <size_t Index>
      inline constexpr auto& get() & {
         return reinterpret_cast<typename detail::get_alternative_t<Index, Alternatives...>&>(_storage);
      }

      template <typename Alt>
      inline constexpr Alt& get() & {
         return reinterpret_cast<Alt&>(_storage);
      }

      template <typename Alt>
      inline constexpr bool is_a() {
         return _which == detail::get_alternatives_index_v<Alt, Alternatives...>;
      }
      inline constexpr void toggle_exiting_which() { _which ^= 0x100; }
      inline constexpr void clear_exiting_which() { _which &= 0xFF; }
      inline constexpr void set_exiting_which() { _which |= 0x100; }

    private:
      static constexpr size_t _sizeof  = detail::max_layout_size_v<Alternatives...>;
      static constexpr size_t _alignof = detail::max_alignof_v<Alternatives...>;
      uint16_t _which                  = 0;
      alignas(_alignof) std::array<char, _sizeof> _storage;
   };

}} // namespace eosio::vm
