#pragma once

#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
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
// This adds an additional indirection but is required for testing or switching handlers
// during operation (this must be done very carefully and is not recommended).

// TODO: report the level

inline void panic()
{
    IOX_LOG_PANIC() << "PANIC";
    auto& h = ErrorHandler::get();
    h.panic();
}

inline void panic(const char* msg)
{
    // TODO: propagate location to this call
    IOX_LOG_PANIC() << "PANIC " << msg;
    auto& h = ErrorHandler::get();
    h.panic();
}

template <class Kind, class Error>
inline void report(const SourceLocation& location, Kind, const Error& error)
{
    // TODO: decide whether we want to log the error in all cases with a custom handler
    auto code = toCode(error);
    IOX_LOG_ERROR(location) << "Error " << code;
    auto& h = ErrorHandler::get();
    h.report(location, code);
}

template <class Error>
inline void report(const SourceLocation& location, iox::err::Fatal, const Error& error)
{
    auto code = toCode(error);
    IOX_LOG_FATAL_ERROR(location) << "Fatal Error " << code;
    auto& h = ErrorHandler::get();
    h.report(location, code);
}

template <class Error>
inline void report(const SourceLocation& location, iox::err::PreconditionViolation, const Error& error)
{
    auto code = toCode(error);
    IOX_LOG_FATAL_ERROR(location) << "Precondition Violation";
    auto& h = ErrorHandler::get();
    h.report(location, code);
}

template <class Error>
inline void report(const SourceLocation& location, iox::err::DebugAssertViolation, const Error& error)
{
    auto code = toCode(error);
    IOX_LOG_FATAL_ERROR(location) << "Debug Assert Violation";
    auto& h = ErrorHandler::get();
    h.report(location, code);
}

} // namespace err
} // namespace iox

#endif
