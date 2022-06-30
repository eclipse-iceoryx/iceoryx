#pragma once

#include "proxy.hpp"

// macros are required for SOURCE_LOCATION
// macros start with IOX_ but constants do not (avoids some clashes)

// - level is one of the error levels defined by the platform (FATAL and user defined in eh namespace)
// - error is an error code or some error type which requires to be convertible to
//   an error via create_error (this can be the identity) defined by the module

// clang-format off

#define IOX_RAISE(level, error) \
    if(requires_handling(eh::level)) \
        eh::create_proxy(SOURCE_LOCATION, eh::level, eh::create_error(error))

#define IOX_FATAL(error) IOX_RAISE(FATAL, error)

// note that the check for expr occurs at runtime (while the other does not, allows to optimize the whole branch away)
#define IOX_RAISE_IF(expr, level, error) \
    if(requires_handling(eh::level)) \
        if([&]() -> bool { return expr; }()) \
             eh::create_proxy(SOURCE_LOCATION, eh::level, eh::create_error(error))

#define IOX_ASSERT(expr, error) \
    if(requires_handling(eh::FATAL)) \
        if([&]() -> bool { return !(expr); }()) \
            eh::create_proxy(SOURCE_LOCATION, eh::FATAL, eh::create_error(error))

#ifdef DEBUG
    #define IOX_DEBUG_ASSERT(expr, error) IOX_ASSERT(expr, error)
#else
    // the proxy is not actually constructed (false branch)
    #define IOX_DEBUG_ASSERT(expr, error) if(false) eh::ErrorProxy<eh::Fatal>()
#endif

// clang-format on
