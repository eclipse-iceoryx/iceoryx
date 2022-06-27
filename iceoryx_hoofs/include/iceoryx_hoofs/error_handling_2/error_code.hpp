#pragma once

#include <stdint.h>

namespace eh
{
using error_code_t = uint32_t;
using module_id_t = uint32_t;

// 0 is reserved for now
constexpr module_id_t INVALID_MODULE = 0;

} // namespace eh
