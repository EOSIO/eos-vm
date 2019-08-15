#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "eosio_test_generator.hpp"

using namespace std;

const string test_includes = "#include <wasm_spec_tests.hpp>\n";
const string end_test      = "BOOST_AUTO_TEST_SUITE_END()\n";

string normalize(string val) {
   string ret_val = val;
   for (int i = 0; i <= val.size(); i++) {
      if (val[i] == '-' || val[i] == '.') {
         ret_val[i] = '_';
      } else {
         ret_val[i] = val[i];
      }
   }

   return ret_val;
}

string create_push_action() {
   stringstream pa;

   pa << "void push_action(TESTER& tester, action&& act, uint64_t authorizer) {";
   pa << "   signed_transaction trx;\n";
   pa << "   if (authorizer) {\n";
   pa << "      act.authorization = vector<permission_level>{{authorizer, config::active_name}};\n";
   pa << "   }\n";
   pa << "   trx.actions.emplace_back(std::move(act));\n";
   pa << "   tester.set_transaction_headers(trx);\n";
   pa << "   if (authorizer) {\n";
   pa << "      trx.sign(tester.get_private_key(authorizer, \"active\"), tester.control->get_chain_id());\n";
   pa << "   }\n";
   pa << "   tester.push_transaction(trx);\n";
   pa << "   tester.produce_block();\n";
   pa << "}\n\n";

   return pa.str();
}

string create_pass_test_function(string file_name, string test_name, int test_index) {
   stringstream func;

   func << "BOOST_AUTO_TEST_CASE(" << test_name << ") { try {\n";
   func << "   TESTER tester;\n";
   func << "   tester.produce_block();\n";
   func << "   tester.create_account( N(wasmtest) );\n";
   func << "   tester.produce_block();\n";
   func << "   tester.set_code(N(wasmtest), wasm);\n";
   func << "   tester.produce_block();\n\n";
   func << "   action test;\n";
   func << "   test.account = N(wasmtest);\n";
   func << "   test.name = account_name((uint64_t)" << test_index << ");\n";
   func << "   test.authorization = {{N(wasmtest), config::active_name}};\n\n";
   func << "   push_action(tester, std::move(test), N(wasmtest).to_uint64_t());\n";
   func << "   tester.produce_block();\n";
   func << "   BOOST_REQUIRE_EQUAL( tester.validate(), true );\n";
   func << "} FC_LOG_AND_RETHROW() }\n";

   return func.str();

}

string create_throw_test_function(string file_name, string test_name, int test_index) {
   stringstream func;

   func << "BOOST_AUTO_TEST_CASE(" << test_name << ") { try {\n";
   func << "   TESTER tester;\n";
   func << "   tester.produce_block();\n";
   func << "   tester.create_account( N(wasmtest) );\n";
   func << "   tester.produce_block();\n";
   func << "   tester.set_code(N(wasmtest), wasm);\n";
   func << "   tester.produce_block();\n\n";
   func << "   action test;\n";
   func << "   test.account = N(wasmtest);\n";
   func << "   test.name = account_name((uint64_t)" << test_index << ");\n";
   func << "   test.authorization = {{N(wasmtest), config::active_name}};\n\n";
   func << "   BOOST_CHECK_THROW(push_action(tester, std::move(test), N(wasmtest).to_uint64_t()), wasm_execution_error);\n";
   func << "   tester.produce_block();\n";
   func << "} FC_LOG_AND_RETHROW() }\n";

   return func.str();
}

void write_tests(map<string, map<int, string>> test_mappings) {
   for (const auto& f : test_mappings) {
      ofstream ofs;
      string   file_name = f.first;

      ofs.open(file_name + ".cpp", ofstream::out);

      ofs << test_includes;
      ofs << "BOOST_AUTO_TEST_SUITE(" << normalize(file_name) << ")\n\n";

      ofs << "const string wasm_str = base_dir + \"/wasm_spec_tests/wasms/" << file_name << ".wasm\";\n";
      ofs << "std::vector<uint8_t> wasm = read_wasm(wasm_str.c_str());\n\n";

      for (const auto& ff : f.second) {
         string test_name  = normalize(file_name) + "_sub_apply_" + to_string(ff.first);
         int    test_index = ff.first;
         string type_test  = ff.second;

         if (type_test == "assert_trap") {
            ofs << create_throw_test_function(file_name, test_name, test_index) << "\n";
         } else {
            ofs << create_pass_test_function(file_name, test_name, test_index) << "\n";
         }
      }
      ofs << end_test;
   }
}
