#ifndef IOX_HOOFS_ERROR_REPORTING_MODULES_HOOFS_ERRORS_HPP
#define IOX_HOOFS_ERROR_REPORTING_MODULES_HOOFS_ERRORS_HPP

#include "iceoryx_hoofs/error_reporting/error.hpp"

namespace iox
{
namespace hoofs_errors
{

using error_code_t = iox::err::error_code_t;
using module_id_t = iox::err::module_id_t;

enum class ErrorCode : error_code_t
{
    Unknown = 0
};

class Error
{
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
        return static_cast<error_code_t>(m_code);
    }

    const char* name() const
    {
        return errorNames[code()];
    }

    static constexpr module_id_t MODULE_ID = 1;

  protected:
    ErrorCode m_code;

    static constexpr const char* errorNames[] = {"Unknown"};
};

} // namespace hoofs_errors
} // namespace iox

namespace iox
{
namespace err
{

inline hoofs_errors::Error toError(hoofs_errors::ErrorCode code)
{
    return hoofs_errors::Error(code);
}

inline module_id_t toModule(hoofs_errors::ErrorCode)
{
    return hoofs_errors::Error::MODULE_ID;
}

} // namespace err
} // namespace iox

#endif
