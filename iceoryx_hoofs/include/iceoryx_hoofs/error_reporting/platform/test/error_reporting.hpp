#pragma once

#ifndef IOX_HOOFS_TEST_ERROR_REPORTING_HPP
#define IOX_HOOFS_TEST_ERROR_REPORTING_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include "iceoryx_hoofs/error_reporting/platform/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/platform/test/error_code.hpp"
#include "iceoryx_hoofs/error_reporting/platform/test/error_handler.hpp"

#include <atomic>
#include <iostream>

namespace iox
{
namespace err
{

// the static reporting interface that must be defined to at least do nothing
// This implementation redirects to the polymorphic handler interface.
// This adds an additional indirection but is required for testing.

inline void panic()
{
    std::cerr << "TEST PANIC " << std::endl;

    auto& h = ErrorHandler::get();
    h.panic();
}

inline void panic(const char* msg)
{
    std::cerr << "TEST PANIC " << msg << std::endl;

    auto& h = ErrorHandler::get();
    h.panic();
}

template <class Kind, class Error>
inline void report(const SourceLocation& location, Kind, const Error& error)
{
    std::cout << "TEST REPORT non-fatal" << std::endl;
    auto& h = ErrorHandler::get();

    h.report(location, toCode(error));
}

template <class Error>
inline void report(const SourceLocation& location, iox::err::Fatal, const Error& error)
{
    std::cout << "TEST REPORT fatal" << std::endl;
    auto& h = ErrorHandler::get();
    h.report(location, toCode(error));
}

template <class Error>
inline void report(const SourceLocation& location, iox::err::PreconditionViolation, const Error& error)
{
    std::cout << "TEST REPORT precondition violation" << std::endl;
    auto& h = ErrorHandler::get();
    h.report(location, toCode(error));
}

template <class Error>
inline void report(const SourceLocation& location, iox::err::DebugAssertViolation, const Error& error)
{
    std::cout << "TEST REPORT debug assert violation" << std::endl;
    auto& h = ErrorHandler::get();
    h.report(location, toCode(error));
}

} // namespace err
} // namespace iox

#endif
