#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <eosio/vm/allocator.hpp>
#include <eosio/vm/stack_elem.hpp>
#include <eosio/vm/utils.hpp>

#include "test_runner.hpp"

struct type_converter32 {
   union {
      uint32_t ui;
      float    f;
   } _data;
   type_converter32(uint32_t n) { _data.ui = n; }
   uint32_t to_ui() const { return _data.ui; }
   float    to_f() const { return _data.f; }
};

struct type_converter64 {
   union {
      uint64_t ui;
      double   f;
   } _data;
   type_converter64(uint64_t n) { _data.ui = n; }
   uint64_t to_ui() const { return _data.ui; }
   double   to_f() const { return _data.f; }
};

// C++20: using std::bit_cast;
template<typename T, typename U>
T bit_cast(const U& u) {
   static_assert(sizeof(T) == sizeof(U), "bitcast requires identical sizes.");
   T result;
   std::memcpy(&result, &u, sizeof(T));
   return result;
}


inline bool check_nan(const std::optional<eosio::vm::operand_stack_elem>& v) {
   return visit(eosio::vm::overloaded{[](eosio::vm::i32_const_t){ return false; },
                                      [](eosio::vm::i64_const_t){ return false; },
                                      [](eosio::vm::f32_const_t f) { return std::isnan(f.data.f); },
                                      [](eosio::vm::f64_const_t f) { return std::isnan(f.data.f); }}, *v);
}

inline eosio::vm::wasm_allocator* get_wasm_allocator() {
   static eosio::vm::wasm_allocator alloc;
   return &alloc;
}

#define BACKEND_TEST_CASE(name, tags) \
  TEMPLATE_TEST_CASE(name, tags, eosio::vm::interpreter, eosio::vm::jit)
