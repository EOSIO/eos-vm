#pragma once

#define EOS_VM_OPEN_NAMESPACE namespace eosio::vm {
#define EOS_VM_CLOSE_NAMESPACE }

#include "error_codes_pp.hpp"

// create a category for parser error codes
GENERATE_ERROR_CATEGORY( parser_errors, "eos-vm parser errors" )

#define PARSER_ERRORS(macro)                                                                                           \
   macro(parser_errors, invalid_magic_number)
   macro(parser_errors, invalid_version)                    \
   macro(parser_errors, general_parsing_failure)

CREATE_ERROR_CODES(parser_errors, PARSER_ERRORS)
