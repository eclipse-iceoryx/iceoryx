#pragma once

// ***
// *** TO BE IMPLEMENTED BY CLIENT
// ***

// Note: the mechanism itself is secondary,
// it just has to be ensured that the desired platform implementation is included
// and only one implementation can exist in a compilation unit

#ifdef TEST_PLATFORM

#include "test_platform/error_reporting.hpp"

#else

#include "default_platform/error_reporting.hpp"

#endif
