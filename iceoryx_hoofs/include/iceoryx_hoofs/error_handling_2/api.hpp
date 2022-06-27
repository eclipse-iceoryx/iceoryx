#pragma once

#include "raise.hpp"

// macros are required for SOURCE_LOCATION
// macros start with IOX_ but constants do not (avoids some clashes)

// note that we can always decide to not allow variadic versions
// which and then always require a concrete error (code)

// clang-format off
#define IOX_RAISE(...) eh::raise(SOURCE_LOCATION, ##__VA_ARGS__)

#define IOX_FATAL(...) eh::raise(SOURCE_LOCATION, FATAL, ##__VA_ARGS__)

// deferred evaluation of expr for performance
#define IOX_RAISE_IF(expr, ...) eh::raise_if(SOURCE_LOCATION, [&]() -> bool { return expr; }, ##__VA_ARGS__)

#define IOX_ASSERT(expr, ...) eh::raise_if(SOURCE_LOCATION, [&]() -> bool {return !(expr);}, FATAL, ##__VA_ARGS__)

// define somewhere reasonable
// #define DEBUG

#ifdef DEBUG
    #define IOX_DEBUG_ASSERT(expr, ...) IOX_ASSERT(expr, ##__VA_ARGS__)
#else    
    #define IOX_DEBUG_ASSERT(expr, ...) eh::EmptyProxy()
#endif

// clang-format on
