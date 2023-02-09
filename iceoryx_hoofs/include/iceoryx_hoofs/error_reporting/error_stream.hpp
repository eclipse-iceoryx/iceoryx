#pragma once

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

// cannot be used in the real implementation (to be replaced by LogStream)
#include <iostream>
#include <sstream>

namespace iox
{
namespace err
{

// TODO: use logger

using ErrorStream = std::stringstream;

inline ErrorStream& errorStream()
{
    thread_local ErrorStream stream;
    return stream;
}

void flush()
{
    auto& s = errorStream();
    s.flush();
    std::cerr << s.str() << std::endl;
    s.str("");
}

} // namespace err
} // namespace iox
