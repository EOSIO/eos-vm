#include <eosio/vm/watchdog.hpp>
#include <chrono>

#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

using eosio::vm::watchdog;

BOOST_AUTO_TEST_SUITE(watchdog_tests)

BOOST_AUTO_TEST_CASE(watchdog_interrupt) {
  std::atomic<bool> okay = false;
  watchdog w{std::chrono::milliseconds(50)};
  {
    auto g = w.scoped_run([&]() { okay = true; });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  BOOST_TEST(okay);
}

BOOST_AUTO_TEST_CASE(watchdog_no_interrupt) {
  std::atomic<bool> okay = true;
  watchdog w{std::chrono::milliseconds(50)};
  {
    auto g = w.scoped_run([&]() { okay = false; });
  } // the guard goes out of scope here, cancelling the timer
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  BOOST_TEST(okay);
}

BOOST_AUTO_TEST_SUITE_END()