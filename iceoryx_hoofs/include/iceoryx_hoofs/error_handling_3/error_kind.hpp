#pragma once

#include <stdint.h>
#include <type_traits>

namespace eh3
{
using error_level_t = uint32_t;
constexpr error_level_t FATAL_LEVEL{0};

// mandatory fatal error category that always exists
struct Fatal
{
    static constexpr char const* name = "Fatal";

    static constexpr error_level_t value = FATAL_LEVEL;

    explicit operator error_level_t()
    {
        return value;
    }
};

struct PreconditionViolation
{
    static constexpr char const* name = "PreconditionViolation";

    // todo: separate level?
    static constexpr error_level_t value = FATAL_LEVEL;

    explicit operator error_level_t()
    {
        return value;
    }
};

// postconditions and other asserts, not for preconditions
struct DebugAssertViolation
{
    static constexpr char const* name = "DebugAssertViolation";

    // todo: separate level?
    static constexpr error_level_t value = FATAL_LEVEL;

    explicit operator error_level_t()
    {
        return value;
    }
};

template <class T>
struct IsFatal : public std::false_type
{
};

template <>
struct IsFatal<Fatal> : public std::true_type
{
};

template <>
struct IsFatal<PreconditionViolation> : public std::true_type
{
};

template <>
struct IsFatal<DebugAssertViolation> : public std::true_type
{
};

template <class Kind>
bool constexpr isFatal(Kind)
{
    return false;
}

template <>
bool constexpr isFatal<Fatal>(Fatal)
{
    return true;
}

template <>
bool constexpr isFatal<PreconditionViolation>(PreconditionViolation)
{
    return true;
}

template <>
bool constexpr isFatal<DebugAssertViolation>(DebugAssertViolation)
{
    return true;
}

template <typename Kind>
bool constexpr requiresHandling(Kind)
{
    return true;
}

// FATAL always requires handling
bool constexpr requiresHandling(Fatal)
{
    return true;
}

bool constexpr requiresHandling(PreconditionViolation)
{
    return true;
}

bool constexpr requiresHandling(DebugAssertViolation)
{
    return true;
}

// shall indicate serious condition, unable to continue
constexpr Fatal FATAL;

// shall indicate a bug (contract breach by caller)
constexpr PreconditionViolation PRECONDITION_VIOLATION;

// shall indicate a bug (contract breach by callee)
constexpr DebugAssertViolation DEBUG_ASSERT_VIOLATION;

} // namespace eh3