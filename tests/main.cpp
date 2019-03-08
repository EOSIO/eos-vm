/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <cstdlib>
#include <iostream>
#include <boost/test/included/unit_test.hpp>
//#include <fc/log/logger.hpp>
#include <eosio/wasm_backend/exceptions.hpp>

void translate_exception(const eosio::wasm_backend::exception& e) {
   std::cout << "\033[33m WHAT " <<  e.what() << " " << e.detail() << "\033[0m" << std::endl;

   BOOST_FAIL("Caught Unexpected Exception");
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[]) {
   // Turn off blockchain logging if no --verbose parameter is not added
   // To have verbose enabled, call "tests/chain_test -- --verbose"
   bool is_verbose = false;
   std::string verbose_arg = "--verbose";
   for (int i = 0; i < argc; i++) {
      if (verbose_arg == argv[i]) {
         is_verbose = true;
         break;
      }
   }
   //if(!is_verbose) fc::logger::get(DEFAULT_LOGGER).set_log_level(fc::log_level::off);

   // Register fc::exception translator
   boost::unit_test::unit_test_monitor.register_exception_translator<eosio::wasm_backend::exception>(&translate_exception);

   std::srand(time(NULL));
   std::cout << "Random number generator seeded to " << time(NULL) << std::endl;

   return nullptr;
}
