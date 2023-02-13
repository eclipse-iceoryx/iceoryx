#pragma once

#include "error_forward.hpp"
#include "error_kind.hpp"

// clang-format off

#define IOX_ERROR(code) iox::err::toError(code)

/// The following macros are statements (not expressions).
/// This is important, as it enforces correct use to some degree.

/// @brief calls panic handler and does not return
/// @param msg optional message string literal
/// @note could actually throw if desired without breaking control flow asssumptions
#define IOX_PANIC(...) do { iox::err::panic(__VA_ARGS__); } while(false)

/// @brief report error of some kind
/// @param error error object (or code)
/// @param kind kind of error
#define IOX_REPORT(error, kind) do { \
    forwardError(CURRENT_SOURCE_LOCATION, error, kind); \
} while(false)

#if 0
/// @brief report fatal error
/// @param error error object (or code)
#define IOX_REPORT_FATAL(error) IOX_REPORT(error, FATAL)

/// @brief report error of some kind if expr evaluates to true
/// @param expr boolean expression
/// @param error error object (or code)
/// @param kind kind of error
#define IOX_REPORT_IF(expr, error, kind) \
    if(requiresHandling(iox::err::kind)) \
        if(expr) \
             iox::err::createProxy(CURRENT_SOURCE_LOCATION, iox::err::kind, iox::err::toError(error))

/// @brief report fatal error if expr evaluates to false
/// @note for conditions that may actually happen during correct use
/// @param expr boolean expression that must hold
/// @param error error object (or code)
#define IOX_ASSERT(expr, error) IOX_REPORT_IF(!(expr), error, FATAL)

/// @brief calls panic handler if expr is false
/// @param expr boolean expression that must hold upon entry of the function it appears in
#ifdef IOX_DEBUG
    #define IOX_PRECONDITION(expr) IOX_REPORT_IF(!(expr), Violation(iox::err::PRECONDITION_VIOLATION_CODE), PRECONDITION_VIOLATION)
#else
    #define IOX_PRECONDITION(expr)
#endif

/// @brief in debug mode report fatal error if expr evaluates to false
/// @note for conditions that should not happen with correct use
/// @param expr boolean expression that must hold
/// @param error error object (or code)
#ifdef IOX_DEBUG
    #define IOX_ASSUME(expr) IOX_REPORT_IF(!(expr), \
      Violation(iox::err::DEBUG_ASSERT_VIOLATION_CODE), DEBUG_ASSERT_VIOLATION)
#else   
    #define IOX_ASSUME(expr) if(false) iox::err::ErrorProxy<iox::err::DebugAssertViolation>()
#endif
#endif

// clang-format on
