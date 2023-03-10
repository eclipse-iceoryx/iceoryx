#ifndef IOX_HOOFS_MODULETESTS_ERROR_REPORTING_MODULE_B_ERRORS_HPP
#define IOX_HOOFS_MODULETESTS_ERROR_REPORTING_MODULE_B_ERRORS_HPP

// ***
// *** TO BE IMPLEMENTED BY CLIENT - part of any module
// ***

#include "iceoryx_hoofs/error_reporting/errors.hpp"
#include "iceoryx_hoofs/error_reporting/types.hpp"

namespace module_b
{
namespace errors
{

using ErrorCode = iox::err::ErrorCode;
using ModuleId = iox::err::ModuleId;

constexpr ModuleId MODULE_ID{13};

enum class Code : ErrorCode::type
{
    Unknown = 24,
    OutOfMemory = 37,
    OutOfBounds = 12
};

class Error
{
  public:
    constexpr explicit Error(Code code = Code::Unknown)
        : m_code(static_cast<ErrorCode::type>(code))
    {
    }

    static constexpr ModuleId module()
    {
        return MODULE_ID;
    }

    ErrorCode code() const
    {
        return static_cast<ErrorCode>(m_code);
    }

  protected:
    ErrorCode m_code;
};

} // namespace errors
} // namespace module_b

namespace iox
{
namespace err
{

// This definition must exist in this namespace for overload resolution.
// Each module must use a unqiue error enum, e.g. by namespace.
inline module_b::errors::Error toError(module_b::errors::Code code)
{
    return module_b::errors::Error(code);
}

// Any error code of this enum has the same module id.
inline ModuleId toModule(module_b::errors::Code)
{
    return module_b::errors::MODULE_ID;
}

} // namespace err
} // namespace iox

#endif
