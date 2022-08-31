#pragma once

#include <stdint.h>

#include "error_code.hpp"
#include "error_level.hpp"

namespace eh
{
// generic error that occurs at runtime (loses information compared to a specific error type)
class RuntimeError
{
    module_id_t m_module{INVALID_MODULE};
    error_code_t m_code{0};
    error_level_t m_level{FATAL_LEVEL};

  public:
    RuntimeError()
    {
    }

    RuntimeError(module_id_t module, error_code_t code, error_level_t level /* = FATAL_LEVEL*/)
        : m_module(module)
        , m_code(code)
        , m_level(level)
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
        return m_module == other.m_module && m_code == other.m_code && m_level == other.m_level;
    }

    bool operator!=(const RuntimeError& other) const
    {
        return !(*this == other);
    }

    template <typename Error, typename Level>
    static RuntimeError from(const Error& error, const Level&)
    {
        return RuntimeError(error.module(), error.code(), Level::value);
    }
};
} // namespace eh
