#pragma once

#include "error_code.hpp"
#include "error_logging.hpp"
#include "location.hpp"

#include "platform/error_reporting.hpp"

#include <iostream>
#include <type_traits>

namespace eh
{
template <class Level>
struct UnspecificErrorProxy
{
    UnspecificErrorProxy()
    {
    }

    UnspecificErrorProxy(const SourceLocation& location, Level level)
        : m_location(location)
        , m_level(level)
        , m_code(0)
        , m_module(eh::INVALID_MODULE)
    {
        m_hasError = true;
        log(m_stream, m_location, m_level);
    }

    template <class Code>
    UnspecificErrorProxy(const SourceLocation& location, Level level, Code code)
        : m_location(location)
        , m_level(level)
        , m_code(code.code())
        , m_module(code.module())
    {
        m_hasError = true;
        log(m_stream, m_location, m_level, m_code);
    }

    UnspecificErrorProxy(UnspecificErrorProxy&&)
    {
        // should not be used (exists for RVO in C++14)
        std::terminate();
    }

    ~UnspecificErrorProxy() noexcept(false)
    {
        raise();
    }

    template <class F, class... Args>
    UnspecificErrorProxy& IF_RAISED(const F& f, Args&&... args)
    {
        if (m_hasError)
        {
            f(*this, std::forward<Args>(args)...);
        }
        return *this;
    }

    template <class T>
    UnspecificErrorProxy& operator<<(const T& value)
    {
        if (m_hasError)
        {
            m_stream << value;
        }
        return *this;
    }

  private:
    SourceLocation m_location;
    Level m_level;
    error_code_t m_code;
    module_id_t m_module;
    bool m_hasError{false};

    ErrorStream m_stream;

    void raise()
    {
        if (m_hasError)
        {
            if (m_module != INVALID_MODULE)
            {
                // generic error
                flush();
                handle(m_location, m_level, m_code, m_module);
            }
            else
            {
                // unknown error code (none provided)
                flush();
                handle(m_location, m_level);
            }
            if (is_fatal<Level>::value)
            {
                preterminate();
                // std::terminate(); // TODO: ensure it is called in regular mode
            }
        }
    }

    void flush()
    {
        // TODO: target would be the logger later
        std::cout << m_stream.str();
    }
};

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
        m_hasError = true;
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
        if (m_hasError)
        {
            f(std::forward<Args>(args)...);
        }
        return *this;
    }

    template <class T>
    ErrorProxy& operator<<(const T& value)
    {
        if (m_hasError)
        {
            m_stream << value;
        }
        return *this;
    }

  private:
    SourceLocation m_location;
    Level m_level;
    Error m_error;
    bool m_hasError{false};

    ErrorStream m_stream;

    void raise()
    {
        if (m_hasError)
        {
            flush();
            handle(m_location, m_level, m_error);

            if (is_fatal<Level>::value)
            {
                preterminate();
                // std::terminate(); // TODO: ensure it is called in regular mode
            }
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

} // namespace eh