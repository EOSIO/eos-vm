#include <eosio/vm/backend.hpp>
#include <eosio/vm/error_codes.hpp>
#include <eosio/vm/watchdog.hpp>
#include <eosio/vm/profile.hpp>

#include <iostream>

using namespace eosio;
using namespace eosio::vm;

/**
 * Simple implementation of an interpreter using eos-vm.
 */
int main(int argc, char** argv) {
   // Thread specific `allocator` used for wasm linear memory.
   wasm_allocator wa;

   if (argc < 2) {
      std::cerr << "Error, no wasm file provided\n";
      return -1;
   }

   watchdog wd{std::chrono::seconds(3)};

   try {
      // Read the wasm into memory.
      auto code = read_wasm( argv[1] );

      // Instaniate a new backend using the wasm provided.
      backend<std::nullptr_t, jit, default_options, profile_instr_map> bkend( code, &wa );
      profile_data prof("profile.out", bkend);
      scoped_profile profile_runner(prof);

      // Execute any exported functions provided by the wasm.
      bkend.execute_all(wd);

   } catch ( const eosio::vm::exception& ex ) {
      std::cerr << "eos-vm interpreter error\n";
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   }
   return 0;
}
