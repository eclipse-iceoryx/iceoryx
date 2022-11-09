#pragma once

// ***
// *** TO BE IMPLEMENTED BY CLIENT - part of any module
// ***

#include "iceoryx_hoofs/error_handling_3/error.hpp"

namespace module_b
{

namespace error
{

using error_code_t = eh3::error_code_t;
using module_id_t = eh3::module_id_t;

enum class ErrorCode : error_code_t
{
    Unknown = 0,
    OutOfMemory = 1,
    OutOfBounds = 2
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

    static constexpr eh3::module_id_t MODULE_ID = 42;

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

} // namespace module_b

namespace eh3
{

// transform codes to error
module_b::error::Error toError(module_b::error::ErrorCode code)
{
    return module_b::error::Error(code);
}

} // namespace eh3