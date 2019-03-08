#include <iostream>
#include <fstream>
#include <chrono>
#include <eosio/wasm_backend/interpret_visitor.hpp>
#include <eosio/wasm_backend/types.hpp>
#include <eosio/wasm_backend/parser.hpp>
#include <eosio/wasm_backend/execution_context.hpp>

using namespace eosio;
using namespace eosio::wasm_backend;

std::vector<uint8_t> read_wasm( const std::string& fname ) {
   std::ifstream wasm_file(fname, std::ios::binary);
   if (!wasm_file.is_open())
      throw std::runtime_error("wasm file cannot be found");
   wasm_file.seekg(0, std::ios::end);
   std::vector<uint8_t> wasm; 
   int len = wasm_file.tellg();
   if (len < 0)
      throw std::runtime_error("wasm file length is -1");
   wasm.resize(len);
   wasm_file.seekg(0, std::ios::beg);
   wasm_file.read((char*)wasm.data(), wasm.size());
   wasm_file.close();
   return wasm;
}

int main(int argc, char** argv) {
   memory_manager::set_memory_limits( 32 * 1024 * 1024 );
   binary_parser bp;
   module mod;
   wasm_code code = read_wasm(argv[1]);
   bp.parse_module( code, mod );
   auto t1 = std::chrono::high_resolution_clock::now();
   execution_context<interpret_visitor> ctx(mod);
   try {
      //ctx.execute("apply", (uint64_t)12, (uint64_t)13, (uint64_t)14);
      ctx.execute("main");
      //ctx.execute("dothedew");
   } catch ( const wasm_interpreter_exception& ex ) {
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   } catch ( const wasm_invalid_element& ex ) {
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   } catch ( const wasm_memory_exception& ex ) {
      std::cerr << ex.what() << " : " << ex.detail() << "\n";
   }
   auto t2 = std::chrono::high_resolution_clock::now();
   std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count() << '\n';
   return 0;
}

