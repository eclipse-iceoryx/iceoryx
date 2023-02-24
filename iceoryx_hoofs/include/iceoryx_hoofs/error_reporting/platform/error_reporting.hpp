#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_REPORTING_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_ERROR_REPORTING_HPP

// ***
// *** Switch between implementations at compile time
// ***

// Select the platform specific implementation here
// and include this file in every module that uses error handling.

#include "iceoryx_hoofs/error_reporting/platform/default/configuration.hpp"

#include "iceoryx_hoofs/error_reporting/platform/default/error_reporting.hpp"

#endif
