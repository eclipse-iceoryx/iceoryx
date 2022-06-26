#pragma once

#include "raise.hpp"

using namespace eh;

// macros are required for SOURCE_LOCATION
// macros start with IOX_ but constants do not (avoids some clashes)

// TODO: ideally ErrorProxy object is only created if there is an error

// we cannot use braces or semicolons in the macro due to the proxy object use case
// does this lead to unsafe use

// clang-format off
#define IOX_RAISE(...) raise(SOURCE_LOCATION, ##__VA_ARGS__)

#define IOX_RAISE_AND(...) raise(SOURCE_LOCATION, ##__VA_ARGS__)

#define IOX_FATAL(...) raise(SOURCE_LOCATION, FATAL, ##__VA_ARGS__)

#define IOX_RAISE_IF(expr, ...) raise_if(SOURCE_LOCATION, [&]() -> bool { return expr; }, ##__VA_ARGS__)

#define IOX_ASSERT(expr, ...) raise_if(SOURCE_LOCATION, [&]() -> bool {return !(expr);}, FATAL, ##__VA_ARGS__)

// define somewhere reasonable
#define DEBUG

#ifdef DEBUG
    #define IOX_DEBUG_ASSERT(expr, ...) IOX_ASSERT(expr, ##__VA_ARGS__)
#else    
    #define IOX_DEBUG_ASSERT(expr, ...)
#endif

// clang-format on
