#ifndef IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_CONFIGURATION_HPP
#define IOX_HOOFS_ERROR_REPORTING_PLATFORM_DEFAULT_CONFIGURATION_HPP

#include "iceoryx_hoofs/error_reporting/configuration.hpp"

namespace iox
{

namespace err
{

// Specialize to change the checks (and other options if needed)at compile time.
// this can later also be done depending on a #define to select a header
// but we should avoid to have a define for each option.
template <>
struct ConfigurationParameters<ConfigurationTag>
{
    static constexpr bool CHECK_PRECONDITIONS{true};
    static constexpr bool CHECK_ASSUMPTIONS{true};
    static constexpr bool CHECK_UNREACHABLE{true};
};

} // namespace err
} // namespace iox

#endif