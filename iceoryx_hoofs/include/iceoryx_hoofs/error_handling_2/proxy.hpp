#pragma once

#include "error_code.hpp"
#include "error_logging.hpp"
#include "location.hpp"

#include "platform/error_reporting.hpp"

#include <iostream>
#include <type_traits>

namespace eh
{
template <class Level, class Error>
struct ErrorProxy
{
    ErrorProxy()
    {
    }
    ErrorProxy(const SourceLocation& location, Level level, Error error)
        : m_location(location)
        , m_level(level)
        , m_error(error)
    {
        log(m_stream, m_location, m_level, m_error);
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
    SourceLocation m_location;
    Level m_level;
    Error m_error;
    bool m_hasError{false};

    // TODO: optimization where it only has a stream when needed by operator <<
    //       (is this possible?)
    ErrorStream m_stream;

    void raise()
    {
        flush();
        report(m_location, m_level, m_error);

        if (is_fatal<Level>::value)
        {
            preterminate(); // hook exists
            terminate();    // no hook exists to avoid terminate (TODO: tests?)
        }
    }

    void flush()
    {
        // TODO: target would be the logger later
        std::cout << m_stream.str();
    }
};

// does nothing but is required for syntax of operator . and <<
// (otherwise compilation fails)
// should be largely optimized away (verified wit toy example in godbolt)
//
// Not strictly needed if we trust optimization (create another proxy in a compile time dead branch).
// Can be removed later

struct EmptyProxy
{
    EmptyProxy()
    {
    }

    EmptyProxy(EmptyProxy&&)
    {
        // should not be used (exists for RVO in C++14)
        std::terminate();
    }

    template <class F, class... Args>
    EmptyProxy& IF_RAISED(const F&, Args&&...)
    {
        return *this;
    }

    template <class T>
    EmptyProxy& operator<<(const T&)
    {
        return *this;
    }
};

template <class Level, class Error>
auto create_proxy(const SourceLocation& location, Level level, const Error& error)
{
    return ErrorProxy<Level, Error>(location, level, error);
}

} // namespace eh