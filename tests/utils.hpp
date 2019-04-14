#include <vector>
#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <fstream>
#include <string>

inline std::vector<uint8_t> read_wasm( const std::string& fname ) {
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
