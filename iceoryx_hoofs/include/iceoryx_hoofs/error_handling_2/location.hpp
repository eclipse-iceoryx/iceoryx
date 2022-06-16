#pragma once

namespace eh
{
struct SourceLocation
{
    const char* file{nullptr};
    unsigned line{0};
    const char* function{nullptr};
};

} // namespace eh

#define SOURCE_LOCATION                                                                                                \
    eh::SourceLocation                                                                                                 \
    {                                                                                                                  \
        __FILE__, __LINE__, __func__                                                                                   \
    }
