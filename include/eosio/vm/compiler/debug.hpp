#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <eosio/vm/compiler/control_flow_graph.hpp>
#include <eosio/vm/opcodes.hpp>

#ifdef EOS_VM_DEBUG_COMPILER
#include <memory>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#endif

namespace eosio { namespace vm {
#ifdef EOS_VM_DEBUG_COMPILER
static constexpr bool compiler_debugging_enabled = true;
#else
static constexpr bool compiler_debugging_enabled = false;
#endif

   class cfg_writer {
      public:
         cfg_writer(const control_flow_graph& cfg) : _cfg(cfg) {}
         void write(const std::string& fname) {
            if constexpr (compiler_debugging_enabled) {
               using dot_graph = boost::adjacency_list<boost::vecS,
                                                       boost::vecS,
                                                       boost::directedS,
                                                       boost::property<boost::vertex_color_t, boost::default_color_type>,
                                                       boost::property<boost::edge_weight_t, int>>;
               std::vector<std::string> labels;
               const auto& bb = _cfg.get_blocks();
               auto& nodes = _cfg.get_nodes();
               using edge = std::pair<int, int>;
               std::vector<edge> edges;
               labels.push_back(generate_basic_block_label(0, bb[0]));
               const basic_block* start_block = &bb[0];
               for (int i=1; i < bb.size(); i++) {
                  labels.push_back(generate_basic_block_label(i, bb[i]));
                  auto& node = nodes[(basic_block*)&(bb[i])];
                  std::cout << "successors " << node.successors.size() << "\n";
                  for (int j=0; j < node.successors.size(); j++)
                     edges.emplace_back((int)(&bb[i]-start_block), (int)(node.successors[j]->block-start_block));
               }
               int* weights = new int[edges.size()];
               std::fill(weights, weights + edges.size(), 1); // all weights are the same
               try {
                  dot_graph g(edges.data(), edges.data() + edges.size(), weights, labels.size());
                  std::ofstream outfile( fname );
                  char** _labels = new char*[labels.size()];
                  for (size_t i = 0; i < labels.size(); i++)
                     _labels[i] = (char*)labels[i].c_str();
                  boost::write_graphviz( outfile, g, boost::make_label_writer(_labels) );
                  delete[] _labels;
               } catch (...) {}
               delete[] weights;
            }
         }
      private:
         std::string generate_basic_block_label(int number, const basic_block& bb)const {
            std::stringstream ss;
            ss << "BB" << number << "\\n";
            opcode_utils ou;
            int i = 0;
            for (auto iter = bb.get_interval().begin(); iter != bb.get_interval().end(); iter++) {
               ss << ou.opcode_map[(*iter)->index()] << "\\n";
            }
            return ss.str();
         }
         control_flow_graph _cfg;
   };
}} // namespace eosio::vm
