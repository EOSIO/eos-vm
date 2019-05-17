#include "picojson.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

using namespace std;

//string generate_tests( string name, 
const string test_preamble_0 = "backend_t bkend( backend_t::read_wasm( ";
const string_test_preamble_1 = "bkend.set_wasm_allocator( &wa );"

string generate_test_call( const picojson::object& obj, string expected_t, string expected_v ) {
   stringstream ss;
   
   if (expected_t == "i32")
     ss << "to_i32";
   else if (expected_t == "i64")
     ss << "to_i64";
   else if (expected_t == "f32")
     ss << "to_f32";
   else
     ss << "to_f64"; 

   ss << "(*bkend.call_with_return(nullptr, \"env\", ";
   for ( picojson::object arg : obj["args"].get_array() ) {
      if ( arg["type"].to_str() == "i32" )
      ss << 
   }
}

void generate_tests( const map<string, vector<picojson::object>>& mappings ) {
   stringstream unit_tests;
   for ( const auto& [tsn, cmds] : mappings ) {
      unit_tests << "TEST_CASE( \"Testing wasm <" << tsn << ">\", \"[" << tsn << "_tests]\" ) {\n";
      unit_tests << "   " << test_preamble_0 << tsn << " );\n";
      unit_tests << "   " << test_preamble_1 << "\n";

      for ( picojson::object cmd : cmds ) {
         if ( cmd["type"].to_str() == "assert_return" ) {
            unit_tests << "   CHECK(";
         }
         cout << cmd["type"].to_str() << '\n';
      }
   }
}

int main() {
   ifstream ifs;
   stringstream ss;
   ifs.open("test.json");
   string s;
   while ( getline(ifs, s) )
      ss << s;
   ifs.close();

   picojson::value v;
   picojson::parse(v, ss.str());
   string test_suite_name;

   map<string, vector<picojson::object>> test_mappings;
   const picojson::value::object& obj = v.get<picojson::object>();
   for ( picojson::value::object::const_iterator i = obj.begin(); i != obj.end(); i++ )
      if ( i->first == "commands" ) {
         for ( const auto& o : i->second.get<picojson::array>() ) {
            picojson::object obj = o.get<picojson::object>();
            if ( obj["type"].to_str() == "module" ) {
               test_suite_name = obj["filename"].to_str();
               [&]() {
                  for (int i=0; i <= test_suite_name.size(); i++)
                     test_suite_name[i] = test_suite_name[i] == '.' ? '_' : test_suite_name[i];
               }();
               test_mappings[test_suite_name] = {};
            }
            test_mappings[test_suite_name].push_back( obj );
         }
         //cout << i->first << " : " << i->second.to_str() << "\n";
      }

   generate_tests( test_mappings );
}
