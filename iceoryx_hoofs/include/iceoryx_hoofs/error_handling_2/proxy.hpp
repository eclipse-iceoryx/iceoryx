#pragma once

#include "error_code.hpp"
#include "error_stream.hpp"
#include "location.hpp"

#include "platform/error_handling.hpp"

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
        : location(location)
        , level(level)
        , code(0)
        , module(eh::INVALID_MODULE)
    {
        hasError = true;
    }

    template <class Code>
    UnspecificErrorProxy(const SourceLocation& location, Level level, Code code)
        : location(location)
        , level(level)
        , code(code.code())
    {
        hasError = true;
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

    // TODO: consider a slightly different WITH_ERROR(Proxy&, args) call in addition
    //       consider syntax (case etc.)
    template <class F, class... Args>
    UnspecificErrorProxy& IF_RAISED(const F& f, Args&&... args)
    {
        if (hasError)
        {
            f(*this, std::forward<Args>(args)...);
        }
        return *this;
    }

    template <class T>
    UnspecificErrorProxy& operator<<(const T& msg)
    {
        if (hasError)
        {
            stream << msg;
        }
        return *this;
    }

  private:
    SourceLocation location;
    Level level;
    error_code_t code;
    module_id_t module;
    bool hasError{false};

    ErrorStream stream;

    void raise()
    {
        if (hasError)
        {
            if (module != INVALID_MODULE)
            {
                // generic error
                handle(stream, location, level, code, module);
            }
            else
            {
                // unknown error code (none provided)
                handle(stream, location, level);
            }
            if (is_fatal<Level>::value)
            {
                preterminate();
                // std::terminate(); // TODO: ensure it is called in regular mode
            }
        }
    }
};

template <class Level, class Error>
struct ErrorProxy
{
    ErrorProxy()
    {
    }
    ErrorProxy(const SourceLocation& location, Level level, Error error)
        : location(location)
        , level(level)
        , error(error)
    {
        hasError = true;
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
        if (hasError)
        {
            f(std::forward<Args>(args)...);
        }
        return *this;
    }

    template <class T>
    ErrorProxy& operator<<(const T& msg)
    {
        if (hasError)
        {
            stream << msg;
        }
        return *this;
    }

  private:
    SourceLocation location;
    Level level;
    Error error;
    bool hasError{false};


    ErrorStream stream;

    void raise()
    {
        if (hasError)
        {
            // log always here with logstream (and add location, level etc.)
            // remove stream from handle
            std::cout << stream.str();
            handle(stream, location, level, error);

            if (is_fatal<Level>::value)
            {
                preterminate();
                // std::terminate(); // TODO: ensure it is called in regular mode
            }
        }
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