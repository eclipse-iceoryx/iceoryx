#pragma once

#include "iceoryx_hoofs/error_handling_2/level.hpp"

namespace eh
{
// define which levels shall exist for the platform, Fatal is mandatory and already exists (with code 0)
// codes are currently unused as we can rely on the C++ type system instead (which has advantages
// for e.g. compile time dispatch)
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

// for convenience, beware of ambiguity (not much of a problem with only one platform defined for
// any given binary)
#define FATAL eh::fatal
#define ERROR eh::error
#define WARNING eh::warning
