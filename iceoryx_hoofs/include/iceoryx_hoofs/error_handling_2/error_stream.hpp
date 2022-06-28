#pragma once

// cannot be used in the real implementation
#include <sstream>

namespace eh
{
// potential abstractions could use a limited buffer etc.
using ErrorStream = std::stringstream;
}; // namespace eh