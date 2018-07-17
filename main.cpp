#include <iostream>
#include <eosio/wasm_backend/wasm_interpreter.hpp>
#include <eosio/wasm_backend/integer_types.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;
int main() {
   eosio::wasm_backend::wasm_interpreter wi;
   wi.print();
   varuint<7> l(129);
   std::cout << "RAW " << *(uint32_t*)l.raw << " size " << (uint32_t)l.size << "\n";
//   str_bits(l);
   std::cout << "U " << l.get() << "\n";

   varint<7> l2(-256);
   std::cout << "RAW " << *(uint32_t*)l2.raw << " size " << (uint32_t)l2.size << "\n";
   //str_bits(l2);
   std::cout << "U " << l2.get() << "\n";

   return 0;
}
