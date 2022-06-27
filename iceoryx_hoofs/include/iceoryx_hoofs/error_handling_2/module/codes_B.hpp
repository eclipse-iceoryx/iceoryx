#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"

namespace module_B
{
using error_code_t = eh::error_code_t;
using module_id_t = eh::module_id_t;

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

    error_code_t code() const
    {
        return (error_code_t)code_;
    }

    const char* name() const
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