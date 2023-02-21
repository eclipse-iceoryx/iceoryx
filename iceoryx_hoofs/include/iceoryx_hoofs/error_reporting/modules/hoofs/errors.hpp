#ifndef IOX_HOOFS_ERROR_REPORTING_MODULES_HOOFS_ERRORS_HPP
#define IOX_HOOFS_ERROR_REPORTING_MODULES_HOOFS_ERRORS_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/types.hpp"

namespace iox
{
namespace hoofs_errors
{

using ModuleId = iox::err::ModuleId;

enum class Code : iox::err::ErrorCode::type
{
    Unknown = 0
};

class Error
{
  public:
    explicit Error(Code code = Code::Unknown)
        : m_code(static_cast<iox::err::ErrorCode::type>(code))
    {
    }

    static constexpr ModuleId module()
    {
        return MODULE_ID;
    }

    iox::err::ErrorCode code() const
    {
        return m_code;
    }

    const char* name() const
    {
        return errorNames[m_code.value];
    }

    static constexpr ModuleId MODULE_ID{1};

  protected:
    iox::err::ErrorCode m_code;

    static constexpr const char* errorNames[] = {"Unknown"};
};

} // namespace hoofs_errors
} // namespace iox

namespace iox
{
namespace err
{

inline hoofs_errors::Error toError(hoofs_errors::Code code)
{
    return hoofs_errors::Error(code);
}

inline ModuleId toModule(hoofs_errors::Code)
{
    return hoofs_errors::Error::MODULE_ID;
}

} // namespace err
} // namespace iox

#endif
