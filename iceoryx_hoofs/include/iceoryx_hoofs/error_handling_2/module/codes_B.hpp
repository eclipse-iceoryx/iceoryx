#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"

namespace module_b
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

class Error
{
    ErrorCode m_code;

  public:
    explicit Error(ErrorCode code = ErrorCode::Unknown)
        : m_code(code)
    {
    }

    static constexpr module_id_t module()
    {
        return MODULE_ID;
    }

    error_code_t code() const
    {
        return (error_code_t)m_code;
    }

    const char* name() const
    {
        return errorNames[(error_code_t)m_code];
    }

    static constexpr module_id_t MODULE_ID = 42;
};

} // namespace module_B

namespace eh
{
// module specific overload is required in eh
module_b::Error createError(module_b::ErrorCode code)
{
    return module_b::Error(code);
}
} // namespace eh