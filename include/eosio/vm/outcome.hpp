#pragma once

/// wrapper header to use either the built in outcome library or an external Boost v1.70 outcome library
#ifdef EOSIO_USE_EXTERNAL_OUTCOME
#   include <outcome/single-header/outcome.hpp>
#else
//#include <
#endif
