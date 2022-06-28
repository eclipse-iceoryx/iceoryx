#pragma once

#include <stdint.h>

#include "error_code.hpp"

namespace eh
{
// we cannot get back to the original module enum error from this GenericError but we do not have to
// we can fall back to generic errors and compare them instead
// can be used in tests to check against against an expected error

// currently only used for testing
class GenericError
{
    module_id_t m_module{INVALID_MODULE};
    error_code_t m_code{0};

  public:
    GenericError()
    {
    }

    GenericError(module_id_t module, error_code_t code)
        : m_module(module)
        , m_code(code)
    {
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

    // cannot use ctor templates (both would have one argument of generic type)
    template <typename Code>
    static GenericError from_code(Code code)
    {
        auto e = create_error(code);
        return GenericError(e.module(), e.code());
    }

    template <typename Error>
    static GenericError from_error(const Error& e)
    {
        return GenericError(e.module(), e.code());
    }
};
} // namespace eh