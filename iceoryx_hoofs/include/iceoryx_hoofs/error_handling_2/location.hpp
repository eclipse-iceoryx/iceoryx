#pragma once

#include <stdint.h>

namespace eh
{
struct SourceLocation
{
    const char* file{nullptr};
    uint32_t line{0};
    const char* function{nullptr};
};

} // namespace eh

#define SOURCE_LOCATION                                                                                                \
    eh::SourceLocation                                                                                                 \
    {                                                                                                                  \
        __FILE__, __LINE__, __func__                                                                                   \
    }
