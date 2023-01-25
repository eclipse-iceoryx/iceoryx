#pragma once

#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include <type_traits>

// ***
// *** IMPLEMENTED BY PLATFORM
// ***

namespace iox
{
namespace err
{

enum class ErrorLevel : error_level_t
{
    ERROR = FATAL_LEVEL + 1,
    WARNING
};

// prefer types to avoid switch statements and the like and allow annotations (such as name here)
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

constexpr Error RUNTIME_ERROR{};
constexpr Warning WARNING{};

// exclude warnings from handling at compile time
template <>
bool constexpr requiresHandling<Warning>(Warning)
{
    return false;
}

} // namespace err
} // namespace iox
