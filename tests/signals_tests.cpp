#include <eosio/vm/signals.hpp>
#include <chrono>
#include <csignal>
#include <thread>
#include <iostream>

#include <catch2/catch.hpp>

struct test_exception {};

TEST_CASE("Testing signals", "[invoke_with_signal_handler]") {
   bool okay = false;
   try {
      eosio::vm::invoke_with_signal_handler([]() {
         std::raise(SIGALRM);
      }, [](int sig) {
         throw test_exception{};
      });
   } catch(test_exception&) {
      okay = true;
   }
   CHECK(okay);
}

TEST_CASE("Testing signals with exceptions", "[signals_exceptions]") {
   eosio::vm::block_sigalrm_outer guard;
   pthread_t self = pthread_self();
   int max_signals = 100;
   std::thread killer([self, max_signals](){
      for(int i = 0; i < max_signals; ++i) {
         pthread_kill(self, SIGALRM);
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
   });
   int signal_count = 0;
   int failed = false;
   while (signal_count < max_signals) {
      bool okay = false;
      try {
         eosio::vm::invoke_with_signal_handler([](){
            eosio::vm::block_sigalrm_inner guard;
            throw test_exception{};
         }, [&](int sig){
            ++signal_count;
            throw test_exception{};
         });
      } catch(test_exception&) {
         okay = true;
      }
      failed = failed || !okay;
   }
   CHECK(!failed);
   killer.join();
}

TEST_CASE("Testing signals threaded", "[signals_threaded]") {
   eosio::vm::block_sigalrm_outer guard;
   pthread_t self = pthread_self();
   int max_signals = 100;
   std::thread killer([self, max_signals](){
      for(int i = 0; i < max_signals; ++i) {
         pthread_kill(self, SIGALRM);
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
   });
   int signal_count = 0;
   while (signal_count < max_signals) {
      int sig = 0;
      eosio::vm::invoke_with_signal_handler([](){}, [&](int signum) {
         sig = signum;
         ++signal_count;
      });
      if (sig)
         CHECK(sig == SIGALRM);
   }
   killer.join();
}


TEST_CASE("Testing blocking sigalrm", "[signals_block_sigalrm]") {
   eosio::vm::block_sigalrm_outer guard;
   pthread_t self = pthread_self();
   int max_signals = 100;
   std::thread killer([self, max_signals](){
      for(int i = 0; i < max_signals; ++i) {
         pthread_kill(self, SIGALRM);
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
   });
   int signal_count = 0;
   int depth = 0;
   while (signal_count < max_signals) {
      int sig = 0;
      eosio::vm::invoke_with_signal_handler([&depth](){
         eosio::vm::block_sigalrm_inner guard;
         ++depth;
         std::this_thread::sleep_for(std::chrono::microseconds(500));
         --depth;
      }, [&](int signum) {
         sig = signum;
         ++signal_count;
      });
      CHECK(depth == 0);
      if (sig)
         CHECK(sig == SIGALRM);
   }
   killer.join();
}

TEST_CASE("Testing canceling sigalrm", "[signals_cancel_sigalrm]") {
   eosio::vm::block_sigalrm_outer guard;
   std::raise(SIGALRM);
}
