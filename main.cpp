#include <iostream>
#include <fstream>
#include <chrono>
#include <eosio/wasm_backend/backend.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;

struct sstr {
   sstr(const char* s, size_t l) {
      data = new char[l];
      len  = l;
   }
   char* data;
   size_t len;
};

struct foo_s {
   void mabort() {
      std::cout << "ABORT!!!\n";
   }

   void eosio_assert(const char*) {
      std::cout << "EOSIO ASSERT\n";
   }

   void fmemset(char* dp, const char* sp, int32_t len) {
      for (int i=0; i < len; i++)
         dp[i] = sp[i];
   }

   void print(const char* s) {
      //std::cout << "PRINT " << a << " " << b << " " << c << " " << d << " " << e << " " << f << " " << g << "\n";
      std::cout << "PRINT " << s << "\n";
   }
};

//registered_function<&print, decltype("env"_hfn), decltype("print"_hfn), backend> _rf;

int main(int argc, char** argv) {
   wasm_allocator wa;
   using backend_t = eosio::wasm_backend::backend<foo_s>;
   using rhf_t     = eosio::wasm_backend::registered_host_functions<foo_s>;
   auto code = backend_t::read_wasm( argv[1] );
   auto t1 = std::chrono::high_resolution_clock::now();
   backend_t bkend( code );
   bkend.set_wasm_allocator( &wa );
   rhf_t::add<foo_s, &foo_s::print, backend_t>("env", "print");
   rhf_t::add<foo_s, &foo_s::mabort, backend_t>("env", "abort");
   rhf_t::add<foo_s, &foo_s::eosio_assert, backend_t>("env", "eosio_assert");
   rhf_t::add<foo_s, &foo_s::fmemset, backend_t>("env", "memset");
   rhf_t::resolve( bkend.get_module() );
   try {
      foo_s fs;
      bkend(&fs, "env", "apply", (uint64_t)0, (uint64_t)0, (uint64_t)0);
   } catch ( const wasm_interpreter_exception& ex ) {
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   } catch ( const wasm_invalid_element& ex ) {
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   } catch ( const wasm_memory_exception& ex ) {
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   }

   std::cout << "INSTRUCTIONS : " << bkend.get_instructions() << "\n";
   auto t2 = std::chrono::high_resolution_clock::now();
   std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << '\n';
   return 0;
}
