#include "test_runner.hpp"
#include "utils.hpp"

#include <eosio/vm/backend.hpp>
#include <eosio/vm/host_function.hpp>
#include <eosio/vm/watchdog.hpp>

using namespace eosio;
using namespace eosio::vm;

struct vm_value_visitor {
   uint32_t operator()( const uint32_t v ) { return v; }
   int32_t  operator()( const int32_t v )  { return v; }
   uint64_t operator()( const uint64_t v ) { return v; }
   int64_t  operator()( const int64_t v )  { return v; }
   float    operator()( const float v )    { return v; }
   double   operator()( const double v )   { return v; }
};

vm_result from_stack_elem(operand_stack_elem&& elem) {
   if ( elem.is_a<i32_const_t>() )
      return vm_result{static_cast<uint32_t>(elem.get<i32_const_t>().data.ui)};
   if ( elem.is_a<f32_const_t>() )
      return vm_result{static_cast<uint32_t>(elem.get<f32_const_t>().data.ui)};
   if ( elem.is_a<i64_const_t>() )
      return vm_result{static_cast<uint32_t>(elem.get<i64_const_t>().data.ui)};
   else
      return vm_result{static_cast<uint32_t>(elem.get<f64_const_t>().data.ui)};
}

struct interp_impl {
   interp_impl(std::vector<uint8_t>& code)
      : bkend(code, get_wasm_allocator()) {}
   interp_impl(const std::string& code)
      : bkend(read_wasm(code), get_wasm_allocator()) {}
   backend<registered_host_functions<standalone_function_t>, interpreter> bkend;
};

interp_test_runner::~interp_test_runner() {
   delete instance;
}

template <typename... Params>
vm_result interp_test_runner::call_impl(const std::string& mod, const std::string& name, Params... params) const {
   return from_stack_elem(*(instance->bkend.call_with_return(mod, name, params...)));
}

void interp_test_runner::exec_all() const {
   instance->bkend.execute_all(eosio::vm::null_watchdog());
}

interp_impl* interp_test_runner::get_impl_instance(std::vector<uint8_t>& code) const {
   return new interp_impl(code);
}

interp_impl* interp_test_runner::get_impl_instance(const std::string& code) const {
   return new interp_impl(code);
}

struct jit_impl {
   jit_impl(std::vector<uint8_t>& code)
      : bkend(code, get_wasm_allocator()) {}
   jit_impl(const std::string& code)
      : bkend(read_wasm(code), get_wasm_allocator()) {}
   backend<registered_host_functions<standalone_function_t>, jit> bkend;
};

jit_test_runner::~jit_test_runner() {
   delete instance;
}

template <typename... Params>
vm_result jit_test_runner::call_impl(const std::string& mod, const std::string& name, Params... params) const {
   return from_stack_elem(*(instance->bkend.call_with_return(mod, name, params...)));
}

void jit_test_runner::exec_all() const {
   instance->bkend.execute_all(eosio::vm::null_watchdog());
}

jit_impl* jit_test_runner::get_impl_instance(std::vector<uint8_t>& code) const {
   return new jit_impl(code);
}

jit_impl* jit_test_runner::get_impl_instance(const std::string& code) const {
   return new jit_impl(code);
}

#define V (uint32_t)0
void explicit_instantiation() {
   std::vector<uint8_t> v;
   interp_test_runner(v).call_impl("", "", V);
   interp_test_runner(v).call_impl("", "", V, V);
   interp_test_runner(v).call_impl("", "", V, V, V);
   interp_test_runner(v).call_impl("", "", V, V, V, V);
   interp_test_runner(v).call_impl("", "", V, V, V, V, V);
   interp_test_runner(v).call_impl("", "", V, V, V, V, V, V);
   interp_test_runner(v).call_impl("", "", V, V, V, V, V, V, V);
   interp_test_runner(v).call_impl("", "", V, V, V, V, V, V, V, V);
   interp_test_runner(v).call_impl("", "", V, V, V, V, V, V, V, V, V);
   interp_test_runner(v).call_impl("", "", V, V, V, V, V, V, V, V, V, V);
   interp_test_runner(v).call_impl("", "", V, V, V, V, V, V, V, V, V, V, V);
   interp_test_runner(v).call_impl("", "", V, V, V, V, V, V, V, V, V, V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V, V, V, V, V, V, V, V, V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V, V, V, V, V, V, V, V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V, V, V, V, V, V, V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V, V, V, V, V, V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V, V, V, V, V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V, V, V, V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V, V, V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V, V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V, V);
   jit_test_runner(v).call_impl("", "",    V, V);
   jit_test_runner(v).call_impl("", "",    V);
}
#undef V
