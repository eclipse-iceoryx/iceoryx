#pragma once

#include "raise.hpp"

using namespace eh;

// macros are required for SOURCE_LOCATION
// macros start with IOX_ but constants do not (avoids some clashes)

// TODO: ideally ErrorProxy object is only created if there is an error

#if 0
#define IOX_RAISE(...)                                                                                                 \
    {                                                                                                                  \
        raise(SOURCE_LOCATION, ##__VA_ARGS__);                                                                         \
    }

#define IOX_FATAL(...)                                                                                                 \
    {                                                                                                                  \
        raise(SOURCE_LOCATION, FATAL, ##__VA_ARGS__);                                                                  \
    }

// this will defer expression evaluation
#define IOX_RAISE_IF(expr, ...)                                                                                        \
    {                                                                                                                  \
        auto e = [&]() -> bool { return expr; };                                                                       \
        raise_if(SOURCE_LOCATION, e, ##__VA_ARGS__);                                                                   \
    }

#define IOX_ASSERT(expr, ...)                                                                                          \
    {                                                                                                                  \
        auto e = [&]() -> bool { return !(expr); };                                                                    \
        raise_if(SOURCE_LOCATION, e, FATAL, ##__VA_ARGS__);                                                            \
    }
#endif

// we cannot use braces or semicolons in the macro due to the proxy object use case
// does this lead to unsafe use

// clang-format off
#define IOX_RAISE(...) raise(SOURCE_LOCATION, ##__VA_ARGS__)

#define IOX_FATAL(...) raise(SOURCE_LOCATION, FATAL, ##__VA_ARGS__)

#define IOX_RAISE_IF(expr, ...) raise_if(SOURCE_LOCATION, [&]() -> bool { return expr; }, ##__VA_ARGS__)

#define IOX_ASSERT(expr, ...) raise_if(SOURCE_LOCATION, [&]() -> bool {return !(expr);}, FATAL, ##__VA_ARGS__)

#define DEBUG
#ifdef DEBUG
    #define IOX_DEBUG_ASSERT(expr, ...) IOX_ASSERT(expr, ##__VA_ARGS__)
#else
    // TODO: not good, will lead to unused variable warning or evaluation of expression
    #define IOX_DEBUG_ASSERT(expr, ...)
#endif

// clang-format on
