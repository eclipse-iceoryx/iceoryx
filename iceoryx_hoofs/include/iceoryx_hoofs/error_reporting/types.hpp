#ifndef IOX_HOOFS_ERROR_REPORTING_TYPES_HPP
#define IOX_HOOFS_ERROR_REPORTING_TYPES_HPP

#include <cstdint>

namespace iox
{
namespace err
{

// These are lightweight regular read/write types that do not require encapsulation (no invariants
// can be broken).

struct ErrorCode
{
    using type = uint32_t;

    static constexpr type DEBUG_ASSERT_VIOLATION{0};
    static constexpr type PRECONDITION_VIOLATION{1};

    type value;

    constexpr explicit ErrorCode(uint32_t value)
        : value(value)
    {
    }

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

    static constexpr type UNKNOWN{0};

    type value;

    constexpr explicit ModuleId(uint32_t value = UNKNOWN)
        : value(value)
    {
    }

    bool operator==(const ModuleId& rhs) const
    {
        return value == rhs.value;
    }

    bool operator!=(const ModuleId& rhs) const
    {
        return !(*this == rhs);
    }
};

} // namespace err
} // namespace iox

#endif
