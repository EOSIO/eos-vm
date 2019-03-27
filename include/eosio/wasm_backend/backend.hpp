#pragma once

#include <vector>
#include <fstream>
#include <optional>
#include <string_view>

#include <eosio/wasm_backend/allocator.hpp>
#include <eosio/wasm_backend/parser.hpp>
#include <eosio/wasm_backend/interpret_visitor.hpp>
#include <eosio/wasm_backend/debug_visitor.hpp>
#include <eosio/wasm_backend/execution_context.hpp>
#include <eosio/wasm_backend/types.hpp>

#define __EOSIO_DBG__

namespace eosio { namespace wasm_backend {
   class backend {
      public:
      backend(wasm_code& code, wasm_allocator& wa)
         : _walloc(wa),
           _balloc(constants::max_code_size*2),
           _mod(*this),
           _ctx( *this, binary_parser<backend>{*this}.parse_module( code, _mod ), wa ) {
         //registered_host_functions::resolve(_mod);
      }

         template <typename... Args>
         inline std::optional<stack_elem> operator()(const std::string_view func, Args... args) {
            #ifdef __EOSIO_DBG__
               return _ctx.execute(debug_visitor<backend>{*this, _ctx}, func, args...);
            #else
               return _ctx.execute(interpret_visitor<backend>{*this, _ctx}, func, args...);
            #endif
         }

         inline void execute_all() {
            for (int i=0; i < _mod.exports.size(); i++) {
               if (_mod.exports[i].kind == external_kind::Function) {
                  std::string s{(const char*)_mod.exports[i].field_str.raw(), _mod.exports[i].field_len};
                  _ctx.execute(interpret_visitor<backend>{*this, _ctx}, s);
               }
            }
         }

         wasm_allocator& get_wasm_allocator() { return _walloc; }
         bounded_allocator& get_allocator() { return _balloc; }
         module<backend>& get_module() { return _mod; }

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
      inline size_t get_instructions()const { return _ctx.insts; }
      private:
         wasm_allocator&    _walloc;
         bounded_allocator  _balloc;
         module<backend>    _mod;
         execution_context<backend> _ctx;
   };
}} // ns eosio::wasm_backend
