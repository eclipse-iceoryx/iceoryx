#pragma once

#include "iceoryx_hoofs/error_handling_2/error_level.hpp"

// ***
// *** TO BE IMPLEMENTED BY CLIENT - platform defines error levels
// ***

namespace eh
{
// define which levels shall exist for the platform, Fatal is mandatory and already exists (with code 0)
// codes are currently unused as we can rely on the C++ type system instead (which has advantages
// for e.g. compile time dispatch and type annotations)
struct Error
{
    static constexpr char const* name = "Error";

    operator error_level_t()
    {
        return 1;
    }
};

struct Warning
{
    static constexpr char const* name = "Warning";

    operator error_level_t()
    {
        return 2;
    }
};

constexpr Error ERROR{};
constexpr Warning WARNING{};

// define which levels shall be handled
// could be done in the types themselves with static functions
//
// Could also define type traits but constexpr functions are more convenient here

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

template <>
bool constexpr requires_handling<Error>(Error)
{
    // return false;
    return true;
}

// NB: this cannot be overridden by design
// template <>
// bool constexpr requires_handling<Fatal>(Fatal)
// {
//     return false;
// }

} // namespace eh
