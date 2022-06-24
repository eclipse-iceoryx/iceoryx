#pragma once

#include <stdint.h>

using error_code_t = uint32_t;
using module_id_t = uint32_t;

namespace module_B
{
enum class ErrorCode : error_code_t
{
    Unknown = 0, // more like default/unspecific
    OutOfMemory = 1,
    OutOfBounds = 2
};

// each module must have exactly one ID
static constexpr module_id_t MODULE_ID = 42;

static const char* errorNames[] = {"Unknown", "OutOfMemory", "OutOfBounds"};

struct Error
{
    ErrorCode code_;

    Error(ErrorCode code = ErrorCode::Unknown)
        : code_(code)
    {
    }

    static module_id_t module()
    {
        return MODULE_ID;
    }

    error_code_t code()
    {
        return (error_code_t)code_;
    }

    const char* name()
    {
        return errorNames[(error_code_t)code_];
    }
};

} // namespace module_B

namespace eh
{
using B_Error = module_B::Error;
using B_Code = module_B::ErrorCode;

// module specific overload
B_Error create_error(B_Code code)
{
    return B_Error(code);
}
} // namespace eh