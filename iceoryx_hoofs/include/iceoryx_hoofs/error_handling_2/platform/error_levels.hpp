#pragma once

#include "iceoryx_hoofs/error_handling_2/error_level.hpp"
#include <type_traits>

// ***
// *** TO BE IMPLEMENTED BY CLIENT - platform defines error levels
// ***

namespace eh
{
// define which levels shall exist for the platform, Fatal is mandatory and already exists (with code 0)
// codes are currently unused as we can rely on the C++ type system instead (which has advantages
// for e.g. compile time dispatch and type annotations)
enum class ErrorLevel : error_level_t
{
    ERROR = FATAL_LEVEL + 1,
    WARNING
};

// prefer types to avoid switch satements and the like and allow annotations (such as name here)
struct Error
{
    static constexpr char const* name = "Error";
    static constexpr error_level_t value = static_cast<error_level_t>(ErrorLevel::ERROR);

    explicit operator error_level_t()
    {
        return value;
    }
};

struct Warning
{
    static constexpr char const* name = "Warning";
    static constexpr error_level_t value = static_cast<error_level_t>(ErrorLevel::WARNING);

    explicit operator error_level_t()
    {
        return value;
    }
};

constexpr Error ERROR{};
constexpr Warning WARNING{};

// define which levels shall be handled
// could be done in the types themselves with static functions
//
// Could also define type traits but constexpr functions are more convenient here

template <class Level>
bool constexpr requiresHandling(Level)
{
    return true;
}

// exclude warnings from handling at compile time(!)
template <>
bool constexpr requiresHandling<Warning>(Warning)
{
    return true;
}

template <>
bool constexpr requiresHandling<Error>(Error)
{
    return true;
}

} // namespace eh
