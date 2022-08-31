#pragma once

#include <stdint.h>

#include <type_traits>

namespace eh
{
using error_level_t = uint32_t;

constexpr error_level_t FATAL_LEVEL{0};

// mandatory level
struct Fatal
{
    static constexpr char const* name = "Fatal";

    static constexpr error_level_t value = FATAL_LEVEL;

    explicit operator error_level_t()
    {
        return value;
    }
};

template <class T>
struct is_fatal : public std::false_type
{
};

template <>
struct is_fatal<Fatal> : public std::true_type
{
};

template <class Level>
bool constexpr isFatal(Level)
{
    return false;
}

template <>
bool constexpr isFatal<Fatal>(Fatal)
{
    return true;
}

// FATAL always requires handling
bool constexpr requiresHandling(Fatal)
{
    return true;
}

constexpr Fatal FATAL{};

} // namespace eh