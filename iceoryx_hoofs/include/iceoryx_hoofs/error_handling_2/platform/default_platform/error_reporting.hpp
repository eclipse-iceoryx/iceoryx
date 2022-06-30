#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

#include "iceoryx_hoofs/error_handling_2/platform/error_levels.hpp"

#include <iostream>

namespace eh
{
// platform specific handling

// static dispatch, cannot be changed at runtime (fewer indirections)
// default does nothing (will be optimized away)

template <class Level, class Error>
void report(const SourceLocation& location, Level level, const Error& error)
{
    (void)location;
    (void)level;
    (void)error;
}

// can overload for fatal errors - can be done for any defined error level
template <class Error>
void report(const SourceLocation& location, Fatal level, const Error& error)
{
    (void)location;
    (void)level;
    (void)error;
    std::cout << "FATAL ERROR occurred" << std::endl;
}

// platform specific termination
void preterminate()
{
}
} // namespace eh