#include <eosio/vm/backend.hpp>
#include <iostream>
#include <iomanip>

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

   const void* wasm_base;
   std::vector<uint32_t> function_offsets;
};

enum class nm_sort_order {
   alphabetic, numeric, none
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

int main(int argc, const char** argv) {
   bool print_file_name = false;
   nm_sort_order sort = nm_sort_order::alphabetic;
   std::vector<std::string> filenames;
   for(int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if(std::strcmp(argv[i], "-A") == 0 || std::strcmp(argv[i], "-o") == 0 || std::strcmp(argv[i], "--print-file-name") == 0) {
         print_file_name = true;
      } else if(std::strcmp(argv[i], "-a") == 0 || std::strcmp(argv[i], "--debug-syms") == 0) {
         // Ignored because we don't know how to parse debug symbols
      } else if(std::strcmp(argv[i], "-C") == 0 || std::strncmp(argv[i], "--demangle=", 11) == 0) {
         // Demangling not implemented
      } else if(std::strcmp(argv[i], "--no-demangle") == 0) {
         // Default
      } else if(std::strcmp(argv[i], "-D") == 0 || std::strcmp(argv[i], "--dynamic") == 0) {
         // Not a dynamic object
      } else if(arg == "-n" || arg == "-v" || arg == "--numeric-sort") {
         sort = nm_sort_order::numeric;
      } else if(arg[0] != '-') {
        filenames.push_back(arg);
      } else {
         std::cerr << "unexpected argument: " << arg << std::endl;
         return 2;
      }
   }

   for(const std::string& filename : filenames) {
      using namespace eosio::vm;
      auto code = read_wasm(filename);
      nm_debug_info info;
      module mod;
      binary_parser<null_writer, nm_options, nm_debug_info> parser(mod.allocator);
      parser.parse_module(code, mod, info);
      for(std::size_t i = 0; i < info.function_offsets.size(); ++i) {
         std::cout << std::hex << std::setw(8) << std::setfill('0') << info.function_offsets[i] << " T ";
         if(guarded_vector<uint8_t>* name = find_export_name(mod, i + mod.get_imported_functions_size())) {
            std::cout << std::string_view(reinterpret_cast<const char*>(name->raw()), name->size());
         } else {
            std::cout << "fn" << i + mod.get_imported_functions_size();
         }
         std::cout << std::endl;
      }
   }
}
