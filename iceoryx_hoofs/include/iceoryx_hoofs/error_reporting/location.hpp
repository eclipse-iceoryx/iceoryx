#pragma once

#include <stdint.h>

namespace err
{
struct SourceLocation
{
    const char* file{nullptr};
    uint32_t line{0};
    const char* function{nullptr};
};

} // namespace err

// clang-format off
#define CURRENT_SOURCE_LOCATION \
    err::SourceLocation{ __FILE__, __LINE__, __func__}
// clang-format on
