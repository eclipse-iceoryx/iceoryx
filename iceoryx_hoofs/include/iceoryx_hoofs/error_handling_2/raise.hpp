#pragma once

#include "proxy.hpp"

#include <iostream>
#include <type_traits>

namespace eh
{
// false type overloads discard arguments and create an EmptyProxy
// true type overloads create a real proxy
// This is needed for compile-time dispatch (determines return type) and cannot be
// done with constexpr if (due to different return types on different branches).
//
// the goal is to

template <class Level>
auto create_proxy(const SourceLocation& location, Level level, std::true_type)
{
    return UnspecificErrorProxy<Level>(location, level);
}

template <class Level>
auto create_proxy(const SourceLocation&, Level, std::false_type)
{
    return EmptyProxy();
}

template <class Expr, class Level>
auto create_proxy(const Expr& expr, const SourceLocation& location, Level level, std::true_type)
{
    if (expr())
    {
        return UnspecificErrorProxy<Level>(location, level);
    }
    return UnspecificErrorProxy<Level>();
}

template <class Expr, class Level>
auto create_proxy(const Expr&, const SourceLocation&, Level, std::false_type)
{
    return EmptyProxy();
}

template <class Level, class Error>
auto create_proxy(const SourceLocation& location, Level level, const Error& error, std::true_type)
{
    auto e = create_error(error);
    return ErrorProxy<Level, decltype(e)>(location, level, e);
}

template <class Level, class Error>
auto create_proxy(const SourceLocation&, Level, const Error&, std::false_type)
{
    return EmptyProxy();
}

template <class Expr, class Level, class Error>
auto create_proxy(const Expr& expr, const SourceLocation& location, Level level, const Error& error, std::true_type)
{
    auto e = create_error(error);
    if (expr())
    {
        return ErrorProxy<Level, decltype(e)>(location, level, e);
    }
    return ErrorProxy<Level, decltype(e)>();
}

template <class Expr, class Level, class Error>
auto create_proxy(const Expr&, const SourceLocation&, Level, const Error&, std::false_type)
{
    return EmptyProxy();
}

// raising the error creates the proxy based on static dispatch (determines the proxy type)
// raise_if also uses dynamic dispatch on expr (to determine the proxy ctor to be called)
//
// note that the Error type raised is generic, but we will usually use lightweight codes (enum class).

template <class Level>
auto raise(const SourceLocation& location, Level level)
{
    constexpr std::integral_constant<bool, requires_handling(level)> rh;
    return create_proxy(location, level, rh);
}

template <class Level, class Error>
auto raise(const SourceLocation& location, Level level, Error error)
{
    constexpr std::integral_constant<bool, requires_handling(level)> rh;
    return create_proxy(location, level, error, rh);
}

template <class Expr, class Level>
auto raise_if(const SourceLocation& location, const Expr& expr, Level level)
{
    constexpr std::integral_constant<bool, requires_handling(level)> rh;
    return create_proxy(expr, location, level, rh);
}

template <class Expr, class Level, class Error>
auto raise_if(const SourceLocation& location, const Expr& expr, Level level, Error error)
{
    constexpr std::integral_constant<bool, requires_handling(level)> rh;
    return create_proxy(expr, location, level, error, rh);
}

} // namespace eh