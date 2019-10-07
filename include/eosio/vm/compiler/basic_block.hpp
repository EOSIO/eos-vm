#pragma once

#include <optional>
#include <limits>
#include <type_traits>

// TODO refactor out the helpers to utility header
#include <eosio/vm/host_function.hpp>
#include <eosio/vm/compiler/code_interval.hpp>


namespace eosio { namespace vm {
   class basic_block {
      enum class visitor_type : uint8_t {
         reentrant,
         stateless
      };

      template <visitor_type VT>
      class visitor {
         public:
            visitor(const code_interval& ci) : _interval(ci) {}
            template <typename Pred>
            opcode* operator()(Pred&& pred) {
               if constexpr (VT == visitor_type::stateless)
                  _visit_loc = _interval.begin();             
               for (; _visit_loc != _interval.end(); _visit_loc++) {
                  if (pred(*_visit_loc))
                     return *_visit_loc;
               }
               if constexpr (VT == visitor_type::reentrant)
                  _visit_loc = _interval.begin();
               return nullptr;
            }
         private:
            code_interval _interval;
            code_interval::iterator _visit_loc = _interval.begin();
      };

      public:
         basic_block(uint64_t parent, const code_interval& ci) : _interval(ci) {}
         template <visitor_type VT = visitor_type::stateless>
         visitor<VT> get_visitor() { return { _interval }; }
         opcode* get_terminator() { return _interval.last(); }

         std::size_t size()const { return _interval.size(); }
         code_interval get_interval()const { return _interval; }

         inline bool operator==(const basic_block& bb)const { return std::tie(_parent, _interval) == std::tie(bb._parent, bb._interval); }
         inline bool operator!=(const basic_block& bb)const { return !(*this == bb); }
      private:
         code_interval _interval;
         uint64_t      _parent;
         unmanaged_vector<basic_block*> _succs;
         unmanaged_vector<basic_block*> _preds;
   };

}} // namespace eosio::vm
