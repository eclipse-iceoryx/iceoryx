#pragma once

#include "error_levels.hpp"
#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

#include "iceoryx_hoofs/error_handling_2/platform/error_reporting.hpp"

#include <iostream>

namespace eh
{
// platform specific handling
// this implementation redirects to reporting which reports to console

// TODO: optimize parameter passing
template <class Level>
void handle(const SourceLocation& location, Level level)
{
    report(location, level);
}

template <class Level, class Error>
void handle(const SourceLocation& location, Level level, Error error)
{
    report(location, level, error);
}

template <class Level>
void handle(const SourceLocation& location, Level level, error_code_t code, module_id_t module)
{
    report(location, level, code, module);
}

// platform specific termination
void terminate()
{
    std::cout << "TERMINATE" << std::endl;
}
} // namespace eh