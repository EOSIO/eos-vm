#include "variant.hpp"
#include "error_codes_def.hpp"
#include <variant>

using namespace eosio::vm;

struct vis {
   void operator()(int v) {
      std::cout << "Visiting int " << v << "\n";
   }
   void operator()(float v) {
      std::cout << "Visiting float " << v << "\n";
   }
   void operator()(double v) {
      std::cout << "Visiting double " << v << "\n";
   }
   void operator()(const std::string& v) {
      std::cout << "Visiting string " << v << "\n";
   }
   void operator()(unsigned long long v) {
      std::cout << "Visiting ull " << v << "\n";
   }
};

int main() {
   std::cout << "Variant\n";
   unsigned long long s = 34;
   variant<int, float, double, unsigned long long> v(53ull);
   visit( vis{}, v );
   visit( overloaded { [](int v) { std::cout << "Visiting int from lambda " << v << "\n"; },
                       [](float v) { std::cout << "Visiting float from lambda " << v << "\n"; },
                       [](double v) { std::cout << "Visiting double from lambda " << v << "\n"; },
                       [&](auto v) { std::cout << "Visiting other \n"; s += 15; } }, v );

   std::cout << s << "\n";
   std::error_code ec = parser_errors::invalid_magic_number;   
   std::cout << "EC " << ec << "\n";
}
