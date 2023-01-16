#pragma once

#include "error_kind.hpp"
#include "error_proxy.hpp"

// macros are required for CURRENT_SOURCE_LOCATION
// macros start with IOX_ but constants do not (avoids some clashes)

// - error kind is defined by the platform (FATAL and user defined in eh namespace)
// - error is an error code or some error type (with mild interface requirements)
// - error and error kind are separated to allow raising the same error (e.g. an
//   out of bounds access) as different kind, e.g. a precondition violation or
//   some memory corruption related error
// - the macros are supposed to be usable only has statements, not expressions
// - macros may be undesirable (but needed for source location and if not to be used
//   as expressions in e.g. function arguments)

// clang-format off

#define IOX_ERROR(code) eh3::toError(code)

/// @brief report error with some kind
#define IOX_REPORT(error, kind) \
    if(requiresHandling(eh3::kind)) \
        eh3::createProxy(CURRENT_SOURCE_LOCATION, eh3::kind, eh3::toError(error))

/// @brief report fatal error
#define IOX_FATAL(error) IOX_REPORT(error, FATAL)

/// @brief report error of some kind if expr evaluates to true
#define IOX_REPORT_IF(expr, error, kind) \
    if(requiresHandling(eh3::kind)) \
        if(expr) \
             eh3::createProxy(CURRENT_SOURCE_LOCATION, eh3::kind, eh3::toError(error))

/// @brief report fatal error if expr evaluates to false
/// @note for conditions that may actually happen during correct use
#define IOX_ASSERT(expr, error) IOX_REPORT_IF(!(expr), error, FATAL)

/// @brief in debug mode report fatal error if expr evaluates to false
/// @note for conditions that should not happen with correct use
#ifdef DEBUG
    #define IOX_DEBUG_ASSERT(expr, error) IOX_REPORT_IF(!(expr), error, DEBUG_ASSERT_VIOLATION)
#else
    // the proxy is not actually constructed (false branch)
    #define IOX_DEBUG_ASSERT(expr, error) if(false) eh3::ErrorProxy<eh3::DebugAssertViolation>()
#endif

/// @todo NB: could actually throw if desired
/// @brief calls panic handler and does not return
#define IOX_PANIC do { eh3::panic(); } while(false)

// TODO: do we want specific errors for this? This is rather
// useless as it indicates a bug and shall terminate. Location should suffice.
/// @brief calls panic handler if expr is false
#define IOX_PRECOND(expr) IOX_REPORT_IF(!(expr), PreconditionError(), PRECONDITION_VIOLATION)

// clang-format on
