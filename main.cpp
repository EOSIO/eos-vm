#include <iostream>
#include <eosio/wasm_backend/wasm_interpreter.hpp>
#include <eosio/wasm_backend/integer_types.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;
int main() {
   eosio::wasm_backend::wasm_interpreter wi;
   wi.print();
   varuint<32> l(129);
   std::cout << "RAW " << (uint32_t)l.raw << "\n";
   str_bits(l);
   std::cout << "U " << l.get();
   return 0;
}
