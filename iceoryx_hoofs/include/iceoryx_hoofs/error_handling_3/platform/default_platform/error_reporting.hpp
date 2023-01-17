#pragma once

#include "iceoryx_hoofs/error_handling_3/error.hpp"
#include "iceoryx_hoofs/error_handling_3/location.hpp"

#include "iceoryx_hoofs/error_handling_3/platform/error_kind.hpp"

#include <iostream>
#include <atomic>

namespace eh3
{
// platform specific, static dispatch (optimized away)

template <class Kind, class Error>
inline void report(const SourceLocation&, Kind, const Error&)
{
    std::cout << "REPORT non-fatal" << std::endl;
}

template <class Error>
inline void report(const SourceLocation&, eh3::Fatal, const Error&)
{
    std::cout << "REPORT fatal" << std::endl;
}

template <class Error>
inline void report(const SourceLocation&, eh3::PreconditionViolation, const Error&)
{
    std::cout << "REPORT precondition violation" << std::endl;
}

template <class Error>
inline void report(const SourceLocation&, eh3::DebugAssertViolation, const Error&)
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

} // namespace eh
