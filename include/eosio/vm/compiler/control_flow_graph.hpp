#pragma once

#include <eosio/vm/compiler/basic_block.hpp>
#include <eosio/vm/compiler/opcode_traits.hpp>

namespace eosio { namespace vm {

   class control_flow_graph {
      public:
         struct node {
            std::vector<node*> predecessors;
            std::vector<node*> successors;
         };

         control_flow_graph(const code_interval& ci) : _full_code_range(ci) {
            construct_blocks();
            resolve_edges();
         }

         unmanaged_vector<basic_block> get_blocks()const { return _blocks; }
      private:
         inline bool should_end(const opcode* op) {
            return op->is_a<end_t>() || op->is_a<return_t>() || opcode_traits::is_a_control(op);
         }
         void construct_blocks() {
            auto last_starting_op = _full_code_range.begin();
            auto iter = last_starting_op;
            for (; iter != _full_code_range.end(); iter++) {
               // clean this up
               if (should_end(*iter)) {
                  _blocks.emplace_back( 0, code_interval{*last_starting_op, *iter} );
                  last_starting_op = iter;
                  last_starting_op++;
               }
            }

            _blocks.emplace_back( 0, code_interval{*last_starting_op, *(--iter)} ); // undo the increment to the iterator from the end
         }
         void resolve_edges() {
         }
         unmanaged_vector<basic_block> _blocks;
         code_interval _full_code_range;
   };
}} // namespace eosio::vm
