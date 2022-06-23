#pragma once

#include <stdint.h>

using error_code_t = uint32_t;
using module_id_t = uint32_t;

namespace module_A
{
enum class ErrorCode : error_code_t
{
    Unknown = 0, // more like default/unspecific
    OutOfMemory = 1,
    OutOfBounds = 2
};

static constexpr module_id_t MODULE_ID = 73;

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

} // namespace module_A


namespace eh
{
using A_Error = module_A::Error;
using A_Code = module_A::ErrorCode;

// module specific overload must exist but can be identity if needed
A_Error create_error(A_Code code)
{
    return A_Error(code);
}
} // namespace eh