#pragma once

// ***
// *** TO BE IMPLEMENTED BY CLIENT
// ***

// does NOT work with tests (as they require the test handler)
// #define IOX_MINIMAL_ERROR_HANDLING

#ifdef IOX_MINIMAL_ERROR_HANDLING

#include "minimal/error_reporting.hpp"

#else

#include "default/error_reporting.hpp"

#endif
