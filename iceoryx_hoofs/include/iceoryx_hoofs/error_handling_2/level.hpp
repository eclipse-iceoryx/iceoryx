#pragma once

#include <stdint.h>

#include <type_traits>

namespace eh
{
using error_level_t = uint32_t;

struct Fatal
{
    operator error_level_t()
    {
        return 0;
    }

    static constexpr char const* name = "Fatal";
};

template <class T>
struct is_fatal
{
    static constexpr bool value = std::is_same<T, Fatal>::value;
};

} // namespace eh