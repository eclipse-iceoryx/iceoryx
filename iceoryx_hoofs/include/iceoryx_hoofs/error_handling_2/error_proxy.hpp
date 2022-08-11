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
        log(location, level, error);
        report(location, level, error);
    }

    ~ErrorProxy() noexcept(false)
    {
        raise();
    }

    ErrorProxy(const ErrorProxy&) = delete;

    // we can do without but then the macros get a little uglier
    ErrorProxy(ErrorProxy&&) noexcept
    {
        // will not be used (exists for RVO in C++14)
        std::terminate();
    }

    ErrorProxy& operator=(const ErrorProxy&) = delete;
    ErrorProxy& operator=(ErrorProxy&&) = delete;


    template <class F, class... Args>
    ErrorProxy& onError(const F& f, Args&&... args)
    {
        f(std::forward<Args>(args)...);
        return *this;
    }

    template <class T>
    ErrorProxy& operator<<(const T& value)
    {
        errorStream() << value;
        return *this;
    }

  private:
    void raise()
    {
        flush();
        if (is_fatal<Level>::value)
        {
            preterminate(); // hook
#ifndef TEST_PLATFORM
            // TODO: how to ensure it cannot be overridden but also is not active in (all) tests?
            //       do we want to make it available in platform as hook?
            std::terminate();
#endif
        }
    }
};

template <class Level, class Error>
auto createProxy(const SourceLocation& location, Level level, const Error& error)
{
    return ErrorProxy<Level>(location, level, error);
}

} // namespace eh