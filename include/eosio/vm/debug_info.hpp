#pragma once

#include <utility>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <algorithm>

namespace eosio::vm {

struct null_debug_info {
   using builder = null_debug_info;
   void on_code_start(const void* compiled_base, const void* wasm_code_start) {}
   void on_function_start(const void* code_addr, const void* wasm_addr) {}
   void on_instr_start(const void* code_addr, const void* wasm_addr) {}
   void on_code_end(const void* code_addr, const void* wasm_addr) {}
   void set(const null_debug_info&) {}
   void relocate(const void*) {}
};

// Maps a contiguous region of code to offsets onto the code section of the original wasm.
class profile_instr_map {
   struct addr_entry {
      uint32_t offset;
      uint32_t wasm_addr;
   };

public:

   struct builder {
      void on_code_start(const void* compiled_base, const void* wasm_code_start) {
         code_base = compiled_base;
         wasm_base = wasm_code_start;
      }
      void on_function_start(const void* code_addr, const void* wasm_addr) {
         data.push_back({
            static_cast<std::uint32_t>(reinterpret_cast<const char*>(code_addr) - reinterpret_cast<const char*>(code_base)),
            static_cast<std::uint32_t>(reinterpret_cast<const char*>(wasm_addr) - reinterpret_cast<const char*>(wasm_base))
         });
      }
      void on_instr_start(const void* code_addr, const void* wasm_addr) {
         data.push_back({
            static_cast<std::uint32_t>(reinterpret_cast<const char*>(code_addr) - reinterpret_cast<const char*>(code_base)),
            static_cast<std::uint32_t>(reinterpret_cast<const char*>(wasm_addr) - reinterpret_cast<const char*>(wasm_base))
         });
      }
      void on_code_end(const void* code_addr, const void* wasm_addr) {
         code_end = code_addr;
      }

      const void* code_base = nullptr;
      const void* wasm_base = nullptr;
      const void* code_end = nullptr;
      std::vector<addr_entry> data;
   };

   void set(builder&& b) {
      data = std::move(b.data);
      std::sort(data.begin(), data.end(), [](const addr_entry& lhs, const addr_entry& rhs){ return lhs.offset < rhs.offset; });
      base_address = b.code_base;
      code_size = reinterpret_cast<const char*>(b.code_end) - reinterpret_cast<const char*>(base_address);
      offset_to_addr = data.data();
      offset_to_addr_len = data.size();
   }

   profile_instr_map() = default;
   profile_instr_map(const profile_instr_map&) = delete;
   profile_instr_map& operator=(const profile_instr_map&) = delete;

   // Indicate that the executable code was moved/copied/mmapped/etc to another location
   void relocate(const void* new_base) { base_address = new_base; }

   // Cannot use most of the standard library as the STL is not async-signal-safe
   std::uint32_t translate(const void* pc) const {
      std::size_t diff = (reinterpret_cast<const char*>(pc) - reinterpret_cast<const char*>(base_address)); // negative values wrap
      if(diff >= code_size || diff < offset_to_addr[0].offset) return 0xFFFFFFFFu;
      std::uint32_t offset = diff;

      // Loop invariant: offset_to_addr[lower].offset <= offset < offset_to_addr[upper].offset
      std::size_t lower = 0, upper = offset_to_addr_len;
      while(upper - lower > 1) {
         std::size_t mid = lower + (upper - lower) / 2;
         if(offset_to_addr[mid].offset <= offset) {
            lower = mid;
         } else {
            upper = mid;
         }
      }

      return offset_to_addr[lower].wasm_addr;
   }
private:
   const void* base_address = nullptr;
   std::size_t code_size = 0;

   addr_entry* offset_to_addr = nullptr;
   std::size_t offset_to_addr_len = 0;

   std::vector<addr_entry> data;
};

}
