#pragma once

#pragma once

#include <stdint.h>

namespace eh
{
using error_code_t = uint32_t;
using module_id_t = uint32_t;

const char* noError = "NoError";

// TODO: do not use this
const char* error_name(error_code_t code)
{
    (void)code;
    return noError;
}

// we cannot get back to the original module enum error from this GenericError but we do not have to
// we can fall back to generic errors and compare them instead
// can be used in tests to check against against an expected error
class GenericError
{
    module_id_t m_module;
    error_code_t m_code;

  public:
    GenericError(module_id_t module, error_code_t code)
        : m_module(module)
        , m_code(code)
    {
    }

    template <typename Code>
    GenericError(Code code)
    {
        auto error = create_error(code);
        m_code = error.code();
        m_module = error.module();
    }

    error_code_t code()
    {
        return m_code;
    }

    module_id_t module()
    {
        return m_module;
    }

    bool operator==(const GenericError& other) const
    {
        return m_module == other.m_module && m_code == other.m_code;
    }

    bool operator!=(const GenericError& other) const
    {
        return !(*this == other);
    }
};
} // namespace eh
