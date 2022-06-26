#pragma once

#include "error_code.hpp"
#include "location.hpp"
#include "platform/error_handling.hpp"
#include "platform/error_levels.hpp"

#include <iostream>
#include <type_traits>

// cannot be used long-term
#include <sstream>

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
        , code(0) // TODO: fix the no-error code
        , module(0)
    {
        error = true;
    }

    template <class Code>
    UnspecificErrorProxy(const SourceLocation& location, Level level, Code code)
        : location(location)
        , level(level)
        , code(code.code())
    {
        error = true;
    }

    UnspecificErrorProxy(UnspecificErrorProxy&&)
    {
        // should not be used (exists for RVO in C++14)
        std::terminate();
    }

    ~UnspecificErrorProxy() noexcept(false)
    {
        if (error)
        {
            // can be compile time dispatched later
            if (code > 0)
            {
                handle(location, level, code, module);
                // we need our own stream to do this, likely bounded
                std::cout << stream.str();
            }
            else
            {
                handle(location, level);
                std::cout << stream.str();
            }
            if (is_fatal<Level>::value)
            {
                terminate();
            }
        }
    }

    template <class F, class... Args>
    UnspecificErrorProxy& and_call(const F& f, Args&&... args)
    {
        if (error)
        {
            f(std::forward<Args>(args)...);
        }
        return *this;
    }

    template <class T>
    UnspecificErrorProxy& operator<<(const T& msg)
    {
        stream << msg;
        return *this;
    }

  private:
    SourceLocation location;
    Level level;
    error_code_t code;
    module_id_t module;
    bool error{false};

    // temporary solution (LogStream?)
    std::stringstream stream;
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
    // this is no prolem for us since we do not use exceptions in our
    // handler and even if we do during testing (ONLY!) we should not have
    // a problem as long as we never use IOX_RAISE in dtors
    // for non-fatal errors

    ~ErrorProxy() noexcept(false)
    {
        raise();
    }

    template <class F, class... Args>
    ErrorProxy& and_call(const F& f, Args&&... args)
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
        stream << msg;
        return *this;
    }

  private:
    SourceLocation location;
    Level level;
    Error error;
    bool hasError{false};

    // temporary solution (LogStream?)
    std::stringstream stream;

    void raise()
    {
        if (hasError)
        {
            handle(location, level, error);
            // we need our own stream to do this, likely bounded
            // TODO: abstract logstream and propagate to handle
            std::cout << stream.str();

            if (is_fatal<Level>::value)
            {
                terminate();
            }
        }
    }
};

} // namespace eh