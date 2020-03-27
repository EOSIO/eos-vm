#include <eosio/vm/span.hpp>

#include <catch2/catch.hpp>

using eosio::vm::span;

TEST_CASE("span static_extent tests", "[span_static_extent]") {
   // currently we only support a static extent of 1
   int a = 0;
   span<int, 1> s(&a);

   CHECK(s.data() == std::addressof(a));
   CHECK(s.front() == a);
   s.front() += 3;
   CHECK(s.front() == a);
   CHECK(3 == a);

   int* lta = &a;
   CHECK_THROWS_AS(span<int>(&a, lta-1), eosio::vm::span_exception);
}
