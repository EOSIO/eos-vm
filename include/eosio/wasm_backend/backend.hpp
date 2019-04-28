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
   template <typename Host>
   class backend {
      public:
      using host_t = Host;
      backend(wasm_code& code)
         : _balloc(constants::initial_module_size),
           _mod(*this),
           _ctx( *this, binary_parser<backend>{*this}.parse_module( code, _mod ) ) {
      }

         template <typename... Args>
         inline bool operator()(Host* host, const std::string_view& mod,  const std::string_view& func, Args... args) {
            #ifdef __EOSIO_DBG__
            _ctx.execute(host, debug_visitor<backend>{*this, _ctx}, func, args...);
            return true;
            #else
            _ctx.execute(host, interpret_visitor<backend>{*this, _ctx}, func, args...);
            return true;
            #endif
         }

         inline void execute_all(Host* host) {
            for (int i=0; i < _mod.exports.size(); i++) {
               if (_mod.exports[i].kind == external_kind::Function) {
                  std::string s{(const char*)_mod.exports[i].field_str.raw(), _mod.exports[i].field_len};
                  _ctx.execute(host, interpret_visitor<backend>{*this, _ctx}, s);
               }
            }
         }

         inline void immediate_exit() { _ctx.exit(); }
         inline void set_wasm_allocator( wasm_allocator* walloc ) {
            _walloc = walloc;
            _ctx.set_wasm_allocator( walloc );
         }
         inline wasm_allocator* get_wasm_allocator() { return _walloc; }

         inline bounded_allocator& get_allocator() { return _balloc; }
         inline module<backend>& get_module() { return _mod; }

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
         wasm_allocator*    _walloc; // non owning pointer
         bounded_allocator  _balloc;
         module<backend>    _mod;
         execution_context<backend> _ctx;
   };
}} // ns eosio::wasm_backend
