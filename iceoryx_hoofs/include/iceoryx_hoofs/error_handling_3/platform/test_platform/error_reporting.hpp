#pragma once

#ifndef IOX_HOOFS_TEST_ERROR_REPORTING_HPP
#define IOX_HOOFS_TEST_ERROR_REPORTING_HPP

#include "iceoryx_hoofs/error_handling_3/error.hpp"
#include "iceoryx_hoofs/error_handling_3/error_kind.hpp"
#include "iceoryx_hoofs/error_handling_3/location.hpp"

#include "iceoryx_hoofs/error_handling_3/platform/error_kind.hpp"

#include "iceoryx_hoofs/cxx/expected.hpp"

#include <atomic>
#include <iostream>

namespace eh3
{
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

// platform specific, static dispatch (optimized away)

template <class Error>
inline bool reportError(const Error&)
{
    return true;
}

// overload for expected or other types
template <class T, class Error>
inline bool reportError(const iox::cxx::expected<T, Error>& exp)
{
    std::cout << "reportError cxx::expected" << std::endl;
    if (!exp.has_error())
    {
        return false;
    }
    // could also deal with nested expected
    return reportError(exp.get_error());
}

template <class Error>
inline void reportOrPanic(const SourceLocation&, const Error& error)
{
    if (!reportError(error))
    {
        // TOOD: log location
        panic();
    }
}

// maybe make location optional (i.e. pointer
// if the framework cannot deal wit the error it will panic (doing nothing is not a good idea)
template <class Kind, class Error>
inline void report(const SourceLocation& location, Kind, const Error& error)
{
    std::cout << "TEST REPORT non-fatal" << std::endl;
    reportOrPanic(location, error);
}

template <class Error>
inline void report(const SourceLocation& location, eh3::Fatal, const Error& error)
{
    std::cout << "TEST REPORT fatal" << std::endl;
    reportOrPanic(location, error);
}

template <class Error>
inline void report(const SourceLocation& location, eh3::PreconditionViolation, const Error& error)
{
    std::cout << "TEST REPORT precondition violation" << std::endl;
    reportOrPanic(location, error);
}

template <class Error>
inline void report(const SourceLocation& location, eh3::DebugAssertViolation, const Error& error)
{
    std::cout << "TEST REPORT debug assert violation" << std::endl;
    reportOrPanic(location, error);
}

} // namespace eh3

#endif
