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

template <class Level, class Code>
void report(const SourceLocation& location, Level level, Code code)
{
    auto levelName = level.name;
    auto codeName = code.name();
    auto module = code.module();
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

void terminate()
{
    std::cout << "TERMINATE" << std::endl;
}
} // namespace eh