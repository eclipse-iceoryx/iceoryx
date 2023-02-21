#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_REPORTING_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_REPORTING_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

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

[[noreturn]] inline void panic(const SourceLocation& location)
{
    IOX_LOG_PANIC(location) << "Panic";
    auto& h = ErrorHandler::get();
    h.panic();
    abort();
}

[[noreturn]] inline void panic(const SourceLocation& location, const char* msg)
// inline void panic(const SourceLocation& location, const char* msg)
{
    IOX_LOG_PANIC(location) << "Panic " << msg;
    auto& h = ErrorHandler::get();
    h.panic();
    abort();
}

template <class Kind, class Error>
inline void report(const SourceLocation& location, Kind, const Error& error)
{
    auto code = toCode(error);
    IOX_LOG_ERROR(location) << " Error " << code.value << " in module " << toModule(error).value;
    auto& h = ErrorHandler::get();
    h.report(location, code);
}

template <class Error>
inline void report(const SourceLocation& location, iox::err::Fatal, const Error& error)
{
    auto code = toCode(error);
    IOX_LOG_FATAL_ERROR(location) << " Fatal Error " << code.value << " in module " << toModule(error).value;
    auto& h = ErrorHandler::get();
    h.report(location, code);
}

template <class Error, class Message>
inline void report(const SourceLocation& location, iox::err::PreconditionViolation, const Error& error, Message&& msg)
{
    auto code = toCode(error);
    IOX_LOG_FATAL_ERROR(location) << ": Precondition Violation " << std::forward<Message>(msg);
    auto& h = ErrorHandler::get();
    h.report(location, code);
}

template <class Error, class Message>
inline void report(const SourceLocation& location, iox::err::DebugAssertViolation, const Error& error, Message&& msg)
{
    auto code = toCode(error);
    IOX_LOG_FATAL_ERROR(location) << ": Debug Assert Violation " << std::forward<Message>(msg);
    auto& h = ErrorHandler::get();
    h.report(location, code);
}

} // namespace err
} // namespace iox

#endif
