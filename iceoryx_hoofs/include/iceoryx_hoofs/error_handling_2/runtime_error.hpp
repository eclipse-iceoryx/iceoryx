#pragma once

#include <stdint.h>

#include "error_code.hpp"

namespace eh
{
// generic error that occurs at runtime (loses information from compared to a specific error type)
class RuntimeError
{
    module_id_t m_module{INVALID_MODULE};
    error_code_t m_code{0};

  public:
    RuntimeError()
    {
    }

    RuntimeError(module_id_t module, error_code_t code)
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

    bool operator==(const RuntimeError& other) const
    {
        return m_module == other.m_module && m_code == other.m_code;
    }

    bool operator!=(const RuntimeError& other) const
    {
        return !(*this == other);
    }

    template <typename Error>
    static RuntimeError from_error(const Error& e)
    {
        return RuntimeError(e.module(), e.code());
    }
};
} // namespace eh
