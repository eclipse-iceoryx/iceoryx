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
    auto levelName = level.name;
    auto codeName = error_name(code);
    std::cout << levelName << "@" << location.file << " " << location.line << " " << location.function << " : "
              << codeName << " in module " << module << std::endl;
}

} // namespace eh