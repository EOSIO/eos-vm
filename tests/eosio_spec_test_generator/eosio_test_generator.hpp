#pragma once
#include <map>
#include <string>
#include <vector>

struct spec_test {
   std::string name;
   int assert_trap_start_index;
   int assert_trap_end_index;
   int assert_return_start_index;
   int assert_return_end_index;
};

std::string normalize(std::string val);

void write_tests(std::vector<spec_test> tests);
