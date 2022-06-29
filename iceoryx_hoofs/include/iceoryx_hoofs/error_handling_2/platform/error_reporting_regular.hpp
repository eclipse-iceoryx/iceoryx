#pragma once

#include "error_levels.hpp"
#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

#include <iostream>

namespace eh
{
// platform specific handling
// this implementation redirects to reporting which reports to console

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

template <class Level>
void report(const SourceLocation& location, Level level, error_code_t code, module_id_t module)
{
    (void)location;
    (void)level;
    (void)code;
    (void)module;
}

// platform specific termination
void preterminate()
{
    std::cout << "TERMINATE" << std::endl;
}
} // namespace eh