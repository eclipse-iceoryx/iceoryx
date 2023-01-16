#pragma once

#include "iceoryx_hoofs/error_handling_3/error.hpp"
#include "iceoryx_hoofs/error_handling_3/error_kind.hpp"
#include "iceoryx_hoofs/error_handling_3/location.hpp"

#include "iceoryx_hoofs/error_handling_3/platform/error_kind.hpp"

#include <iostream>

namespace eh3
{
// platform specific, static dispatch (optimized away)

template <class Kind, class Error>
inline void report(const SourceLocation&, Kind, const Error&)
{
    std::cout << "TEST REPORT non-fatal" << std::endl;
}

template <class Error>
inline void report(const SourceLocation&, eh3::Fatal, const Error&)
{
    std::cout << "TEST REPORT fatal" << std::endl;
}

template <class Error>
inline void report(const SourceLocation&, eh3::PreconditionViolation, const Error&)
{
    std::cout << "TEST REPORT precondition violation" << std::endl;
}

template <class Error>
inline void report(const SourceLocation&, eh3::DebugAssertViolation, const Error&)
{
    std::cout << "TEST REPORT debug assert violation" << std::endl;
}

inline void panic()
{
    std::cout << "TEST PANIC" << std::endl;
}

} // namespace eh3
