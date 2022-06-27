#pragma once

#include "error_levels.hpp"
#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

#include "iceoryx_hoofs/error_handling_2/platform/error_reporting.hpp"

#include <iostream>

// note: this file needs to replace the platform specific "error_handling.hpp"

namespace eh
{
// implementation of the test platform handler

// if the mechanism to check for an error with exceptions or some error stack works
// (the latter should always work), there is no reason to implement a more costly runtime dispatch
// for the handling

template <class Level>
void handle(const SourceLocation& location, Level level)
{
    report(location, level);
    throw GenericError();
}

template <class Level, class Error>
void handle(const SourceLocation& location, Level level, const Error& error)
{
    report(location, level, error);
    throw Error(error);
}

template <class Level>
void handle(const SourceLocation& location, Level level, error_code_t code, module_id_t module)
{
    report(location, level, code, module);
    throw GenericError(module, code);
}

// platform specific termination
void terminate()
{
    std::cout << "TERMINATE" << std::endl;
}
} // namespace eh