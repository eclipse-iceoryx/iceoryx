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

// handle unspecific error
template <class Level>
void handle(const SourceLocation& location, Level level)
{
    report(location, level);
}

// handle concrete error
template <class Level, class Error>
void handle(const SourceLocation& location, Level level, const Error& error)
{
    report(location, level, error);
}

// overload for fatal errors - can be done for any defined error level
template <class Error>
void handle(const SourceLocation& location, Fatal level, const Error& error)
{
    std::cout << "FATAL ERROR occurred" << std::endl;
    report(location, level, error);
}

// handle generic error where only the code and module id is known
// (only required if the proxy will not store the error type)
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