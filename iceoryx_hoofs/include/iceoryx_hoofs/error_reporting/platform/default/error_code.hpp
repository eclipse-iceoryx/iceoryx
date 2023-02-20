#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_CODE_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_ERROR_CODE_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"

#include "iox/expected.hpp"

namespace iox
{
namespace err
{
// Specialization for expected
// Can be done for other generic error types (e.g. monads)
// overload for expected, but this will not work at a polymorphic level (we need a base class)
template <class T, class Error>
inline error_code_t toCode(const iox::expected<T, Error>& exp)
{
    if (!exp.has_error())
    {
        // not allowed, we cannot properly continue
        IOX_LOG(FATAL) << " Fatal Error - reported expected with a value";
        std::abort();
    }
    return toCode(exp.get_error());
}

template <class T, class Error>
inline error_code_t toModule(const iox::expected<T, Error>& exp)
{
    if (!exp.has_error())
    {
        // not allowed, we cannot properly continue
        IOX_LOG(FATAL) << " Fatal Error - reported expected with a value";
        std::abort();
    }
    return toModule(exp.get_error());
}

} // namespace err
} // namespace iox

#endif
