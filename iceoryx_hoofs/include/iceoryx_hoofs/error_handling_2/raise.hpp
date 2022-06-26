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
        return UnspecificErrorProxy<Level>();
    }

    return UnspecificErrorProxy<Level>(location, level);
}

template <class Level, class Error>
auto raise(const SourceLocation& location, Level level, Error error)
{
    if (!requires_handling(level))
    {
        auto e = create_error(error);
        return ErrorProxy<Level, decltype(e)>();
    }

    auto e = create_error(error);
    return ErrorProxy<Level, decltype(e)>(location, level, e);
}

template <class Expr, class Level>
auto raise_if(const SourceLocation& location, const Expr& expr, Level level)
{
    if (!requires_handling(level))
    {
        return UnspecificErrorProxy<Level>();
    }

    if (expr())
    {
        return UnspecificErrorProxy<Level>(location, level);
    }

    return UnspecificErrorProxy<Level>();
}

template <class Expr, class Level, class Error>
auto raise_if(const SourceLocation& location, const Expr& expr, Level level, Error error)
{
    if (!requires_handling(level))
    {
        auto e = create_error(error);
        return ErrorProxy<Level, decltype(e)>();
    }

    if (expr())
    {
        // TODO: create only if needed
        auto e = create_error(error);
        return ErrorProxy<Level, decltype(e)>(location, level, e);
    }

    auto e = create_error(error);
    return ErrorProxy<Level, decltype(e)>();
}

} // namespace eh