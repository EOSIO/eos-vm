#include <eosio/vm/backend.hpp>
#include <iostream>
#include <iomanip>
#include <cctype>

struct nm_debug_info {
   using builder = nm_debug_info;
   void on_code_start(const void* compiled_base, const void* wasm_code_start) {
         wasm_base = wasm_code_start;
   }
   void on_function_start(const void* code_addr, const void* wasm_addr) {
      function_offsets.push_back(static_cast<std::uint32_t>(reinterpret_cast<const char*>(wasm_addr) - reinterpret_cast<const char*>(wasm_base)));
   }
   void on_instr_start(const void* code_addr, const void* wasm_addr) {}
   void on_code_end(const void* code_addr, const void* wasm_addr) {}
   void set(nm_debug_info&& other) { *this = std::move(other); }
   void relocate(const void*) {}

   uint32_t get_function(std::uint32_t addr) {
      auto pos = std::lower_bound(function_offsets.begin(), function_offsets.end(), addr + 1);
      if(pos == function_offsets.begin()) return 0;
      return (pos - function_offsets.begin()) - 1;
   }

   const void* wasm_base;
   std::vector<uint32_t> function_offsets;
};

eosio::vm::guarded_vector<uint8_t>* find_export_name(eosio::vm::module& mod, uint32_t idx) {
   if(mod.names && mod.names->function_names) {
      for(uint32_t i = 0; i < mod.names->function_names->size(); ++i) {
         if((*mod.names->function_names)[i].idx == idx) {
            return &(*mod.names->function_names)[i].name;
         }
      }
   }
   for(uint32_t i = 0; i < mod.exports.size(); ++i) {
      if(mod.exports[i].index == idx && mod.exports[i].kind == eosio::vm::Function) {
         return &mod.exports[i].field_str;
      }
   }
   return nullptr;
}

struct nm_options {
   static constexpr bool parse_custom_section_name = true;
};

bool starts_with(std::string_view arg, std::string_view prefix) {
   return arg.substr(0, prefix.size()) == prefix;
}

int main(int argc, const char** argv) {
   bool print_addresses = false;
   bool print_functions = false;
   bool pretty = false;
   std::vector<std::string> addresses;
   std::string filename = "a.out";
   for(int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if(arg == "-h" || arg == "--help") {
        std::cerr << "Usage: " << argv[0] << " [-h] [-a] [-C] [-e wasm] [-f] [-s] [-i] [-p] address..." << std::endl;
          return 0;
      } else if(arg == "-a" || arg == "--addresses") {
         print_addresses = true;
      } else if(arg == "-C" || starts_with(arg, "--demangle=")) {
         // Ignore requests to demangle
      } else if(arg == "-e") {
         if(++i < argc) {
            filename = argv[i];
         } else {
            std::cerr << "Missing argument to -e" << std::endl;
            return 2;
         }
      } else if(starts_with(arg, "--exe=")) {
         filename = arg.substr(6);
      } else if(arg == "-f" || arg == "--functions") {
         print_functions = true;
      } else if(arg == "-s" || arg == "--basenames") {
         // No effect without filenames
      } else if(arg == "-i" || arg == "--inlines") {
         // No effect without source information
      } else if(arg == "-p" || arg == "--pretty-print") {
         pretty = true;
      } else if(arg[0] != '-') {
         addresses.push_back(arg);
      } else {
         std::cerr << "unexpected argument: " << arg << std::endl;
         return 2;
      }
   }

   std::vector<std::string> function_names;

   using namespace eosio::vm;
   auto code = read_wasm(filename);
   nm_debug_info info;
   module mod;
   binary_parser<null_writer, nm_options, nm_debug_info> parser(mod.allocator);
   parser.parse_module(code, mod, info);
   {
      for(std::size_t i = 0; i < info.function_offsets.size(); ++i) {
         if(guarded_vector<uint8_t>* name = find_export_name(mod, i + mod.get_imported_functions_size())) {
            function_names.push_back(std::string(reinterpret_cast<const char*>(name->raw()), name->size()));
         } else {
            function_names.push_back("fn" + std::to_string( i + mod.get_imported_functions_size()));
         }
      }
   }

   std::string_view sep = pretty ? " " : "\n";

   auto print_one_address = [&](const std::string& addr) {
      unsigned x = std::stoul(addr, nullptr, 16);
      if(print_addresses) {
         std::cout << std::showbase << std::hex << x << sep;
      }
      if(print_functions) {
         std::cout << std::hex << function_names[info.get_function(x)] << sep;
      }
      std::cout << "??:0\n";
   };

   if(!addresses.empty()) {
      for(const auto& addr : addresses) {
         print_one_address(addr);
      }
   } else {
      std::string addr;
      while(std::cin >> addr) {
         print_one_address(addr);
      }
   }
}
