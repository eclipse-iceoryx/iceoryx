#pragma once

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/error_reporting/error.hpp"

// defined here in platform specific and not in the general code because it
// depends on expected and we generally do not want this

namespace iox
{
namespace err
{

// forward declared
void panic();

template <class Error>
inline error_code_t toCode(const Error& error)
{
    return error.code();
}

template <>
inline error_code_t toCode<error_code_t>(const error_code_t& error)
{
    return error;
}

// overload for expected, but this will not work at a polymorphic level (we need a base class)
template <class T, class Error>
inline error_code_t toCode(const iox::cxx::expected<T, Error>& exp)
{
    if (!exp.has_error())
    {
        // not allowed
        panic();
        std::abort();
    }
    return toCode(exp.get_error());
}

template <class Error>
inline bool reportError(const Error&)
{
    return true;
}

} // namespace err
} // namespace iox