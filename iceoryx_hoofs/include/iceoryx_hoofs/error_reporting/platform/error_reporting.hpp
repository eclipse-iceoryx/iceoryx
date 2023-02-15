#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_REPORTING_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_REPORTING_HPP

// ***
// *** Switch between implementations at compile time
// ***

// does NOT work with tests (as they require the test handler)
// #define IOX_MINIMAL_ERROR_HANDLING

#ifdef IOX_MINIMAL_ERROR_HANDLING

#include "minimal/error_reporting.hpp"

#else

#include "default/error_reporting.hpp"

#endif

#endif
