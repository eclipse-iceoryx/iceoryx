#pragma once

#include <stdint.h>

namespace eh3
{
struct SourceLocation
{
    const char* file{nullptr};
    uint32_t line{0};
    const char* function{nullptr};
};

} // namespace eh3

// clang-format off
#define CURRENT_SOURCE_LOCATION \
    eh3::SourceLocation{ __FILE__, __LINE__, __func__}
// clang-format on
