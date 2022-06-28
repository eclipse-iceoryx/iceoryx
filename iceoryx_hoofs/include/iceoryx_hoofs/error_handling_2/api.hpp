#pragma once

#include "raise.hpp"

// macros are required for SOURCE_LOCATION
// macros start with IOX_ but constants do not (avoids some clashes)

// note that we can always decide to not allow variadic versions
// which and then always require a concrete error (code)

// define somewhere reasonable
#define DEBUG

// clang-format off
// TODO:make defines FATAL typesafe like this
#define IOX_RAISE(level, ...) eh::raise(SOURCE_LOCATION, eh::level, ##__VA_ARGS__)

#define IOX_FATAL(...) eh::raise(SOURCE_LOCATION, eh::FATAL, ##__VA_ARGS__)

// deferred evaluation of expr for performance
#define IOX_RAISE_IF(expr, level, ...) eh::raise_if(SOURCE_LOCATION, [&]() -> bool { return expr; }, eh::level, ##__VA_ARGS__)

#define IOX_ASSERT(expr, ...) eh::raise_if(SOURCE_LOCATION, [&]() -> bool {return !(expr);}, eh::FATAL, ##__VA_ARGS__)

#ifdef DEBUG
    #define IOX_DEBUG_ASSERT(expr, ...) IOX_ASSERT(expr, ##__VA_ARGS__)
#else    
    #define IOX_DEBUG_ASSERT(expr, ...) eh::EmptyProxy()
#endif

// clang-format on
