#pragma once

#include <stdint.h>

namespace iox
{
namespace err
{
struct SourceLocation
{
    const char* file{nullptr};
    int line{0};
    const char* function{nullptr};
};

} // namespace err
} // namespace iox

// clang-format off
#define CURRENT_SOURCE_LOCATION \
    iox::err::SourceLocation{ __FILE__, __LINE__, __func__}
// clang-format on
