#pragma once

#ifndef IOX_HOOFS_TEST_ERROR_REPORTING_HPP
#define IOX_HOOFS_TEST_ERROR_REPORTING_HPP

#include "iceoryx_hoofs/error_handling_3/error.hpp"
#include "iceoryx_hoofs/error_handling_3/error_kind.hpp"
#include "iceoryx_hoofs/error_handling_3/location.hpp"

#include "iceoryx_hoofs/error_handling_3/platform/error_kind.hpp"

#include <atomic>
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

/// @todo abstract as a clean singleton in in a PolymorphicHandler once available
inline std::atomic<bool>& panicState()
{
    static std::atomic<bool> state{false};
    return state;
}

inline void panicSwitch()
{
    panicState() = true;
}

inline bool hasPanicked()
{
    return panicState();
}

inline void resetPanic()
{
    panicState() = false;
}

// NB: cannot have noreturn attribute for test cases
// need to abort thread later or similar
inline void panic()
{
    panicSwitch();
    std::cout << "TEST PANIC" << std::endl;
}

inline void panic(const char* msg)
{
    panicSwitch();
    std::cout << "TEST PANIC " << msg << std::endl;
}

} // namespace eh3

#endif
