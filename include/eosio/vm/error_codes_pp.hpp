#pragma once

#include <string>
#include <system_error>

#define GENERATE_ERROR_CATEGORY(CATEGORY, NAME)                                                                        \
   EOS_VM_OPEN_NAMESPACE                                                                                               \
   struct CATEGORY##_category : std::error_category {                                                                  \
      const char* name() const noexcept override { return #NAME; }                                                     \
      std::string message(int ev) const override;                                                                      \
   };                                                                                                                  \
   const CATEGORY##_category __##CATEGORY##_category{};
   EOS_VM_CLOSE_NAMESPACE

#define GENERATE_ENUM_ELEM( PARENT, ITEM ) \
   ITEM,

#define GENERATE_STR_ELEM( PARENT, ITEM ) \
   case PARENT::ITEM: return #ITEM;

#define STD_NAMESPACE_OPEN namespace std {
#define STD_NAMESPACE_CLOSE }

#define CREATE_ERROR_CODES(CATEGORY, ERRORS)                                                                           \
   EOS_VM_OPEN_NAMESPACE                                                                                               \
   enum class CATEGORY { ERRORS(GENERATE_ENUM_ELEM) };                                                                 \
   EOS_VM_CLOSE_NAMESPACE                                                                                              \
   STD_NAMESPACE_OPEN                                                                                                  \
   template <>                                                                                                         \
   struct is_error_code_enum<CATEGORY> : true_type {};                                                                 \
   STD_NAMESPACE_CLOSE                                                                                                 \
   EOS_VM_OPEN_NAMESPACE                                                                                               \
   std::string CATEGORY##_category::message(int ev) const {                                                            \
      switch (static_cast<CATEGORY>(ev)) { ERRORS(GENERATE_STR_ELEM) }                                                 \
   }                                                                                                                   \
   std::error_code make_error_code(CATEGORY e) noexcept { return { static_cast<int>(e), __##CATEGORY##_category }; }   \
   EOS_VM_CLOSE_NAMESPACE

#undef STD_NAMESPACE_OPEN
#undef STD_NAMESPACE_CLOSE
