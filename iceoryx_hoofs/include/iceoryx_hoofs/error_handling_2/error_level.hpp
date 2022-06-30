#pragma once

#include <stdint.h>

#include <type_traits>

namespace eh
{
using error_level_t = uint32_t;

// mandatory level
struct Fatal
{
    static constexpr char const* name = "Fatal";

    operator error_level_t()
    {
        return 0;
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

// FATAL always requires handling
bool constexpr requires_handling(Fatal)
{
    return true;
}

constexpr Fatal FATAL{};

void terminate()
{
    // do we want to make this configurable?
    // in tests it is not desirable to call terminate in all cases
    // std::terminate();
}

} // namespace eh