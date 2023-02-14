#pragma once

#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
#ifndef IOX_HOOFS_TEST_ERROR_REPORTING_HPP
#define IOX_HOOFS_TEST_ERROR_REPORTING_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include "iceoryx_hoofs/error_reporting/platform/default/error_code.hpp"
#include "iceoryx_hoofs/error_reporting/platform/default/error_handler.hpp"
#include "iceoryx_hoofs/error_reporting/platform/error_kind.hpp"

#include <atomic>
#include <iostream>

namespace iox
{
namespace err
{

// The static reporting interface that must be defined to at least do nothing
// This implementation redirects to the polymorphic handler interface.
// This adds an additional indirection but is required for testing or switching handlers
// during operation (this must be done very carefully and is not recommended).

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

template <class Error, class Message>
inline void report(const SourceLocation& location, iox::err::PreconditionViolation, const Error& error, Message&& msg)
{
    auto code = toCode(error);
    /// @todo we want to log the type of error (Preconditon violation) in the same line but
    /// but this does not work here
    IOX_LOG_FATAL_ERROR(location) << std::forward<Message>(msg);
    ;
    auto& h = ErrorHandler::get();
    h.report(location, code);
}

template <class Error, class Message>
inline void report(const SourceLocation& location, iox::err::DebugAssertViolation, const Error& error, Message&& msg)
{
    auto code = toCode(error);
    /// @todo we want to log the type of error (Debug assert violation) in the same line but
    /// but this does not work here
    IOX_LOG_FATAL_ERROR(location) << std::forward<Message>(msg);
    auto& h = ErrorHandler::get();
    h.report(location, code);
}

} // namespace err
} // namespace iox

#endif
