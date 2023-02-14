#pragma once

#include <cstdint>

namespace iox
{
namespace err
{
using error_code_t = uint32_t;
using module_id_t = uint32_t;

// no need to reserve, we can distinguish violations from error
constexpr error_code_t DEBUG_ASSERT_VIOLATION_CODE = 0;
constexpr error_code_t PRECONDITION_VIOLATION_CODE = 1;


// a complex hierarchy is not required yet, maybe move to a violation header
class Violation
{
  public:
    explicit Violation(error_code_t code)
        : m_code(code)
    {
    }

    static constexpr module_id_t module()
    {
        return 0;
    }

    error_code_t code() const
    {
        return m_code;
    }

    // Contract: must return a pointer to data segment (no dynamic memory)
    const char* name() const
    {
        return NAME;
    }

  public:
    error_code_t m_code{DEBUG_ASSERT_VIOLATION_CODE};

    static constexpr const char* NAME = "Violation";
};

// we expect an error to have
// 1. error_code_t code()
// 2. module_id_t module()
// 3. const char* name()

// 0 is reserved
constexpr module_id_t INVALID_MODULE = 0;

// primary template is the identity
template <typename Error>
auto toError(Error&& error)
{
    return error;
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
