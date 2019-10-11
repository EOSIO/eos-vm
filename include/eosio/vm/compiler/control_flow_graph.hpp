#pragma once

#include <unordered_map>

#include <eosio/vm/compiler/basic_block.hpp>
#include <eosio/vm/compiler/opcode_traits.hpp>
#include <eosio/vm/utils.hpp>
#include <eosio/vm/outcome.hpp>

namespace eosio { namespace vm {

   class control_flow_graph {
      public:
         struct node {
            std::vector<node*> predecessors;
            std::vector<node*> successors;
            basic_block*       block = nullptr;
         };

         control_flow_graph(const code_interval& ci) : _full_code_range(ci) {
            _blocks.reserve(_full_code_range.size());
            construct_blocks();
            _blocks.reserve(_blocks.size());
            _nodes.reserve(_blocks.size());
            resolve_edges();
         }

         const unmanaged_vector<basic_block>& get_blocks()const { return _blocks; }
         std::unordered_map<basic_block*, node>& get_nodes() { return _nodes; }
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
         outcome::result<basic_block*> search_for_opcode(std::size_t index) {
            std::size_t low = 0;
            std::size_t high = _blocks.size()-1;
            std::size_t mid = 0;
            while ( !(low==high && high==mid) ) {
               mid = low+((high-low)/2);
               if (_blocks[mid].get_interval().contains(_full_code_range.at(index)))
                  return &_blocks[mid];
               else if (index < _blocks[mid].get_interval().first() - _full_code_range.first())
                  high = mid-1;
               else if (index > _blocks[mid].get_interval().last() - _full_code_range.first()) {
                  low = mid+1;
               }
            }
            return nullptr;
         }
         void resolve_edges() {
            std::cout << "_block.size() " << _blocks.size() << "\n";
            for (std::size_t i=0; i < _blocks.size(); i++) {
               const auto& res = visit(overloaded { 
                     [&](const if_t& op) -> outcome::result<basic_block*> {
                        std::cout << "Found an if\n";
                        OUTCOME_TRY(bb, search_for_opcode(op.pc));
                        std::cout << "BB " << bb << "\n";
                        return bb;
                     },
                     [&](const else_t& op) -> outcome::result<basic_block*> {
                        std::cout << "Found an else\n";
                        OUTCOME_TRY(bb, search_for_opcode(op.pc));
                        std::cout << "BB " << bb << "\n";
                        return bb;
                     },
                     [&](const return_t& op) -> outcome::result<basic_block*>{
                        std::cout << "Found a return\n";
                        return compilation_errors::general_failure;
                     },
                     [&](const auto& op) -> outcome::result<basic_block*>{
                        std::cout << "Found another op\n";
                        return compilation_errors::general_failure;
                     } }, *(_blocks[i].get_terminator()));
               if (res) {
                  auto& succ1 = _nodes[&_blocks[i+1]];
                  auto& succ2 = _nodes[res.value()];
                  _nodes.insert_or_assign(&_blocks[i], node{{}, {&succ1, &succ2}, &_blocks[i]});
                  std::cout << "Size " << _nodes[&_blocks[i]].successors.size() << "\n";
               }   
            }
         }
         unmanaged_vector<basic_block> _blocks;
         code_interval _full_code_range;
         node* _start = nullptr;
         std::unordered_map<basic_block*, node> _nodes;
   };
}} // namespace eosio::vm
