#pragma once

#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/error_logging.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include "platform/error_reporting.hpp"

#include <utility>

namespace iox
{
namespace err
{

template <typename Error, typename Kind>
[[noreturn]] void forwardFatalError(const SourceLocation& location, Error&& error, Kind&& kind)
{
    report(location, kind, error);
    panic();
    abort(); // to satisfy the no-return guarantee
}

template <typename Error, typename Kind>
void forwardNonFatalError(const SourceLocation& location, Error&& error, Kind&& kind)
{
    report(location, kind, error);
}

template <typename Error, typename Kind>
void forwardError(const SourceLocation& location, Error&& error, Kind&& kind)
{
    // forwarding selection happens at compile time
    if (requiresHandling(kind))
    {
        if (IsFatal<Kind>::value)
        {
            forwardFatalError(location, std::forward<Error>(error), std::forward<Kind>(kind));
        }
        else
        {
            forwardNonFatalError(location, std::forward<Error>(error), std::forward<Kind>(kind));
        }
    }
}

/// @brief Reports error and optionally acts as a logger for additional messages
/// Will call panic in destructor and not return if the error kind is
/// configured for this behaviour.
/// @tparam Kind kind of error, controls behaviour of the ErrorProxy (e.g. panic)
/// @note A raw function for this kind of deleagtion does not suffice for optional
/// logging with multiple arguments.
template <typename Kind>
class ErrorProxy final
{
  public:
    ErrorProxy() = default;

    template <class Error>
    ErrorProxy(const SourceLocation& location, Kind kind, Error error)
        : m_location(location)
    {
        // use the logger (problematic if we want to allow expected as we expect something)
        // log(location, kind, error);

        // report to other framework
        report(location, kind, error);
    }

    ~ErrorProxy()
    {
        // defer the panic to be able to add functionality to the proxy
        // such as printing messages
        if (IsFatal<Kind>::value)
        {
            panic();
            // will not return, note that the ErrorProxy does not leak resources
        }
    }

    ErrorProxy(const ErrorProxy&) = delete;
    ErrorProxy(ErrorProxy&&) noexcept = default;

    ErrorProxy& operator=(const ErrorProxy&) = delete;
    ErrorProxy& operator=(ErrorProxy&&) = delete;

    template <typename T>
    ErrorProxy& operator<<(const T& value)
    {
        (void)value;
        IOX_LOG_ERROR(m_location) << value;


        // allows chaining of additional messages into the eror stream
        // TODO: we need the logstream for this and return it instead to get nice output
        return *this;
    }

  private:
    SourceLocation m_location;
};

template <class Kind, class Error>
auto createProxy(const SourceLocation& location, Kind kind, const Error& error)
{
    return ErrorProxy<Kind>(location, kind, error);
}

} // namespace err
} // namespace iox
