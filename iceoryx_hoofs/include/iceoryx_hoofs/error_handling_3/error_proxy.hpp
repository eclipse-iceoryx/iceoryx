#pragma once

#include "iceoryx_hoofs/error_handling_2/error_level.hpp"
#include "iceoryx_hoofs/error_handling_3/error_logging.hpp"
#include "iceoryx_hoofs/error_handling_3/location.hpp"

#include "platform/error_reporting.hpp"

#include <iostream>
#include <type_traits>

namespace eh3
{

// class to later allow using state of the object if necessary
// must be lightweight to construct
template <typename Level>
class ErrorProxy final
{
  public:
    ErrorProxy()
    {
    }

    template <class Error>
    ErrorProxy(const SourceLocation& location, Level level, Error error)
    {
        // use the logger
        log(location, level, error);

        // report to other framework
        report(location, level, error);
    }

    ~ErrorProxy()
    {
        errorStream() << std::endl;
        errorStream().flush();
        // defer the panic to be able to add functionality to the proxy
        // compile time
        if (isFatal<Level>)
        {
            panic();
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
        return s;
    }

  private:
};

// can we sensibly make this static?
template <class Level, class Error>
auto createProxy(const SourceLocation& location, Level level, const Error& error)
{
    return ErrorProxy<Level>(location, level, error);
}
} // namespace eh3
