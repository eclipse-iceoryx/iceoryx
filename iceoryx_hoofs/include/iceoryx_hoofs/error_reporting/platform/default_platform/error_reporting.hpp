#pragma once

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include "iceoryx_hoofs/error_reporting/platform/error_kind.hpp"

#include <atomic>
#include <iostream>

namespace err
{
// platform specific, static dispatch (optimized away)

template <class Kind, class Error>
inline void report(const SourceLocation&, Kind, const Error&)
{
    std::cout << "REPORT non-fatal" << std::endl;
}

template <class Error>
inline void report(const SourceLocation&, err::Fatal, const Error&)
{
    std::cout << "REPORT fatal" << std::endl;
}

template <class Error>
inline void report(const SourceLocation&, err::PreconditionViolation, const Error&)
{
    std::cout << "REPORT precondition violation" << std::endl;
}

template <class Error>
inline void report(const SourceLocation&, err::DebugAssertViolation, const Error&)
{
    std::cout << "REPORT debug assert violation" << std::endl;
}

[[noreturn]] inline void panic()
{
    std::cout << "PANIC" << std::endl;
    std::terminate();
}

[[noreturn]] inline void panic(const char* msg)
{
    std::cout << "PANIC " << msg << std::endl;
    std::terminate();
}

} // namespace err
