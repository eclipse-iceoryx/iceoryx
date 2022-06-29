#pragma once

#include "error_code.hpp"
#include "error_logging.hpp"
#include "location.hpp"

#include "platform/error_reporting.hpp"

#include <iostream>
#include <type_traits>

namespace eh
{
template <typename Level>
struct ErrorProxy
{
    ErrorProxy()
    {
    }

    template <class Error>
    ErrorProxy(const SourceLocation& location, Level level, Error error)
    {
        // TODO: stream can be a thread local (check new LogStream)
        log(m_stream, location, level, error);
        report(location, level, error);
    }

    ErrorProxy(ErrorProxy&&)
    {
        // should not be used (exists for RVO in C++14)
        std::terminate();
    }

    // it may throw if the user defined handler does
    // this is dangerous (if used incorrectly) but well-defined: if a destructor throws
    // while an exception is propagated, terminate is called
    // immediately and no further propagation takes place
    //
    // this is no problem for us since we do not use exceptions in our
    // handler and even if we do during testing (ONLY!) we should not have
    // a problem as long as we never use IOX_RAISE in dtors
    // for non-fatal errors

    ~ErrorProxy() noexcept(false)
    {
        raise();
    }

    template <class F, class... Args>
    ErrorProxy& IF_RAISED(const F& f, Args&&... args)
    {
        f(std::forward<Args>(args)...);
        return *this;
    }

    template <class T>
    ErrorProxy& operator<<(const T& value)
    {
        m_stream << value;
        return *this;
    }

  private:
    // TODO: optimization where it only has a stream when needed by operator <<
    //       (is this possible?)
    ErrorStream m_stream;

    void raise()
    {
        std::cout << m_stream.str(); // flush to logger
        if (is_fatal<Level>::value)
        {
            preterminate(); // hook exists
            terminate();    // no hook exists to avoid terminate (TODO: tests?)
        }
    }
};

template <class Level, class Error>
auto create_proxy(const SourceLocation& location, Level level, const Error& error)
{
    return ErrorProxy<Level>(location, level, error);
}

} // namespace eh