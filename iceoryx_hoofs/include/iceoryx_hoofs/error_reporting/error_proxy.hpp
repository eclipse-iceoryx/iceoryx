#pragma once

#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/error_stream.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include "platform/error_reporting.hpp"

#include <iostream>
#include <type_traits>

namespace iox
{
namespace err
{

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
    {
        // use the logger (problematic if we want to allow expected as we expect something)
        // log(location, kind, error);

        // report to other framework
        report(location, kind, error);
    }

    ~ErrorProxy()
    {
        iox::err::flush();
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

    template <typename Msg>
    ErrorStream& operator<<(const Msg& msg)
    {
        // will not be reported but logged
        // reporting would be inefficient, as we would need to store the error
        // until we have recorded the message
        auto& s = errorStream();
        s << msg;

        // allows chaining of additional messages into the eror stream
        return s;
    }

  private:
};

template <class Kind, class Error>
auto createProxy(const SourceLocation& location, Kind kind, const Error& error)
{
    return ErrorProxy<Kind>(location, kind, error);
}

} // namespace err
} // namespace iox
