#ifndef IOX_HOOFS_MODULETESTS_ERROR_REPORTING_MODULE_A_ERRORS_HPP
#define IOX_HOOFS_MODULETESTS_ERROR_REPORTING_MODULE_A_ERRORS_HPP

// ***
// *** TO BE IMPLEMENTED BY CLIENT - part of any module
// ***

#include "iceoryx_hoofs/error_reporting/errors.hpp"
#include "iceoryx_hoofs/error_reporting/types.hpp"

namespace module_a
{
namespace errors
{

using ErrorCode = iox::err::ErrorCode;
using ModuleId = iox::err::ModuleId;

constexpr ModuleId MODULE_ID{666};

enum class Code : ErrorCode::type
{
    Unknown = 42,
    OutOfMemory = 73,
    OutOfBounds = 21
};

class Error
{
  public:
    explicit Error(Code code = Code::Unknown)
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

    const char* name() const
    {
        return errorNames[code().value];
    }

  protected:
    ErrorCode m_code;

    static constexpr const char* errorNames[] = {"Unknown", "OutOfMemory", "OutOfBounds"};
};

} // namespace errors
} // namespace module_a

namespace iox
{
namespace err
{

// This definition must exist in this namespace for overload resolution.
// Each module must use a unqiue error enum, e.g. by namespace.
inline module_a::errors::Error toError(module_a::errors::Code code)
{
    return module_a::errors::Error(code);
}

// Any error code of this enum has the same module id.
inline ModuleId toModule(module_a::errors::Code)
{
    return module_a::errors::MODULE_ID;
}

} // namespace err
} // namespace iox

#endif
