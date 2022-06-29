#pragma once

#include "raise.hpp"

// macros are required for SOURCE_LOCATION
// macros start with IOX_ but constants do not (avoids some clashes)

// note that we can always decide to not allow variadic versions
// which and then always require a concrete error (code)

// define somewhere reasonable
#define DEBUG

// prevent misuse in assignment with "if(true)"

// clang-format off

#if 0
#define IOX_RAISE(level, ...) if(true) eh::raise(SOURCE_LOCATION, eh::level, ##__VA_ARGS__)

#define IOX_FATAL(...) if(true) eh::raise(SOURCE_LOCATION, eh::FATAL, ##__VA_ARGS__)

// deferred evaluation of expr for performance
#define IOX_RAISE_IF(expr, level, ...) if(true) eh::raise_if(SOURCE_LOCATION, [&]() -> bool { return expr; }, eh::level, ##__VA_ARGS__)

#define IOX_ASSERT(expr, ...) if(true) eh::raise_if(SOURCE_LOCATION, [&]() -> bool {return !(expr);}, eh::FATAL, ##__VA_ARGS__)

#ifdef DEBUG
    #define IOX_DEBUG_ASSERT(expr, ...) IOX_ASSERT(expr, ##__VA_ARGS__)
#else    
    #define IOX_DEBUG_ASSERT(expr, ...) eh::EmptyProxy()
#endif
#else

// non variadic (slight limitation)
#define IOX_RAISE(level, error) if(true) eh::raise(SOURCE_LOCATION, eh::level, eh::create_error(error))

#define IOX_FATAL(error) if(true) eh::raise(SOURCE_LOCATION, eh::FATAL, eh::create_error(error))

// deferred evaluation of expr for performance
#define IOX_RAISE_IF(expr, level, error) if(true) eh::raise_if(SOURCE_LOCATION, [&]() -> bool { return expr; }, eh::level, eh::create_error(error))

#define IOX_ASSERT(expr, error) if(true) eh::raise_if(SOURCE_LOCATION, [&]() -> bool {return !(expr);}, eh::FATAL, eh::create_error(error))

#ifdef DEBUG
    #define IOX_DEBUG_ASSERT(expr, error) IOX_ASSERT(expr, error)
#else    
    #define IOX_DEBUG_ASSERT(expr, error) eh::EmptyProxy()
#endif



#endif

// clang-format on
