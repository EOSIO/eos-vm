#include <iostream>
#include <fstream>
#include <chrono>
#include <eosio/wasm_backend/backend.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;

int main(int argc, char** argv) {
   wasm_allocator wa;
   auto code = backend::read_wasm( argv[1] );
   auto t1 = std::chrono::high_resolution_clock::now();
   eosio::wasm_backend::backend bkend( code, wa );

   try {
      bkend.execute_all();
   } catch ( const wasm_interpreter_exception& ex ) {
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   } catch ( const wasm_invalid_element& ex ) {
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   } catch ( const wasm_memory_exception& ex ) {
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   }
   auto t2 = std::chrono::high_resolution_clock::now();
   std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << '\n';
   return 0;
}

