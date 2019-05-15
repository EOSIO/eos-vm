#pragma once

#include <system_error>
#include <eosio/vm/error_codes_def.hpp>

#define CREATE_PAIR(X, Y) \
   X : Y;

#define CREATE_ERROR_CODES(...) \
   CREATE_PAIR(
