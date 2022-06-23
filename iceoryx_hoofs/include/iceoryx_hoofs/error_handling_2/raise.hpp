#pragma once

#include "proxy.hpp"

#include <iostream>
#include <type_traits>

namespace eh
{
template <class Level>
auto raise(const SourceLocation& location, Level level)
{
    if (!requires_handling(level))
    {
        return ErrorProxy<Level>();
    }

    return ErrorProxy<Level>(location, level);
}

// this will increase the binary size too much since each error has its own class
// (it is ok for level, of which we will have few)
template <class Level, class Error>
auto raise(const SourceLocation& location, Level level, Error error)
{
    if (!requires_handling(level))
    {
        auto e = create_error(error);
        return ErrorProxy2<Level, decltype(e)>();
    }

    auto e = create_error(error);
    return ErrorProxy2<Level, decltype(e)>(location, level, e);
}

template <class Expr, class Level>
auto raise_if(const SourceLocation& location, const Expr& expr, Level level)
{
    if (!requires_handling(level))
    {
        return ErrorProxy<Level>();
    }

    if (expr())
    {
        return ErrorProxy<Level>(location, level);
    }

    // always created, bad
    return ErrorProxy<Level>();
}

template <class Expr, class Level, class Error>
auto raise_if(const SourceLocation& location, const Expr& expr, Level level, Error error)
{
    if (!requires_handling(level))
    {
        auto e = create_error(error);
        return ErrorProxy2<Level, decltype(e)>();
    }

    if (expr())
    {
        // TODO: create only if needed
        auto e = create_error(error);
        return ErrorProxy2<Level, decltype(e)>(location, level, e);
    }

    // always created, bad
    auto e = create_error(error);
    return ErrorProxy2<Level, decltype(e)>();
}

} // namespace eh