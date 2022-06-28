#pragma once

#include "error_levels.hpp"
#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

#include "iceoryx_hoofs/error_handling_2/platform/error_reporting.hpp"

#include <iostream>

namespace eh
{
// we can have handlers for every level but not with typed errors(!)
// function pointer indirection, can be changed at runtime

// need mutex/atomic protection if they can change while running
struct Handler
{
    using fatal_handler_t = void (*)(const SourceLocation&, Fatal, error_code_t, module_id_t);
    using error_handler_t = void (*)(const SourceLocation&, Error, error_code_t, module_id_t);
    using warning_handler_t = void (*)(const SourceLocation&, Warning, error_code_t, module_id_t);

    fatal_handler_t fatal = report<Fatal>;
    error_handler_t error = report<Error>;
    warning_handler_t warning = report<Warning>;

    void operator()(const SourceLocation& location, Fatal level, error_code_t code, module_id_t module)
    {
        fatal(location, level, code, module);
    }

    void operator()(const SourceLocation& location, Error level, error_code_t code, module_id_t module)
    {
        error(location, level, code, module);
    }

    void operator()(const SourceLocation& location, Warning level, error_code_t code, module_id_t module)
    {
        warning(location, level, code, module);
    }
};

// could also be a singleton etc.
Handler g_handler;

// handle unspecific error
template <class Level>
void handle(const SourceLocation& location, Level level)
{
    g_handler(location, level, 0, INVALID_MODULE);
}

// handle concrete error
template <class Level, class Error>
void handle(const SourceLocation& location, Level level, const Error& error)
{
    g_handler(location, level, error.code(), error.module());
}

template <class Level>
void handle(ErrorStream& stream, const SourceLocation& location, Level level, error_code_t code, module_id_t module)
{
    g_handler(location, level, code, module);
}

// platform specific termination
void preterminate()
{
    std::cout << "TERMINATE" << std::endl;
}
} // namespace eh
