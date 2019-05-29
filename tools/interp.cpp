#include <eosio/vm/backend.hpp>
#include <eosio/vm/error_codes.hpp>
#include <eosio/vm/watchdog.hpp>
#include <eosio/vm/detail/module_macros.hpp>

#include <iostream>

using namespace eosio;
using namespace eosio::vm;

template <auto S>
struct SS {
  using type = decltype(S);
};

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

   watchdog<std::chrono::nanoseconds> wd;
   wd.set_duration(std::chrono::seconds(3));
   try {
     /*
      // Read the wasm into memory.
      auto code = backend_t::read_wasm( argv[1] );

      // Instantiate a new backend using the wasm provided.
      backend_t bkend( code );
      wd.set_callback([&](){
		      bkend.get_context().exit();
		      });

      // Point the backend to the allocator you want it to use.
      bkend.set_wasm_allocator( &wa );

      // Execute any exported functions provided by the wasm.
      bkend.execute_all(&wd);
     */
      auto bt = detail::bound_tuple(detail::bound_pair("this"_c, (int)44));
      auto& ii = bt.get<("this"_c)::t>();
      ii = 24;
      std::cout << "I " << bt.get<("this"_c)::t>();
   } catch (...) { std::cerr << "eos-vm interpreter error\n"; }
   return 0;
}
/*

class ModuleInterface {
  void apply( int, int, int ) = 0;
  import<"apply", void(int,int,int) > apply;
  export<"env", "eosio_assert", 
}

  backend<ModuleInterface, "apply", .... >
  BACKEND( ModuleInterface, (apply) 

  class blackend {

    std::function<void(int,int,int)> apply;
  }


           class context : ModuleInterface {
              interpreter
              timeout_time
              memlimit
              self...

              context& timeout(t){ return *this; }
             ModuleInterface api;


           }

           con.timeout(0).api.apply(...)


           module_interface mi(module, ptr_to_context);
           mi.apply(...);


           module::read(...);
auto module = bet::read(...);
module.execute_all( timeout );
           module.timeout(x).memlimit(z).apply( 1, 2, 3 );

           backend.exec(module).timeout()... 

*/
