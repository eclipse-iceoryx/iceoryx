#pragma once

#include "iceoryx_hoofs/error_handling_3/error.hpp"
#include "iceoryx_hoofs/error_handling_3/location.hpp"

#include "iceoryx_hoofs/error_handling_3/platform/error_kind.hpp"

#include <iostream>

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

inline void panic()
{
    std::cout << "PANIC" << std::endl;
}

} // namespace eh
