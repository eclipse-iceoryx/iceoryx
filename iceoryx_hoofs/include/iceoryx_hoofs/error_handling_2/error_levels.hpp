#pragma once

#include <stdint.h>

namespace eh
{
using error_level_t = uint32_t;

#if 0
// not needed and not ideal for compile time dispatch
// enforce type safety
enum class ErrorLevel : error_level_t
{
    Fatal = 0,
    Error = 1,
    Warning = 2
};

#endif

static const char* errorLevels[] = {"Fatal", "Error", "Warning"};

// note that the array apporach is by design (instead of switch) to allow a fast lookup
const char* error_level_to_name(error_level_t level)

{
    return errorLevels[level];
}

// tag types

struct Fatal_t
{
    operator error_level_t()
    {
        return 0;
    }
};

struct Error_t
{
    operator error_level_t()
    {
        return 1;
    }
};

struct Warning_t
{
    operator error_level_t()
    {
        return 2;
    }
};

constexpr Fatal_t Fatal{};
constexpr Error_t Error{};
constexpr Warning_t Warning{};

} // namespace eh


// convenience
#define FATAL eh::Fatal
#define ERROR eh::Error
#define WARNING eh::Warning
