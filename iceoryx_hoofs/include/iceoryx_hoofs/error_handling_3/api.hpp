#pragma once

#include "basic_error_level.hpp"
#include "error_proxy.hpp"

// macros are required for CURRENT_SOURCE_LOCATION
// macros start with IOX_ but constants do not (avoids some clashes)

// - level is one of the error levels defined by the platform (FATAL and user defined in eh namespace)
// - error is an error code or some error type (with mild interface requirements)
// clang-format off


/// @brief report error with some level
#define IOX_REPORT(level, error) \
    if(requiresHandling(eh3::level)) \
        eh3::createProxy(CURRENT_SOURCE_LOCATION, eh3::level, eh3::toError(error))

/// @brief report fatal error
#define IOX_FATAL(error) IOX_REPORT(FATAL, error)

/// @brief report error of some level if expr evaluates to true
#define IOX_REPORT_IF(expr, level, error) \
    if(requiresHandling(eh::level)) \
        if(expr) \
             eh3::createProxy(CURRENT_SOURCE_LOCATION, eh3::level, eh3::toError(error))

/// @brief report fatal error if expr evaluates to false
/// @note for conditions that may actually happen during correct use
#define IOX_ASSERT(expr, error) IOX_REPORT_IF(!(expr), FATAL, error)

/// @brief in debug mode report fatal error if expr evaluates to false
/// @note for conditions that should not happen with correct use
#ifdef DEBUG
    #define IOX_DEBUG_ASSERT(expr, error) IOX_ASSERT(expr, error)
#else
    // the proxy is not actually constructed (false branch)
    #define IOX_DEBUG_ASSERT(expr, error) if(false) eh::ErrorProxy<eh::Fatal>()
#endif

/// @brief calls panic handler and does not return
#define IOX_PANIC do { eh3::panic(); } while(false)

// clang-format on
