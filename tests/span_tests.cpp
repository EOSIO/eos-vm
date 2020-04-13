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

TEST_CASE("default constructor", "[span_dynamic_extent]") {
   //static_assert(std::is_constructible_v<span<int, 0>>);
   static_assert(!std::is_constructible_v<span<int, 1>>);
   static_assert(std::is_constructible_v<span<int>>);
   span<int> s;
   CHECK(s.size() == 0);
   CHECK(s.data() == nullptr);
}

TEST_CASE("array constructor", "[span]") {
   static_assert(std::is_constructible_v<span<int>, int(&)[1]>);
   static_assert(std::is_constructible_v<span<const int>, int(&)[1]>);
   static_assert(std::is_constructible_v<span<const int>, const int(&)[1]>);
   static_assert(std::is_constructible_v<span<int, 1>, int(&)[1]>);
   static_assert(std::is_constructible_v<span<const int, 1>, int(&)[1]>);
   static_assert(std::is_constructible_v<span<const int, 1>, const int(&)[1]>);
   static_assert(!std::is_constructible_v<span<int>, long(&)[1]>);
   static_assert(!std::is_constructible_v<span<int, 1>, int(&)[2]>);
   static_assert(!std::is_constructible_v<span<int>, const int(&)[1]>);
   static_assert(!std::is_constructible_v<span<int, 1>, const int(&)[1]>);
   int a1[1];
   span<int> s1(a1);
   CHECK(s1.data() == a1);
   CHECK(s1.size() == 1);
}

TEST_CASE("std::array constructor", "[span]") {
   static_assert(std::is_constructible_v<span<int>, std::array<int, 1>&>);
   static_assert(std::is_constructible_v<span<const int>, std::array<int, 1>&>);
   static_assert(std::is_constructible_v<span<const int>, const std::array<int, 1>&>);
   static_assert(std::is_constructible_v<span<int, 1>, std::array<int, 1>&>);
   static_assert(std::is_constructible_v<span<const int, 1>, std::array<int, 1>&>);
   static_assert(std::is_constructible_v<span<const int, 1>, const std::array<int, 1>&>);
   static_assert(!std::is_constructible_v<span<int>, std::array<long, 1>&>);
   static_assert(!std::is_constructible_v<span<int, 1>, std::array<int, 2>&>);
   static_assert(!std::is_constructible_v<span<int>, const std::array<int, 1>&>);
   static_assert(!std::is_constructible_v<span<int, 1>, const std::array<int, 1>&>);
   std::array<int, 1> a1;
   span<int> s1(a1);
   CHECK(s1.data() == a1.data());
   CHECK(s1.size() == 1);
}

namespace eosio::vm {
template<typename T, std::size_t Extent>
bool operator==(const span<T, Extent>& lhs, const span<T, Extent>& rhs) {
   return lhs.data() == rhs.data() && lhs.size() == rhs.size();
}
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

   for ( auto& elem : s ) {
      elem = 7;
   }

   for ( int i=0; i < 10; i++ ) {
      CHECK(a[i] == s[i]);
   }

   CHECK(s.first(3) == span<int>(a, 3));
   CHECK(s.last(3)  == span<int>(a+7, 3));
   CHECK(s.subspan(3, 3) == span<int>(a+3, 3));
   CHECK(s.subspan(3, eosio::vm::dynamic_extent) == span<int>(a+3, 7));
}
