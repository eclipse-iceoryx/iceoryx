#pragma once

#include <stdint.h>

#include <type_traits>

namespace eh
{
using error_level_t = uint32_t;

struct Fatal
{
    static constexpr char const* name = "Fatal";

    operator error_level_t()
    {
        return 0;
    }
};

template <class T>
struct is_fatal
{
    static constexpr bool value = std::is_same<T, Fatal>::value;
};

bool constexpr requires_handling(Fatal)
{
    return true;
}

constexpr Fatal FATAL{};

} // namespace eh