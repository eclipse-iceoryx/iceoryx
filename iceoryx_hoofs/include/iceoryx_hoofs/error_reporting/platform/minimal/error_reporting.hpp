#pragma once

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

// TODO: log with logger?

template <class Kind, class Error>
inline void report(const SourceLocation&, Kind, const Error&)
{
}

template <class Error>
inline void report(const SourceLocation&, iox::err::Fatal, const Error&)
{
}

template <class Error>
inline void report(const SourceLocation&, iox::err::PreconditionViolation, const Error&)
{
}

template <class Error>
inline void report(const SourceLocation&, iox::err::DebugAssertViolation, const Error&)
{
}

[[noreturn]] inline void panic()
{
    std::abort();
}

[[noreturn]] inline void panic(const char* msg)
{
    std::cout << msg << std::endl;
    std::abort();
}

} // namespace err
} // namespace iox
