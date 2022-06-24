#pragma once

#include "iceoryx_hoofs/error_handling_2/level.hpp"

namespace eh
{
// define which levels shall exist, Fatal is mandatory and already exists (with code 0)
// codes are currently unused as we can rely on the C++ type system
struct Error
{
    operator error_level_t()
    {
        return 1;
    }

    static constexpr char const* name = "Error";
};

struct Warning
{
    operator error_level_t()
    {
        return 2;
    }

    static constexpr char const* name = "Warning";
};

constexpr Fatal fatal{};
constexpr Error error{};
constexpr Warning warning{};

// define which levels shall be handled
// could be done in the types themselves with static functions
template <class Level>
bool constexpr requires_handling(Level)
{
    return true;
}

// exclude warnings from handling at compile time(!)
template <>
bool constexpr requires_handling<Warning>(Warning)
{
    return false;
}

} // namespace eh

// convenience
#define FATAL eh::fatal
#define ERROR eh::error
#define WARNING eh::warning
