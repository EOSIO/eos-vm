#pragma once

#include <string>
#include <system_error>
#include <type_traits>

#define GENERATE_ERROR_CATEGORY(CATEGORY, NAME)                                                                        \
   namespace eosio { namespace vm {                                                                                    \
   struct CATEGORY##_category : std::error_category {                                                                  \
      const char* name() const noexcept override { return #NAME; }                                                     \
      std::string message(int ev) const override;                                                                      \
   };                                                                                                                  \
   const CATEGORY##_category __##CATEGORY##_category{};                                                                \
   template <typename T>                                                                                               \
   static inline constexpr auto is_a( const std::error_code& ec )                                                      \
      -> std::enable_if_t<std::is_same_v<T, CATEGORY##_category>, bool>{                                               \
      return ec.category() == __##CATEGORY##_category;                                                                 \
   }                                                                                                                   \
   }}

#define GENERATE_ENUM_ELEM( PARENT, ITEM ) \
   ITEM,

#define GENERATE_STR_ELEM( PARENT, ITEM ) \
   case PARENT::ITEM: return #ITEM;

#define CREATE_ERROR_CODES(CATEGORY, ERRORS)                                                                           \
   namespace eosio { namespace vm {                                                                                    \
   enum class CATEGORY { ERRORS(GENERATE_ENUM_ELEM) };                                                                 \
   }}                                                                                                                  \
   namespace std {                                                                                                     \
   template <>                                                                                                         \
   struct is_error_code_enum<eosio::vm::CATEGORY> : true_type {};                                                      \
   }                                                                                                                   \
   namespace eosio { namespace vm {                                                                                    \
   std::string CATEGORY##_category::message(int ev) const {                                                            \
      switch (static_cast<CATEGORY>(ev)) { ERRORS(GENERATE_STR_ELEM) }                                                 \
   }                                                                                                                   \
   std::error_code make_error_code(CATEGORY e) noexcept { return { static_cast<int>(e), __##CATEGORY##_category }; }   \
   }}                                                                                                                  \
