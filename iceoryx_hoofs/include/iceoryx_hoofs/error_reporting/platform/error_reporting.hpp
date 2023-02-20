#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_REPORTING_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_REPORTING_HPP

// ***
// *** Switch between implementations at compile time
// ***

// does NOT work with tests (as they require the test handler)
// #define IOX_MINIMAL_ERROR_HANDLING

#ifdef IOX_MINIMAL_ERROR_HANDLING

#include "iceoryx_hoofs/error_reporting/platform/minimal/error_reporting.hpp"

#else

#include "iceoryx_hoofs/error_reporting/platform/default/error_reporting.hpp"

#endif

#endif
