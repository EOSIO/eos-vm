#pragma once

#include <eosio/vm/compiler/basic_block.hpp>

namespace eosio { namespace vm {

   class control_flow_graph {
      public:
         control_flow_graph(const code_interval& ci) : _full_code_range(ci) {
         }
      private:
         void construct_blocks() {
            opcode* last_starting_op = _full_code_range.begin();
            for (auto iter = last_starting_op; iter != _full_code_range; iter++) {
               // clean this up
                
            }
         }
         unmanaged_vector<basic_block> _blocks;
         code_interval _full_code_range;
   };
}} // namespace eosio::vm
