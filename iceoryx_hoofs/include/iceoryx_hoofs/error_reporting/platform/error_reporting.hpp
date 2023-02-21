#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_REPORTING_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_REPORTING_HPP

// ***
// *** Switch between implementations at compile time
// ***

// does NOT work with tests (as they require the test handler)
// #define IOX_MINIMAL_ERROR_HANDLING

#if 0

// Minimal error reporting implementation
// 1. Terminates on fatal error
// 2. Cannot be exchanged at runtime

#include "iceoryx_hoofs/error_reporting/platform/minimal/error_reporting.hpp"

#else

// Default error reporting version
// 1. Terminates on fatal error
// 2. Can be exchanged at runtime
// 3. Logs errors

#include "iceoryx_hoofs/error_reporting/platform/default/error_reporting.hpp"

#endif

#endif
