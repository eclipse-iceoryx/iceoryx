#ifndef IOX_HOOFS_ERROR_REPORTING_TYPES_HPP
#define IOX_HOOFS_ERROR_REPORTING_TYPES_HPP

#include <cstdint>

namespace iox
{
namespace err
{

struct ErrorCode
{
    using type = uint32_t;

    constexpr explicit ErrorCode(uint32_t value)
        : value(value)
    {
    }

    type value;

    static constexpr type DEBUG_ASSERT_VIOLATION{0};
    static constexpr type PRECONDITION_VIOLATION{1};

    bool operator==(const ErrorCode& rhs) const
    {
        return value == rhs.value;
    }

    bool operator!=(const ErrorCode& rhs) const
    {
        return !(*this == rhs);
    }
};

struct ModuleId
{
    using type = uint32_t;

    constexpr explicit ModuleId(uint32_t value)
        : value(value)
    {
    }

    type value;

    static constexpr type UNKNOWN{0};

    bool operator==(const ErrorCode& rhs) const
    {
        return value == rhs.value;
    }

    bool operator!=(const ErrorCode& rhs) const
    {
        return !(*this == rhs);
    }
};

} // namespace err
} // namespace iox

#endif
