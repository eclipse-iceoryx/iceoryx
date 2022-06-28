#pragma once

// ***
// *** TO BE IMPLEMENTED BY CLIENT - part of any module
// ***

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"

namespace module_A
{
using error_code_t = eh::error_code_t;
using module_id_t = eh::module_id_t;

enum class ErrorCode : error_code_t
{
    Unknown = 0,
    OutOfMemory = 1,
    OutOfBounds = 2
};

// can be part of the struct
static constexpr eh::module_id_t MODULE_ID = 73;

// could be a static in Error
static const char* errorNames[] = {"Unknown", "OutOfMemory", "OutOfBounds"};

struct Error
{
  private:
    ErrorCode code_;

  public:
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

    // Contract: must return a pointer to data segment (no dynamic memory)
    const char* name() const
    {
        return errorNames[(error_code_t)code_];
    }
};

} // namespace module_A

namespace eh
{
using A_Error = module_A::Error;
using A_Code = module_A::ErrorCode;

// module specific overload is required
A_Error create_error(A_Code code)
{
    return A_Error(code);
}
} // namespace eh