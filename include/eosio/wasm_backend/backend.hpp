#pragma once

#include <vector>
#include <fstream>
#include <optional>
#include <string_view>

#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/parser.hpp>
#include <eosio/wasm_backend/interpret_visitor.hpp>
#include <eosio/wasm_backend/execution_context.hpp>
#include <eosio/wasm_backend/types.hpp>

namespace eosio { namespace wasm_backend {
   class backend {
      public:
      backend(wasm_code& code, wasm_allocator& wa)
         : _walloc(wa),
           _galloc(1024 * 1024),
           _ctx( *this, binary_parser<backend>{*this}.parse_module( code ), wa ) {}

         template <typename... Args>
         inline std::optional<stack_elem> operator()(const std::string_view func, Args... args) {
            return _ctx.execute(interpret_visitor<backend>{*this, _ctx}, func, args...);
         }

         wasm_allocator& get_wasm_allocator() { return _walloc; }
         growable_allocator& get_allocator() { return _galloc; }

         static std::vector<uint8_t> read_wasm( const std::string& fname ) {
            std::ifstream wasm_file( fname, std::ios::binary );
            if (!wasm_file.is_open())
               throw std::runtime_error("wasm file not found");
            wasm_file.seekg( 0, std::ios::end );
            std::vector<uint8_t> wasm;
            int len = wasm_file.tellg();
            if ( len < 0 )
               throw std::runtime_error("wasm file length is -1");
            wasm.resize(len);
            wasm_file.seekg(0, std::ios::beg);
            wasm_file.read((char*)wasm.data(), wasm.size());
            wasm_file.close();
            return wasm;
         }

      private:
         wasm_allocator&    _walloc;
         growable_allocator _galloc;
         execution_context<backend> _ctx;
   };
}} // ns eosio::wasm_backend
