#ifndef IOX_HOOFS_ERROR_REPORTING_CONFIGURATION_HPP
#define IOX_HOOFS_ERROR_REPORTING_CONFIGURATION_HPP

#include <type_traits>

namespace iox
{
namespace err
{

struct ConfigurationTag
{
};

// can be specialized here to change parameters at compile time
template <typename T>
struct ConfigurationParameters
{
    static_assert(std::is_same<T, ConfigurationTag>::value, "");

    static constexpr bool CHECK_PRECONDITIONS{true};
    static constexpr bool CHECK_ASSUMPTIONS{true};
    static constexpr bool CHECK_UNREACHABLE{true};
};

using Configuration = ConfigurationParameters<ConfigurationTag>;

} // namespace err
} // namespace iox

#endif
