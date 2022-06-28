#pragma once

// Note: the mechanism itself is secondary,
// it just has to be ensured that the desired platform implementation is included
// and only one implementation can exist in a compilation unit

#ifdef TEST_PLATFORM

#include "error_handling_test.hpp"

#else

#include "error_handling_regular.hpp"

#endif
