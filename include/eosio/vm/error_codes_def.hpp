#pragma once

#include "error_codes_pp.hpp"

namespace eosio { namespace vm {
// create a category for parser error codes
GENERATE_ERROR_CATEGORY( parser_errors, "eos-vm parser errors" )

#define PARSER_ERRORS( macro ) \
   macro( parser_errors, invalid_magic_number ) \
   macro( parser_errors, invalid_version ) \
   macro( parser_errors, general_parsing_failure )

CREATE_ERROR_CODES( parser_errors, PARSER_ERRORS )

}} // namespace eosio::vm
