#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"

namespace module_B
{
using error_code_t = eh::error_code_t;
using module_id_t = eh::module_id_t;

enum class ErrorCode : error_code_t
{
    Unknown = 0,
    OutOfMemory = 1,
    OutOfBounds = 2
};


static const char* errorNames[] = {"Unknown", "OutOfMemory", "OutOfBounds"};

struct Error
{
    ErrorCode code_;

    Error(ErrorCode code = ErrorCode::Unknown)
        : code_(code)
    {
    }

    static constexpr module_id_t module()
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

    static constexpr module_id_t MODULE_ID = 42;
};

} // namespace module_B

namespace eh
{
// module specific overload is required in eh
module_B::Error create_error(module_B::ErrorCode code)
{
    return module_B::Error(code);
}
} // namespace eh