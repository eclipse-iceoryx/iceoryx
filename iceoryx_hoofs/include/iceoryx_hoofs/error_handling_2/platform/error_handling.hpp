#pragma once


// Note: the mechanism itself does not matter, it just has to be ensured that the desired platform implementation is
// included

#define TEST_PLATFORM

#ifdef TEST_PLATFORM

#include "error_handling_test.hpp"

#else

#include "error_handling_regular.hpp"

#endif
