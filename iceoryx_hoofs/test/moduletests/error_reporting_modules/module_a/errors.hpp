#pragma once

// ***
// *** TO BE IMPLEMENTED BY CLIENT - part of any module
// ***

#include "iceoryx_hoofs/error_reporting/error.hpp"

namespace module_a
{

namespace error
{

using error_code_t = iox::err::error_code_t;
using module_id_t = iox::err::module_id_t;

enum class ErrorCode : error_code_t
{
    Unknown = 42,
    OutOfMemory = 73,
    OutOfBounds = 66
};

// names exist in static segment without dynamic memory
static const char* errorNames[] = {"Unknown", "OutOfMemory", "OutOfBounds"};

// simple lightweight error class
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
        return (error_code_t)m_code;
    }

    // Contract: must return a pointer to data segment (no dynamic memory)
    const char* name() const
    {
        return errorNames[(error_code_t)m_code];
    }

    static constexpr module_id_t MODULE_ID = 73;

  protected:
    ErrorCode m_code;
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

    // dummy
    void* details()
    {
        return m_details;
    }

  private:
    // more infos if available
    void* m_details{nullptr};
};

} // namespace error

} // namespace module_a

namespace iox
{
namespace err
{

// transform codes to error
inline module_a::error::Error toError(module_a::error::ErrorCode code)
{
    return module_a::error::Error(code);
}

} // namespace err
} // namespace iox