#ifndef IOX_HOOFS_ERROR_REPORTING_ERROR_HPP
#define IOX_HOOFS_ERROR_REPORTING_ERROR_HPP

#include <utility>

#include "iceoryx_hoofs/error_reporting/types.hpp"

namespace iox
{
namespace err
{

// a complex hierarchy is not required yet, maybe move to a violation header
class Violation
{
  public:
    /// @todo: fix ctor types and class hierarchy (violation)
    explicit Violation(ErrorCode::type code)
        : m_code(code)
    {
    }

    Violation(ErrorCode::type code, ModuleId module)
        : m_code(code)
        , m_module(module)
    {
    }

    ErrorCode code() const
    {
        return m_code;
    }

    ModuleId module() const
    {
        return m_module;
    }

    // must return a pointer to data segment (no dynamic memory)
    const char* name() const
    {
        return NAME;
    }

  public:
    ErrorCode m_code{ErrorCode::DEBUG_ASSERT_VIOLATION};
    ModuleId m_module{ModuleId::UNKNOWN};

    static constexpr const char* NAME = "Violation";
};

// we expect an error to have
// 1. ErrorCode code()
// 2. module_id_t module()
// 3. const char* name()

// primary template is the identity
// this can be overriden by modules to create their own errors
template <typename ErrorLike>
auto toError(ErrorLike&& value)
{
    return std::forward<ErrorLike>(value);
}

template <class Error>
inline ErrorCode toCode(const Error& error)
{
    return error.code();
}

template <>
inline ErrorCode toCode<ErrorCode>(const ErrorCode& error)
{
    return error;
}

template <class Error>
inline ModuleId toModule(const Error& error)
{
    return error.module();
}

// generic comparison, has interface requirements on error types E1 and E2 without
// inheritance
template <typename E1, typename E2>
bool equals(const E1& a, const E2& b)
{
    return a.code() == b.code() && a.module() == b.module();
}

} // namespace err
} // namespace iox

#endif
