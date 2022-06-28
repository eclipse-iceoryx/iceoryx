#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

// cannot be used in the real implementation
#include <iostream>
#include <sstream>

namespace eh
{
using ErrorStream = std::stringstream;

template <class Level>
void log(ErrorStream& stream, const SourceLocation& location, Level level)
{
    auto name = level.name;
    stream << name << "@" << location.file << " " << location.line << " " << location.function << std::endl;
}

template <class Level, class Error>
void log(ErrorStream& stream, const SourceLocation& location, Level level, Error error)
{
    auto levelName = level.name;
    auto codeName = error.name();
    auto module = error.module();

    stream << levelName << "@" << location.file << " " << location.line << " " << location.function << " : " << codeName
           << " in module " << module << std::endl;
}

template <class Level>
void log(ErrorStream& stream, const SourceLocation& location, Level level, error_code_t code, module_id_t module)
{
    auto levelName = level.name;
    stream << levelName << "@" << location.file << " " << location.line << " " << location.function << " : " << code
           << " in module " << module << std::endl;
}

} // namespace eh