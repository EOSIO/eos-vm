#include "picojson.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const string include_eosio = "#include <eosio/eosio.hpp>\n\n";
const string extern_c =  "extern \"C\" {\n";
const string apply_func = "   void apply(uint64_t, uint64_t, uint64_t) {\n";

map<string, bool> func_already_written;

void write_file(ofstream& file, string funcs, string func_calls) {
   stringstream out;
   string end_brace = "}";

   out << include_eosio;
   out << extern_c;
   out << funcs;
   out << apply_func;
   out << func_calls;
   out << "   }\n";
   out << "}\n";

   file << out.str();
   out.str("");
}

string normalize(string val) {
   string ret_val = val;
   for (int i = 0; i <= val.size(); i++) {
      ret_val[i] = val[i] == '-' || val[i] == '.' ? '_' : val[i];
   }

   return ret_val;
}

string c_type(string wasm_type) {
   string c_type = "";
   if (wasm_type == "i32") {
      c_type = "int32_t";
   }
   else if (wasm_type == "i64") {
      c_type = "int64_t";
   }
   else if (wasm_type == "f32") {
      c_type = "float";
   }
   else if (wasm_type == "f64") {
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

string write_function(string function_name, vector<tuple<string, string>> params, tuple<string, string> expected_return) {
   stringstream out;

   if (check_exists(function_name, params, expected_return)) {
      return "";
   }

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
      out << "   " << "   return 0;";
   }
   else if (return_type == "f32" || return_type == "f64") {
      out << "   " << "   return 0.0f;";
   } else {
      out << "   " << "   return;";
   }

   out << "\n   }\n\n";
   return out.str();
}

string write_function_call(string function_name, vector<tuple<string, string>> params, tuple<string, string> expected_return,
                           int var_index, int param_index_offset) {
   stringstream out;
   auto [return_type, return_val] = expected_return;

   string return_cast = "";
   stringstream func_call;

   if (params.size() > 0) {
      int param_index = 0;
      for (auto p = params.begin(); p != params.end(); p++) {
         auto [type, value] = *p;
         if (type == "f32") {
            out << "      " << "int32_t " << "y" << param_index << param_index_offset << " = " << value << ";\n";
            param_index++;
         }
         else if (type == "f64") {
            out << "      " << "int64_t " << "y" << param_index << param_index_offset << " = " << value << ";\n";
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

      func_call << "   " << "   " << c_type(return_type) << " " << "x" << var_index << " = ";
   }

   func_call << function_name << "(";


   if (params.size() > 0) {
      int param_index = 0;
      for (auto p = params.begin(); p != params.end() - 1; p++) {
         auto [type, value] = *p;
         if (type == "f32") {
            func_call << "*(float*)&" << "y" << param_index << param_index_offset << ", ";
            param_index++;
         }
         else if (type == "f64") {
            func_call << "*(double*)&" << "y" << param_index << param_index_offset << ", ";
            param_index++;
         } else {
            func_call << "(" << c_type(type) << ") " << value << ", ";
         }
      }

      auto [type, value] = *(params.end() - 1);
      if (type == "f32") {
         func_call << "*(float*)&" << "y" << param_index << param_index_offset;
      }
      else if (type == "f64") {
         func_call << "*(double*)&" << "y" << param_index << param_index_offset;
      }
      else {
         func_call << value;
      }
   }
   func_call << ");";

   if (needs_local_return) {
      out << func_call.str() << "\n";
   }

   if (return_val != "" && return_val != "null") {
      out << "   " << "   " << "eosio::check(";
      if (needs_local_return) {
         out << return_cast << "x" << var_index;
      } else {
         out << func_call.str();
      }
      out << " == ";
      out << "(" << c_type(return_type) << ")" << return_val;
      out << ", " << "\"" << function_name << " fail\"";
      out << ");\n\n";
   } else {
      // If there's no expected return, just call the function to prove it doesn't blow up.
      out << "   " << "   " << func_call.str() << "\n\n";
   }
   return out.str();
}

string generate_functions(picojson::object test, bool call, int var_index) {
   vector<tuple<string, string>> params = {};
   tuple<string, string> expected_return;

   auto action_obj = test["action"].get<picojson::object>();
   string function_name = action_obj["field"].to_str();
   function_name = "_" + normalize(function_name);

   auto args = action_obj["args"].get<picojson::array>();
   for (auto a : args) {
      auto arg = a.get<picojson::object>();

      string type = arg["type"].to_str();
      string value = arg["value"].to_str();
      params.push_back(make_tuple(type, value));
   }

   auto expecteds = test["expected"].get<picojson::array>();
   for (auto e: expecteds) {
      auto expect = e.get<picojson::object>();
      string type = expect["type"].to_str();
      string value = expect["value"].to_str();
      expected_return = make_tuple(type, value);
   }

   if (call) {
      return write_function_call(function_name, params, expected_return, var_index, var_index);
   } else {
      return write_function(function_name, params, expected_return);
   }
}

void usage(const char* name) {
   std::cerr << "Usage:\n"
             << "  " << name << " [json file created by wast2json]\n";
   std::exit(2);
}

int main(int argc, char** argv) {
   ifstream     ifs;
   stringstream ss;
   if(argc != 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
      usage(argc?argv[0]:"eosio_test_generator");
   }
   ifs.open(argv[1]);
   if(!ifs) {
      std::cerr << "Cannot open file: " << argv[1] << std::endl;
      return EXIT_FAILURE;
   }
   string s;
   while (getline(ifs, s)) {
      ss << s;
   }
   ifs.close();

   picojson::value v;
   picojson::parse(v, ss.str());
   string test_suite_name;

   map<string, vector<picojson::object>> file_func_mappings;
   const auto& o = v.get<picojson::object>();

   string filename = "";
   for (auto i = o.begin(); i != o.end(); i++) {
      if (i->first == "commands") {
         for (const auto& o : i->second.get<picojson::array>()) {
            auto obj = o.get<picojson::object>();
            if (obj["type"].to_str() == "module") {
               filename = obj["filename"].to_str();
            }
            if (obj["type"].to_str() == "assert_return" ||
                obj["type"].to_str() == "action" ||
                obj["type"].to_str() == "assert_exhaustion" ||
                obj["type"].to_str() == "assert_return_canonical_nan" ||
                obj["type"].to_str() == "assert_return_arithmetic_nan" ||
                obj["type"].to_str() == "assert_exhaustion" ||
                obj["type"].to_str() == "assert_trap") {
               file_func_mappings[filename].push_back(obj);
            }
         }
      }
   }

   for (const auto& f : file_func_mappings) {
      ofstream ofs;
      stringstream funcs;
      stringstream func_calls;

      int pos = f.first.find_last_of('.');
      string out_file = f.first.substr(0, pos) + ".cpp";

      ofs.open(out_file, ofstream::out);

      func_already_written.clear();
      int var_index = 0;
      for (const auto& ff : f.second) {
         funcs << generate_functions(ff, false, var_index);
         func_calls << generate_functions(ff, true, var_index);
         var_index++;
      }

      write_file(ofs, funcs.str(), func_calls.str());
   }
}
