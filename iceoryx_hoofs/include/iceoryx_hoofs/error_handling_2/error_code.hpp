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
} // namespace eh
