#include <eosio/vm/span.hpp>

#include <catch2/catch.hpp>

using namespace eosio::vm;

TEST_CASE("Testing span raw pointer", "[span_raw_ptr_tests]") {
   const char* bytes = "hello";
   span<const char> bytes_span_0( bytes, 0 );
   span<const char> bytes_span_1( bytes, 3 );

   CHECK(bytes_span_0.empty());
   CHECK(!bytes_span_1.empty());
}
