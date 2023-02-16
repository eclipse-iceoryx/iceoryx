#ifndef IOX_HOOFS_ERROR_REPORTING_API_HPP
#define IOX_HOOFS_ERROR_REPORTING_API_HPP

#include "error_forwarding.hpp"
#include "error_kind.hpp"
#include "platform/error_reporting.hpp"

// clang-format on

/// @brief transforms an error code to an error object according to
///        default specification or override by module
/// @code error code to be transformed
/// @note this relies on overloading
#define IOX_ERROR(code) iox::err::toError(code)

// The following macros are statements (not expressions).
// This is important, as it enforces correct use to some degree.
// For example thye cannot be used as function arguments and must be terminated with a ';'.

/// @brief calls panic handler and does not return
/// @param msg optional message string literal
/// @note could actually throw if desired without breaking control flow asssumptions
#define IOX_PANIC(...)                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        iox::err::panic(CURRENT_SOURCE_LOCATION);                                                                      \
    } while (false)

/// @brief report error of some kind
/// @param error error object (or code)
/// @param kind kind of error
#define IOX_REPORT(error, kind)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        forwardError(CURRENT_SOURCE_LOCATION, IOX_ERROR(error), kind);                                                 \
    } while (false)

/// @brief report fatal error
/// @param error error object (or code)
#define IOX_REPORT_FATAL(error) IOX_REPORT(error, FATAL)

/// @brief report error of some kind if expr evaluates to true
/// @param expr boolean expression
/// @param error error object (or code)
/// @param kind kind of error
#define IOX_REPORT_IF(expr, error, kind)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (requiresHandling(iox::err::kind))                                                                          \
        {                                                                                                              \
            if (expr)                                                                                                  \
            {                                                                                                          \
                forwardError(CURRENT_SOURCE_LOCATION, IOX_ERROR(error), kind);                                         \
            }                                                                                                          \
        }                                                                                                              \
    } while (false)

/// @brief report fatal error if expr evaluates to false
/// @note for conditions that may actually happen during correct use
/// @param expr boolean expression that must hold
/// @param error error object (or code)
#define IOX_ASSERT(expr, error) IOX_REPORT_IF(!(expr), error, FATAL)

//*****************************
//* For safe mode and debugging
//*****************************

// Later on there can be variadic versions that do not need a default message but for now we leave
// make the more general version mandatory

// There are no error codes/errors required here on purpose, as it would make the use cumbersome.
// Instead a special internal error type is used.
// If required, a custom error option can be added but for now location should be sufficient.

/// @brief if enabled: report fatal error if expr evaluates to false
/// @param expr boolean expression that must hold upon entry of the function it appears in
/// @param message message to be logged in case of violation
#ifdef IOX_CHECK_PRECONDITIONS
#define IOX_PRECONDITION(expr, message)                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        if (requiresHandling(iox::err::PRECONDITION_VIOLATION_CODE))                                                   \
            if (!(expr))                                                                                               \
                forwardError(CURRENT_SOURCE_LOCATION,                                                                  \
                             Violation(iox::err::PRECONDITION_VIOLATION_CODE),                                         \
                             PRECONDITION_VIOLATION,                                                                   \
                             message);                                                                                 \
    } while (false)
#else
#define IOX_PRECONDITION(expr, message)                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        discard([&]() { return expr; });                                                                               \
        discard(message);                                                                                              \
    } while (false)
#endif

/// @brief if enabled: report fatal error if expr evaluates to false
/// @note for conditions that should not happen with correct use
/// @param expr boolean expression that must hold
/// @param message message to be logged in case of violation
#ifdef IOX_CHECK_ASSUMPTIONS
#define IOX_ASSUME(expr, message)                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        if (requiresHandling(iox::err::DEBUG_ASSERT_VIOLATION_CODE))                                                   \
            if (!(expr))                                                                                               \
                forwardError(CURRENT_SOURCE_LOCATION,                                                                  \
                             Violation(iox::err::DEBUG_ASSERT_VIOLATION_CODE),                                         \
                             DEBUG_ASSERT_VIOLATION,                                                                   \
                             message);                                                                                 \
    } while (false)

#else
#define IOX_ASSUME(expr, message)                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        discard([&]() { return expr; });                                                                               \
        discard(message);                                                                                              \
    } while (false)
#endif

// clang-format on

#endif
