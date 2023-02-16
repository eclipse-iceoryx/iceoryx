#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_MINIMAL_ERROR_REPORTING_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_MINIMAL_ERROR_REPORTING_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include "iceoryx_hoofs/error_reporting/platform/error_kind.hpp"

#include <iostream>

namespace iox
{
namespace err
{
// The static reporting interface that must be defined to at least do nothing.
// It can be configured for various kinds of errors and uses the type system (i.e. overloading)
// for efficient dispatch.

// This minimal version ignores all errors and just aborts in fatal cases.

/// @brief report an error of some kind
template <class Kind, class Error>
inline void report(const SourceLocation&, Kind, const Error&)
{
}

/// @brief report a fatal (non-recoverable) error
template <class Error>
inline void report(const SourceLocation&, iox::err::Fatal, const Error&)
{
}

/// @brief report a precondition violation (non-recoverable)
template <class Error>
inline void report(const SourceLocation&, iox::err::PreconditionViolation, const Error&)
{
}

/// @brief report a debug assert violation (non-recoverable)
/// @note this is used for assumptions in the code that are only checked in a debug/safe mode
template <class Error>
inline void report(const SourceLocation&, iox::err::DebugAssertViolation, const Error&)
{
}

/// @brief react on panic (non-recoverable)
[[noreturn]] inline void panic(const SourceLocation&)
{
    std::abort();
}

/// @brief react on panic with additional message (non-recoverable)
[[noreturn]] inline void panic(const SourceLocation&, const char* msg)
{
    // deliberately do not use logger in minimal version
    std::cerr << msg << std::endl;
    std::abort();
}

} // namespace err
} // namespace iox

#endif
