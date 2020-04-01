#include <eosio/vm/span.hpp>

#include <catch2/catch.hpp>

using eosio::vm::span;

TEST_CASE("span static_extent tests", "[span_static_extent]") {
   // currently we only support a static extent of 1
   int a = 0;
   span<int, 1> s(&a, 1);

   CHECK(s.data() == std::addressof(a));
   CHECK(s.front() == a);
   s.front() += 3;
   CHECK(s.front() == a);
   CHECK(3 == a);

   int* lta = &a;
   CHECK_THROWS_AS(span<int>(&a, lta-1), eosio::vm::span_exception);
}

TEST_CASE("span dynamic_extent tests", "[span_dynamic_extent]") {
   // currently we only support a static extent of 1
   int a[10] = {1};
   span<int> s(a, 10);

   CHECK(s.data() == a);
   CHECK(s.front() == a[0]);
   s.front() += 3;
   CHECK(s.front() == a[0]);
   CHECK(4 == a[0]);

   span<int> test_a(a, 3);
   span<int> test_b(a + 7, 3);

   CHECK_THROWS_AS(s.at(10), eosio::vm::span_exception);
   for ( auto& elem : s ) {
      elem = 7;
   }

   for ( int i=0; i < 10; i++ ) {
      CHECK(a[i] == s[i]);
      CHECK(a[i] == s.at(i));
   }

   CHECK(s.first(3) == span<int>(a, 3));
   CHECK(s.last(3)  == span<int>(a+7, 3));
   CHECK(s.subspan(3, 3) == span<int>(a+3, 3));
}
