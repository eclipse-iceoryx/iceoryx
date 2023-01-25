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

#define IOX_ERROR(code) err::toError(code)

/// @todo these macro constructions have (at the top level) no type safe signature
// but should actually be type safe (thoug very generic),
// they also prevent usage as expressions (they are incomplete statements), which is
// desirable to help enforce correct usage

/// @brief report error with some kind
/// @param error error object (or code)
/// @param kind kind of error
#define IOX_REPORT(error, kind) \
    if(requiresHandling(err::kind)) \
        err::createProxy(CURRENT_SOURCE_LOCATION, err::kind, err::toError(error))

/// @brief report fatal error
/// @param error error object (or code)
#define IOX_FATAL(error) IOX_REPORT(error, FATAL)

/// @todo: we may not want to expose this to reduce user options
/// @brief report error of some kind if expr evaluates to true
/// @param expr boolean expression
/// @param error error object (or code)
/// @param kind kind of error
#define IOX_REPORT_IF(expr, error, kind) \
    if(requiresHandling(err::kind)) \
        if(expr) \
             err::createProxy(CURRENT_SOURCE_LOCATION, err::kind, err::toError(error))

/// @brief report fatal error if expr evaluates to false
/// @note for conditions that may actually happen during correct use
/// @param expr boolean expression that must hold
/// @param error error object (or code)
#define IOX_REQUIRE(expr, error) IOX_REPORT_IF(!(expr), error, FATAL)

/// @brief in debug mode report fatal error if expr evaluates to false
/// @note for conditions that should not happen with correct use
/// @param expr boolean expression that must hold
/// @param error error object (or code)
#ifdef DEBUG
    #define IOX_DEBUG_ASSERT(expr) IOX_REPORT_IF(!(expr), \
      Violation(err::DEBUG_ASSERT_VIOLATION_CODE), DEBUG_ASSERT_VIOLATION)
#else
    // the proxy is not actually constructed (false branch)
    #define IOX_DEBUG_ASSERT(expr) if(false) err::ErrorProxy<err::DebugAssertViolation>()
#endif

/// @brief calls panic handler and does not return
/// @param msg optional message string literal
/// @note could actually throw if desired without breaking control flow asssumptions
#define IOX_PANIC(...) do { err::panic(__VA_ARGS__); } while(false)

// TODO: do we want specific errors for this? This is rather
// useless as it indicates a bug and shall terminate. Location should suffice.
/// @brief calls panic handler if expr is false
/// @param expr boolean expression that must hold upon entry of the function it appears in
#ifdef DEBUG
    #define IOX_EXPECTS(expr) IOX_REPORT_IF(!(expr), Violation(err::PRECONDITION_VIOLATION_CODE), PRECONDITION_VIOLATION)
#else
    #define IOX_EXPECTS(expr)
#endif


// considerations about contracts in C++
// - they are a langage feature (we cannot emulate it)
// - expects/ensures stated at the function definition
// - scoped, cannot access internal variables (locals)
// - only assert can access implementation details at a specific point during computation
// - modifiers (default, audit and axiom) control checking level
// - default - runtime check is cheap
// - audit - runtime check is expensive (disable these but keep default)
// - axiom - no runtime check (but compile time where possible?)
// - audit > default > off gradually turns checks off
//
// - expects state requirements
// - ensures provides (and checks) guarantees

// clang-format on
