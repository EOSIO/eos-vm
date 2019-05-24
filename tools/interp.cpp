#include <eosio/vm/backend.hpp>

#include <filesystem>
#include <iostream>

using namespace eosio;
using namespace eosio::vm;

/**
 * Simple implementation of an interpreter using eos-vm.
 */
int main(int argc, char** argv) {
   // Thread specific `allocator` used for wasm linear memory.
   wasm_allocator wa;
   // Specific the backend with no "host" for host functions.
   using backend_t = eosio::vm::backend<nullptr_t>;

   if (argc < 2) {
      std::cerr << "Error, no wasm file provided\n";
      return -1;
   }

   try {
      if (!std::filesystem::is_regular_file( argv[1] )) {
         std::cerr << "Error, " << argv[1] << " is not a file.\n";
	 return -1;
      }

      // Read the wasm into memory.
      auto code = backend_t::read_wasm( argv[1] );

      // Instaniate a new backend using the wasm provided.
      backend_t bkend( code );

      // Point the backend to the allocator you want it to use.
      bkend.set_wasm_allocator( &wa );

      // Execute any exported functions provided by the wasm.
      bkend.execute_all(nullptr);

   } catch ( ... ) {
      std::cerr << "eos-vm interpreter error\n";
   }
   return 0;
}
