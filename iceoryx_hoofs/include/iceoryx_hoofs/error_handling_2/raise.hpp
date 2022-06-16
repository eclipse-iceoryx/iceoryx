#pragma once

#include "proxy.hpp"

#include <iostream>
#include <type_traits>

// rename raise.hpp

namespace eh
{

#if 0
template <class Level>
void raise(const SourceLocation& location, Level level)
{
    report(location, level);
    // TODO: check whether it is a compile time if as expected
    // could be constexpr explicitly with C++17 but should be optimized like this anyway
    if (is_fatal<Level>::value)
    {
        terminate();
    }
}

template <class Level, class Code>
void raise(const SourceLocation& location, Level level, Code code)
{
    report(location, level, code);
    if (is_fatal<Level>::value)
    {
        terminate();
    }
}
template <class Expr, class Level>
void raise_if(const SourceLocation& location, const Expr& expr, Level level)
{
    // it is possible to add a compile time condition that causes expr() never to be evaluated
    // (e.g. for certain error levels)
    if (expr())
    {
        report(location, level);
        if (is_fatal<Level>::value)
        {
            terminate();
        }
    }
}

template <class Expr, class Level, class Code>
void raise_if(const SourceLocation& location, const Expr& expr, Level level, Code code)
{
    // it is possible to add a compile time condition that causes expr() never to be evaluated
    // (e.g. for certain error levels)
    if (expr())
    {
        report(location, level, code);
        if (is_fatal<Level>::value)
        {
            terminate();
        }
    }
}
#else
template <class Level>
ErrorProxy<Level> raise(const SourceLocation& location, Level level)
{        
    return ErrorProxy<Level>(location, level);
}

template <class Level, class Code>
ErrorProxy<Level> raise(const SourceLocation& location, Level level, Code code)
{    
    return ErrorProxy<Level>(location, level, code);
}

template <class Expr, class Level>
ErrorProxy<Level> raise_if(const SourceLocation& location, const Expr& expr, Level level)
{    
    if (expr())
    {
        return ErrorProxy<Level>(location, level);
    }

    // always created, bad
    return ErrorProxy<Level>();
}

template <class Expr, class Level, class Code>
ErrorProxy<Level> raise_if(const SourceLocation& location, const Expr& expr, Level level, Code code)
{    
    if (expr())
    {
        return ErrorProxy<Level>(location, level, code);
    }

    // always created, bad
    return ErrorProxy<Level>();
}
#endif

} // namespace eh