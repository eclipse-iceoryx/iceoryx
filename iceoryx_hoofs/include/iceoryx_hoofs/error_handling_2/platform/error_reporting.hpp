#pragma once

#include "error_levels.hpp"
#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

#include <iostream>

namespace eh
{
template <class Level>
void report(const SourceLocation& location, Level level)
{
    auto name = level.name;
    std::cout << name << "@" << location.file << " " << location.line << " " << location.function << std::endl;
}

template <class Level, class Error>
void report(const SourceLocation& location, Level level, Error error)
{
    auto levelName = level.name;
    auto codeName = error.name();
    auto module = error.module();

    std::cout << levelName << "@" << location.file << " " << location.line << " " << location.function << " : "
              << codeName << " in module " << module << std::endl;
}

template <class Level>
void report(const SourceLocation& location, Level level, error_code_t code, module_id_t module)
{
    // we cannot identify the code by name in this way (we do not get the enum type back from the
    // module_id - maybe with some meta-programming)
    // note that this is not required with typed reporting with concrete Error type
    auto levelName = level.name;
    std::cout << levelName << "@" << location.file << " " << location.line << " " << location.function << " : " << code
              << " in module " << module << std::endl;
}

} // namespace eh