#pragma once

#include "iceoryx_hoofs/error_handling_3/error.hpp"
#include "iceoryx_hoofs/error_handling_3/location.hpp"

#include "iceoryx_hoofs/error_handling_3/platform/error_level.hpp"

#include <iostream>

namespace eh3
{
// platform specific, static dispatch (optimized away)

template <class Level, class Error>
void report(const SourceLocation&, Level, const Error&)
{
    std::cout << "TEST REPORT level" << std::endl;
}

template <class Error>
void report(const SourceLocation&, eh3::Fatal, const Error&)
{
    std::cout << "TEST REPORT fatal" << std::endl;
}

void panic()
{
    std::cout << "TEST PANIC" << std::endl;
}

} // namespace eh3
