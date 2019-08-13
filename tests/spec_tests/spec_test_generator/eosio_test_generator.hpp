#pragma once
#include <map>
#include <string>

std::string normalize(std::string val);

void write_tests(std::map<std::string, std::map<int, std::string>> test_mappings);
