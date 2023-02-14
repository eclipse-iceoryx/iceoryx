#pragma once

#include "error_forward.hpp"
#include "error_kind.hpp"

// clang-format on

#define IOX_ERROR(code) iox::err::toError(code)

// The following macros are statements (not expressions).
// This is important, as it enforces correct use to some degree.
// For example thye cannot be used as function arguments and must be terminated with a ';'.

/// @brief calls panic handler and does not return
/// @param msg optional message string literal
/// @note could actually throw if desired without breaking control flow asssumptions
#define IOX_PANIC(...) do { iox::err::panic(__VA_ARGS__); } while(false)

/// @brief report error of some kind
/// @param error error object (or code)
/// @param kind kind of error
#define IOX_REPORT(error, kind)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        forwardError(CURRENT_SOURCE_LOCATION, error, kind);                                                            \
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
                forwardError(CURRENT_SOURCE_LOCATION, error, kind);                                                    \
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

/// @brief in debug mode only: report fatal error if expr evaluates to false
/// @param expr boolean expression that must hold upon entry of the function it appears in
/// @param message message to be logged in case of violation
#ifdef IOX_DEBUG
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
    } while (false)
#endif

/// @brief in debug mode only: report fatal error if expr evaluates to false
/// @note for conditions that should not happen with correct use
/// @param expr boolean expression that must hold
/// @param message message to be logged in case of violation
#ifdef IOX_DEBUG
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
    } while (false)
#endif

// clang-format on
