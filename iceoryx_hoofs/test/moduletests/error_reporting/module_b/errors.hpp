#pragma once

// ***
// *** TO BE IMPLEMENTED BY CLIENT - part of any module
// ***

#include "iceoryx_hoofs/error_reporting/error.hpp"

namespace module_b
{

namespace errors
{

using error_code_t = iox::err::error_code_t;
using module_id_t = iox::err::module_id_t;

enum class ErrorCode : error_code_t
{
    Unknown = 24,
    OutOfMemory = 37,
    OutOfBounds = 12
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

    // Contract: must return a pointer to data segment (no dynamic memory)
    const char* name() const
    {
        return errorNames[code()];
    }

    static constexpr module_id_t MODULE_ID = 2;

  protected:
    ErrorCode m_code;

    static constexpr const char* errorNames[] = {"Unknown", "OutOfMemory", "OutOfBounds"};
};

// could be wrapped by a result/optional monadic type
// could also be implemented without inheritence
class OutOfBoundsError : public Error
{
  public:
    OutOfBoundsError()
        : Error(ErrorCode::OutOfBounds)
    {
    }

    void* details()
    {
        return m_details;
    }

  private:
    // more infos if available
    void* m_details{nullptr};
};

} // namespace errors

} // namespace module_b

namespace iox
{
namespace err
{

inline module_b::errors::Error toError(module_b::errors::ErrorCode code)
{
    return module_b::errors::Error(code);
}

} // namespace err
} // namespace iox