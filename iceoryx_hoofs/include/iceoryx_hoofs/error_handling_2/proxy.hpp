#pragma once

#include "error_codes.hpp"  // this must be provided by the module
#include "error_levels.hpp" // do we want this dependency?
#include "location.hpp"

#include <iostream>
#include <type_traits>

// cannot be used long-term
#include <sstream>

namespace eh
{
void terminate()
{
    std::cout << "TERMINATE" << std::endl;
}

template <class T>
struct is_fatal
{
    static constexpr bool value = std::is_same<T, Fatal_t>::value;
};

// alternatively map to default error code
template <class Level>
void report(const SourceLocation& location, Level level)
{
    auto name = error_level_to_name(level);
    std::cout << name << "@" << location.file << " " << location.line << " " << location.function << std::endl;
}

template <class Level, class Code>
void report(const SourceLocation& location, Level level, Code code)
{
    auto levelName = error_level_to_name(level);
    auto codeName = error_code_to_name(code);
    std::cout << levelName << "@" << location.file << " " << location.line << " " << location.function << " : "
              << codeName << std::endl;
}

template <class Level>
void report(const SourceLocation& location, Level level, error_code_t code)
{
    auto levelName = error_level_to_name(level);
    auto codeName = error_code_to_name(code);
    std::cout << levelName << "@" << location.file << " " << location.line << " " << location.function << " : "
              << codeName << std::endl;
}

template <class Level>
struct ErrorProxy
{
    ErrorProxy()
    {
    }

    ErrorProxy(const SourceLocation& location, Level level)
        : location(location)
        , level(level)
        , code(0) // fix magic number for no code
    {
        error = true;
    }

    template <class Code>
    ErrorProxy(const SourceLocation& location, Level level, Code code)
        : location(location)
        , level(level)
        , code(error_code_to_num(code))
    {
        error = true;
    }

    ErrorProxy(ErrorProxy&&)
    {
        // should not be used (exists for RVO in C++14)
        std::terminate();
    }

    ~ErrorProxy()
    {
        if (error)
        {
            // can be compile time dispatched later
            if (code > 0)
            {
                report(location, level, code);
                // we need our own stream to do this, likely bounded
                std::cout << stream.str();
            }
            else
            {
                report(location, level);
                std::cout << stream.str();
            }
            if (is_fatal<Level>::value)
            {
                terminate();
            }
        }
    }

    template <class F, class... Args>
    ErrorProxy& and_call(const F& f, Args&&... args)
    {
        if (error)
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
    error_code_t code;
    bool error{false};

    // temporary solution (LogStream?)
    std::stringstream stream;
};

} // namespace eh