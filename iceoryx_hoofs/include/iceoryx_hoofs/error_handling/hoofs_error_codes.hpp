#pragma once

// to check errorHandler replacement strategy in hoofs


#include "iceoryx_hoofs/error_handling_2/error_code.hpp"

namespace iox
{
// clang-format off

#define GENERATE_VALUE(name) name,
#define GENERATE_STRING(name) #name,

// apply macro e to all the listed inputs
#define GENERATE_HOOFS_ERRORS(e) \
    e(ExpectsEnsuresFailed) \
    e(SomeOtherError)

// clang-format on

enum class HoofsErrorCode : eh::error_code_t
{
    Unknown = 0,
    GENERATE_HOOFS_ERRORS(GENERATE_VALUE)

};

const char* HOOFS_MODULE_ERROR_NAMES[] = {"Unknown", GENERATE_HOOFS_ERRORS(GENERATE_STRING)};

struct Hoofs_Error
{
  private:
    HoofsErrorCode m_code;

    static constexpr eh::module_id_t MODULE_ID = 1;

  public:
    Hoofs_Error(HoofsErrorCode code = HoofsErrorCode::Unknown)
        : m_code(code)
    {
    }

    static constexpr eh::module_id_t module()
    {
        return MODULE_ID;
    }

    eh::error_code_t code() const
    {
        return (eh::error_code_t)m_code;
    }

    // Contract: must return a pointer to data segment (no dynamic memory)
    const char* name() const
    {
        return HOOFS_MODULE_ERROR_NAMES[(eh::error_code_t)m_code];
    }
};

} // namespace iox

namespace eh
{
// module specific overload is required in eh
iox::Hoofs_Error createError(iox::HoofsErrorCode code)
{
    return iox::Hoofs_Error(code);
}
} // namespace eh
