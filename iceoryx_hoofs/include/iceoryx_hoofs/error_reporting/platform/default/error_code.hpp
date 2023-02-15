#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_CODE_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_CODE_HPP

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/error_reporting/error.hpp"

// defined here in platform specific and not in the general code because it
// depends on expected and we generally do not want this

namespace iox
{
namespace err
{

// forward declaration
void panic();

// Specialization for expected
// Can be done for other generic error types (e.g. monads)
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

template <class T, class Error>
inline error_code_t toModule(const iox::cxx::expected<T, Error>& exp)
{
    if (!exp.has_error())
    {
        // not allowed
        panic();
        std::abort();
    }
    return toModule(exp.get_error());
}

} // namespace err
} // namespace iox

#endif
