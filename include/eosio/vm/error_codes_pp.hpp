#pragma once

#include <string>
#include <system_error>

#define GENERATE_ERROR_CATEGORY( CATEGORY, NAME ) \
   struct CATEGORY ## _category : std::error_category { \
      const char* name() const noexcept override { return #NAME; } \
      std::string message(int ev) const override; \
   }; \
   const CATEGORY ## _category __ ## CATEGORY ## _category{};

#define GENERATE_ENUM_ELEM( PARENT, ITEM ) \
   ITEM,

#define GENERATE_STR_ELEM( PARENT, ITEM ) \
   case PARENT::ITEM: return #ITEM;

#define CREATE_ERROR_CODES( CATEGORY, ERRORS ) \
   enum class CATEGORY { \
      ERRORS( GENERATE_ENUM_ELEM ) \
   }; \
   namespace std { \
      template <> \
      struct is_error_code_enum<CATEGORY> : true_type {}; \
   }\
   \
   std::string CATEGORY ## _category::message(int ev)const { \
      switch (static_cast<CATEGORY>(ev)) { \
         ERRORS( GENERATE_STR_ELEM ) \
      } \
   } \
   /*namespace { \*/ \
   std::error_code make_error_code(CATEGORY e) noexcept { \
      return {static_cast<int>(e), __ ## CATEGORY ## _category }; \
   /*} \ */ \
   }
