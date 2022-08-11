#pragma once

#include <stdint.h>

namespace eh
{
using error_code_t = uint32_t;
using module_id_t = uint32_t;

// 0 is reserved for now
constexpr module_id_t INVALID_MODULE = 0;

// primary template is the identity, i.e. code and error are the same
template <typename Code>
Code createError(Code code)
{
    return code;
}

} // namespace eh
