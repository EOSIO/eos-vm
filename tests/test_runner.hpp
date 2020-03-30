#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct vm_result {
   explicit vm_result(uint32_t v) { value.ui32 = v; }
   explicit vm_result(int32_t  v) { value.i32  = v; }
   explicit vm_result(uint64_t v) { value.ui64 = v; }
   explicit vm_result(float v)    { value.f    = v; }
   explicit vm_result(double v)   { value.d    = v; }

   template <typename T>
   auto as() const {
      if constexpr (std::is_same_v<uint32_t, T>)
         return value.ui32;
      else if constexpr (std::is_same_v<int32_t, T>)
         return value.i32;
      else if constexpr (std::is_same_v<uint64_t, T>)
         return value.ui64;
      else if constexpr (std::is_same_v<int64_t, T>)
         return value.i64;
      else if constexpr (std::is_same_v<float, T>)
         return value.f;
      else if constexpr (std::is_same_v<double, T>)
         return value.d;
      else
         return;
   }

   union {
      float  f;
      double d;
      uint32_t ui32;
      int32_t  i32;
      uint64_t ui64;
      int64_t  i64;
   } value;
};

struct interp_impl;

struct interp_test_runner {

   interp_test_runner( std::vector<uint8_t>& code ) : instance(get_impl_instance(code)) {}
   interp_test_runner( const std::string& code ) : instance(get_impl_instance(code)) {}
   ~interp_test_runner();

   template <typename... Params>
   vm_result call_impl(const std::string& mod, const std::string& name, Params... params) const;

   template <typename T, typename... Ts>
   T call(const std::string& mod, const std::string& name, Ts... params) const {
      return call_impl(mod, name, params...).template as<T>();
   }

   void exec_all() const;

   interp_impl* get_impl_instance(std::vector<uint8_t>& code_path) const;
   interp_impl* get_impl_instance(const std::string& code_path) const;

   interp_impl* instance;
};

struct jit_impl;

struct jit_test_runner {
   jit_test_runner( std::vector<uint8_t>& code ) : instance(get_impl_instance(code)) {}
   jit_test_runner( const std::string& code ) : instance(get_impl_instance(code)) {}
   ~jit_test_runner();

   template <typename... Params>
   vm_result call_impl(const std::string& mod, const std::string& name, Params... params) const;

   template <typename T, typename... Ts>
   T call(const std::string& mod, const std::string& name, Ts... params) const {
      return call_impl(mod, name, params...).template as<T>();
   }

   void exec_all() const;

   jit_impl* get_impl_instance(std::vector<uint8_t>& code_path) const;
   jit_impl* get_impl_instance(const std::string& code_path) const;

   jit_impl* instance;
};

