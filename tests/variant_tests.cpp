#include <eosio/vm/variant.hpp>

#include <catch2/catch.hpp>

using namespace eosio;
using namespace eosio::vm;

TEST_CASE("Testing variant with stateless class", "[variant_stateless_tests]") {
   struct vis {
      int operator()(signed char v) { return v + 1; }
      int operator()(short v) { return v + 2; }
      int operator()(int v) { return v + 3; }
      int operator()(long v) { return v + 4; }
      int operator()(float v) { return v + 5.0f; }
      int operator()(double v) { return v + 6.0; }
   };

   using variant_type = variant<signed char, short, int, long, float, double>;

   {
      variant_type v((signed char)42);
      CHECK(visit(vis{}, v) == 43);
   }
   {
      variant_type v((short)142);
      CHECK(visit(vis{}, v) == 144);
   }
   {
      variant_type v((int)242);
      CHECK(visit(vis{}, v) == 245);
   }
   {
      variant_type v((long)342);
      CHECK(visit(vis{}, v) == 346);
   }
   {
      variant_type v((float)442);
      CHECK(visit(vis{}, v) == 447);
   }
   {
      variant_type v((double)542);
      CHECK(visit(vis{}, v) == 548);
   }
}


TEST_CASE("Testing argument forwarding for visit", "[variant_forward_tests]") {

   using variant_type = variant<signed char, short, int, long, float, double>;

   // visitor forwarding

   // lvalue
   {
      struct vis {
         int r = -1;
         void operator()(double v) & { r = v + 1; }
         void operator()(...) {}
      };
      variant_type v((double)42);
      vis f;
      visit(f, v);
      CHECK(f.r == 43);
   }
   // rvalue
   {
      struct vis {
         int r = -1;
         void operator()(double v) && { r = v + 1; }
         void operator()(...) {}
      };
      variant_type v((double)42);
      vis f;
      visit(std::move(f), v);
      CHECK(f.r == 43);
   }
   // const lvalue
   {
      struct vis {
         mutable int r = -1;
         void operator()(double v) const & { r = v + 1; }
         void operator()(...) {}
      };
      variant_type v((double)42);
      const vis f;
      visit(f, v);
      CHECK(f.r == 43);
   }

   // variant forwarding

   // lvalue
   {
      struct vis {
         void operator()(double& v) { v = v + 1; }
         void operator()(...) {}
      };
      variant_type v((double)42);
      visit(vis{}, v);
      CHECK(v.get<double>() == 43);
   }

   // rvalue
   {
      struct vis {
         void operator()(double&& v) {
            v = v + 1;
         }
         void operator()(...) {}
      };
      variant_type v((double)42);
      visit(vis{}, std::move(v));
      CHECK(v.get<double>() == 43);
   }

   // const lvalue
   {
      struct vis {
         void operator()(const double& v) {
            const_cast<double&>(v) = v + 1;
         }
         void operator()(...) {}
      };
      // Don't declare this const, because that would make casting away const illegal.
      variant_type v((double)42);
      visit(vis{}, static_cast<const variant_type&>(v));
      CHECK(v.get<double>() == 43);
   }
}

// Minimal requirements.  Delete everything that shouldn't be needed by visit
struct minimal_vis {
   minimal_vis(const minimal_vis&) = delete;
   minimal_vis& operator=(const minimal_vis&) = delete;
   template<typename T>
   int operator()(T v) const {
      return v + 1;
   }
   static minimal_vis& make() { static minimal_vis singleton; return singleton; }
 private:
   minimal_vis() = default;
   ~minimal_vis() = default;
};
void check_requirements() {
   using variant_type = variant<signed char, short, int, long, float, double>;
   variant_type v((double)42);
   visit(minimal_vis::make(), v);
   visit(std::move(minimal_vis::make()), v);
   visit(const_cast<const minimal_vis&>(minimal_vis::make()), v);
}
