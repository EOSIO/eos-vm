#include "picojson.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "eosio_test_generator.hpp"

using namespace std;

map<string, bool> blacklist_memory_clearing = {
   { "address.0", true },      { "address.2", true },      { "address.3", true },      { "address.4", true },
   { "float_exprs.59", true }, { "float_exprs.60", true }, { "float_memory.0", true }, { "float_memory.1", true },
   { "float_memory.2", true }, { "float_memory.3", true }, { "float_memory.4", true }, { "float_memory.5", true },
   { "memory.25", true },      { "memory_trap.1", true },  { "start.3", true },        { "start.4", true },
};

map<string, bool> whitelist_force_check_throw = {
   { "memory.6", true },
   { "memory.7", true },
};

const string include_eosio = "#include <eosio/eosio.hpp>\n\n";
const string extern_c      = "extern \"C\" {\n";
const string apply_func    = "   void apply(uint64_t, uint64_t, uint64_t test_to_run) {\n";
const string mem_clear     = "      volatile uint64_t* r = (uint64_t*)0;\n      *r = 0;\n";

// NOTE: Changing this will likely break one or more tests.
const int NUM_TESTS_PER_SUB_APPLY = 100;

map<string, bool> func_already_written;
map<string, int>  func_name_to_index;
int               func_index = 0;

void write_file(ofstream& file, string test_name, string funcs, string sub_applies, string apply) {
   stringstream out;
   string       end_brace = "}";

   out << include_eosio;
   out << extern_c;
   out << funcs;
   out << sub_applies;
   out << apply_func;
   if (!blacklist_memory_clearing.count(test_name)) {
      out << mem_clear;
   }
   out << apply;
   out << "   }\n";
   out << "}\n";

   file << out.str();
   out.str("");
}

void write_map_file(ofstream& file) {
   stringstream out;

   out << "{\n";
   auto last = prev(func_name_to_index.end(), 1);
   for (auto it = func_name_to_index.begin(); it != last; ++it) {
      out << "  \"" << it->first << "\""
          << " : " << it->second << ","
          << "\n";
   }
   auto it = func_name_to_index.rbegin();
   out << "  \"" << it->first << "\""
       << " : " << it->second << "\n";
   out << "}\n";

   file << out.str();
   out.str("");
}

string c_type(string wasm_type) {
   string c_type = "";
   if (wasm_type == "i32") {
      c_type = "int32_t";
   } else if (wasm_type == "i64") {
      c_type = "int64_t";
   } else if (wasm_type == "f32") {
      c_type = "float";
   } else if (wasm_type == "f64") {
      c_type = "double";
   } else {
      c_type = "void";
   }

   return c_type;
}

bool check_exists(string function_name, vector<tuple<string, string>> params, tuple<string, string> expected_return) {
   stringstream func_hash;
   func_hash << function_name;
   for (auto p = params.begin(); p != params.end(); p++) {
      auto [type, _] = *p;
      func_hash << '-' << type;
   }
   auto [return_type, _] = expected_return;
   func_hash << '-' << return_type;

   if (func_already_written.find(func_hash.str()) == func_already_written.end()) {
      func_already_written[func_hash.str()] = true;
      return false;
   } else {
      return true;
   }
}

vector<tuple<string, string>> get_params(picojson::object action) {
   vector<tuple<string, string>> params = {};

   auto args = action["args"].get<picojson::array>();
   for (auto a : args) {
      auto arg = a.get<picojson::object>();

      string type  = arg["type"].to_str();
      string value = arg["value"].to_str();
      params.push_back(make_tuple(type, value));
   }

   return params;
}

tuple<string, string> get_expected_return(picojson::object test) {
   tuple<string, string> expected_return;

   auto expecteds = test["expected"].get<picojson::array>();
   for (auto e : expecteds) {
      auto   expect   = e.get<picojson::object>();
      string type     = expect["type"].to_str();
      string value    = expect["value"].to_str();
      expected_return = make_tuple(type, value);
   }

   return expected_return;
}

string write_test_function(string function_name, picojson::object test) {
   stringstream out;

   auto action = test["action"].get<picojson::object>();

   vector<tuple<string, string>> params          = get_params(action);
   tuple<string, string>         expected_return = get_expected_return(test);

   if (check_exists(function_name, params, expected_return)) {
      return "";
   }
   func_name_to_index[function_name] = func_index;
   ++func_index;

   auto [return_type, return_val] = expected_return;
   out << "   " << c_type(return_type) << " ";
   out << function_name;
   out << "(";

   if (params.size() > 0) {
      for (auto p = params.begin(); p != params.end() - 1; p++) {
         auto [type, _] = *p;
         out << c_type(type) << ", ";
      }

      auto [type, _] = *(params.end() - 1);
      out << c_type(type);
   }

   out << ") {\n";

   if (return_type == "i32" || return_type == "i64") {
      out << "   "
          << "   return 0;";
   } else if (return_type == "f32" || return_type == "f64") {
      out << "   "
          << "   return 0.0f;";
   } else {
      out << "   "
          << "   return;";
   }

   out << "\n   }\n\n";
   return out.str();
}

string write_test_function_call(string function_name, picojson::object test, int var_index) {
   stringstream out;
   stringstream func_call;
   string       return_cast = "";

   auto                          action          = test["action"].get<picojson::object>();
   vector<tuple<string, string>> params          = get_params(action);
   tuple<string, string>         expected_return = get_expected_return(test);

   auto [return_type, return_val] = expected_return;

   int param_index_offset = var_index;
   if (params.size() > 0) {
      int param_index = 0;
      for (auto p = params.begin(); p != params.end(); p++) {
         auto [type, value] = *p;
         if (type == "f32") {
            out << "      "
                << "int32_t "
                << "y" << param_index << param_index_offset << " = " << value << ";\n";
            param_index++;
         } else if (type == "f64") {
            out << "      "
                << "int64_t "
                << "y" << param_index << param_index_offset << " = " << value << ";\n";
            param_index++;
         }
      }
   }

   bool needs_local_return = false;
   if (return_val != "null") {
      needs_local_return = c_type(return_type) != "void";
   }

   // if return_type is float or double,
   // generate local var and reinterpret cast.
   if (needs_local_return) {
      if (return_type == "f32") {
         return_cast = "*(uint32_t*)&";
      } else if (return_type == "f64") {
         return_cast = "*(uint64_t*)&";
      }

      func_call << "   "
                << "   " << c_type(return_type) << " "
                << "x" << var_index << " = ";
   }

   func_call << function_name << "(";

   if (params.size() > 0) {
      int param_index = 0;
      for (auto p = params.begin(); p != params.end() - 1; p++) {
         auto [type, value] = *p;
         if (type == "f32") {
            func_call << "*(float*)&"
                      << "y" << param_index << param_index_offset << ", ";
            param_index++;
         } else if (type == "f64") {
            func_call << "*(double*)&"
                      << "y" << param_index << param_index_offset << ", ";
            param_index++;
         } else {
            func_call << "(" << c_type(type) << ") " << value << ", ";
         }
      }

      auto [type, value] = *(params.end() - 1);
      if (type == "f32") {
         func_call << "*(float*)&"
                   << "y" << param_index << param_index_offset;
      } else if (type == "f64") {
         func_call << "*(double*)&"
                   << "y" << param_index << param_index_offset;
      } else {
         func_call << "(" << c_type(type) << ")" << value;
      }
   }
   func_call << ");";

   if (needs_local_return) {
      out << func_call.str() << "\n";
   }

   if (return_val != "" && return_val != "null") {
      out << "   "
          << "   "
          << "eosio::check(";
      if (needs_local_return) {
         out << return_cast << "x" << var_index;
      } else {
         out << func_call.str();
      }
      out << " == ";
      out << "(" << c_type(return_type) << ")" << return_val;
      out << ", "
          << "\"" << function_name << " fail " << var_index << "\"";
      out << ");\n\n";
   } else {
      // If there's no expected return, just call the function to prove it doesn't blow up.
      out << "   "
          << "   " << func_call.str() << "\n\n";
   }
   return out.str();
}

void usage(const char* name) {
   std::cerr << "Usage:\n"
             << "  " << name << " [json file created by wast2json]\n";
   std::exit(2);
}

map<string, vector<picojson::object>> get_file_func_mappings(picojson::value v) {
   map<string, vector<picojson::object>> file_func_mappings;
   const auto&                           o = v.get<picojson::object>();

   string filename = "";
   for (auto i = o.begin(); i != o.end(); i++) {
      if (i->first == "commands") {
         for (const auto& o : i->second.get<picojson::array>()) {
            auto obj = o.get<picojson::object>();
            if (obj["type"].to_str() == "module") {
               filename = obj["filename"].to_str();
            }
            if (obj["type"].to_str() == "assert_return" || obj["type"].to_str() == "action" ||
                obj["type"].to_str() == "assert_exhaustion" || obj["type"].to_str() == "assert_return_canonical_nan" ||
                obj["type"].to_str() == "assert_return_arithmetic_nan" || obj["type"].to_str() == "assert_exhaustion" ||
                obj["type"].to_str() == "assert_trap") {
               file_func_mappings[filename].push_back(obj);
            }
         }
      }
   }

   return file_func_mappings;
}

string create_sub_apply(string func_calls, string name) {
   stringstream ss;
   ss << "   void " << name << "() {\n";
   ss << func_calls;
   ss << "   }\n";
   return ss.str();
}

int main(int argc, char** argv) {
   ifstream     ifs;
   stringstream ss;
   if (argc != 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
      usage(argc ? argv[0] : "eosio_test_generator");
   }
   ifs.open(argv[1]);
   if (!ifs) {
      std::cerr << "Cannot open file: " << argv[1] << std::endl;
      return EXIT_FAILURE;
   }
   string s;
   while (getline(ifs, s)) { ss << s; }
   ifs.close();

   picojson::value v;
   picojson::parse(v, ss.str());
   string test_suite_name;

   map<string, vector<picojson::object>> file_func_mappings = get_file_func_mappings(v);
   map<string, map<int, string>>         test_mappings;

   vector<spec_test> spec_tests;

   for (const auto& f : file_func_mappings) {
      ofstream ofs_cpp;
      ofstream ofs_map;

      stringstream test_funcs;
      stringstream sub_apply_funcs;
      stringstream apply_func;

      vector<string> sub_applies;

      int    pos          = f.first.find_last_of('.');
      string test_name    = f.first.substr(0, pos);
      string out_file_cpp = test_name + ".wasm.cpp";
      string out_file_map = test_name + ".wasm.map";

      ofs_cpp.open(out_file_cpp, ofstream::out);
      ofs_map.open(out_file_map, ofstream::out);

      func_already_written.clear();
      func_name_to_index.clear();

      vector<picojson::object> assert_trap_tests;
      vector<picojson::object> assert_return_tests;

      func_index = 0;
      for (auto test : f.second) {
         string type_test     = test["type"].to_str();
         auto   action        = test["action"].get<picojson::object>();
         string function_name = action["field"].to_str();
         function_name        = "_" + normalize(function_name);

         test_funcs << write_test_function(function_name, test);
         int is_whitelisted = whitelist_force_check_throw.count(test_name);
         if (type_test == "assert_trap" || type_test == "assert_exhaustion" || is_whitelisted) {
            assert_trap_tests.push_back(test);
         } else {
            assert_return_tests.push_back(test);
         }
      }

      int sub_apply_index = 0;
      for (auto test : assert_trap_tests) {
         auto   action        = test["action"].get<picojson::object>();
         string function_name = action["field"].to_str();
         function_name        = "_" + normalize(function_name);

         string name = "sub_apply_" + to_string(sub_apply_index);
         sub_applies.push_back(name);
         string func_call = write_test_function_call(function_name, test, sub_apply_index);
         string sub_apply = create_sub_apply(func_call, name);

         sub_apply_funcs << sub_apply;
         test_mappings[test_name].insert(std::make_pair(sub_apply_index, "assert_trap"));
         ++sub_apply_index;
      }

      int assert_trap_end_index = sub_apply_index;

      if (assert_return_tests.size() < 1 ) {
         cout << test_name << " -- NO ASSERT_RETURN" << endl;
      }

      stringstream ss;
      int          i            = 1;
      int          length_tests = assert_return_tests.size();
      for (auto test : assert_return_tests) {
         auto   action        = test["action"].get<picojson::object>();
         string function_name = action["field"].to_str();
         function_name        = "_" + normalize(function_name);

         if (i % NUM_TESTS_PER_SUB_APPLY == 0 || i == length_tests) {
            string name = "sub_apply_" + to_string(sub_apply_index);
            sub_applies.push_back(name);

            ss << write_test_function_call(function_name, test, i);
            sub_apply_funcs << create_sub_apply(ss.str(), name);
            ss.str("");
            test_mappings[test_name].insert(std::make_pair(sub_apply_index, ""));
            ++sub_apply_index;
         } else {
            ss << write_test_function_call(function_name, test, i);
         }
         ++i;
      }

      spec_tests.push_back(spec_test{test_name, 0, assert_trap_end_index, assert_trap_end_index, sub_apply_index});

      int index = 0;
      apply_func << "      switch(test_to_run) {\n";
      for (auto sub_apply : sub_applies) {
         apply_func << "         case " << index << ":\n";
         apply_func << "            " << sub_apply << "();"
                    << "\n";
         apply_func << "            break;\n";
         ++index;
      }
      apply_func << "      }\n";

      write_file(ofs_cpp, test_name, test_funcs.str(), sub_apply_funcs.str(), apply_func.str());
      write_map_file(ofs_map);
   }

   write_tests(spec_tests);
}
