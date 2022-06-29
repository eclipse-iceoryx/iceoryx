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

// TODO: how configurable shall this be?
// currently the user shall have no option to avoid terminate being called for FATAL errors
// however, in the test platform at least we do not want this
#ifndef TEST_PLATFORM
void terminate()
{
    // std::terminate(); // will never return
}
#else
void terminate()
{
    // TODO: abort the thread
}
#endif

} // namespace eh