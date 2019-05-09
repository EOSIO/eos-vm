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

      backend(wasm_code& code) : _ctx( binary_parser{_mod.allocator}.parse_module( code, _mod ) ) {
      }

      template <typename... Args>
      inline bool operator()(Host* host, const std::string_view& mod,  const std::string_view& func, Args... args) {
         return call(host, mod, func, args...);
      }

      template <typename... Args>
      inline bool call(Host* host, const std::string_view& mod,  const std::string_view& func, Args... args) {
         #ifdef __EOSIO_DBG__
         _ctx.execute(host, debug_visitor(_ctx), func, args...);
         return true;
         #else
         _ctx.execute(host, interpret_visitor(_ctx), func, args...);
         return true;
         #endif
      }

      template <typename... Args>
      inline auto call_with_return(Host* host, const std::string_view& mod,  const std::string_view& func, Args... args) {
         return _ctx.execute(host, interpret_visitor(_ctx), func, args...);
      }

      inline void execute_all(Host* host) {
         for (int i=0; i < _mod.exports.size(); i++) {
            if (_mod.exports[i].kind == external_kind::Function) {
               std::string s{(const char*)_mod.exports[i].field_str.raw(), _mod.exports[i].field_str.size()};
               _ctx.execute(host, interpret_visitor(_ctx), s);
            }
         }
      }

      inline void immediate_exit() { _ctx.exit(); }
      inline void set_wasm_allocator( wasm_allocator* walloc ) {
         _walloc = walloc;
         _ctx.set_wasm_allocator( walloc );
      }

      inline wasm_allocator* get_wasm_allocator() { return _walloc; }
      inline module& get_module() { return _mod; }

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
      wasm_allocator*         _walloc = nullptr; // non owning pointer
      module                  _mod;
      execution_context<Host> _ctx;
   };
}} // ns eosio::wasm_backend
