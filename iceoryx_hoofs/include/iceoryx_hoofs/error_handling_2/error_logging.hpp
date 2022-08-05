#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

// cannot be used in the real implementation (to be replaced by LogStream)
#include <iostream>
#include <sstream>

namespace eh
{

// will be handled by logger later on
using ErrorStream = std::stringstream;

inline ErrorStream& errorStream()
{
    thread_local ErrorStream stream;
    return stream;
}

inline void flush()
{
    errorStream().flush();
}

template <class Level>
void log(const SourceLocation& location, Level level)
{
    errorStream() << level.name << "@" << location.file << " " << location.line << " " << location.function
                  << std::endl;
}

template <class Level, class Error>
void log(const SourceLocation& location, Level level, Error error)
{
    auto levelName = level.name;
    auto codeName = error.name();
    auto module = error.module();

    errorStream() << levelName << "@" << location.file << " " << location.line << " " << location.function << " : "
                  << codeName << " in module " << module << std::endl;
}

template <class Level>
void log(const SourceLocation& location, Level level, error_code_t code, module_id_t module)
{
    auto levelName = level.name;
    errorStream() << levelName << "@" << location.file << " " << location.line << " " << location.function << " : "
                  << code << " in module " << module << std::endl;
}

} // namespace eh