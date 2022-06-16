#pragma once

#pragma once

#include <stdint.h>

using error_code_t = uint32_t;

enum class ModuleError : error_code_t
{
    None = 0,
    OutOfMemory = 1,
    OutOfBounds = 2
};

static const char* errorCodes[] = {"None", "OutOfMemory", "OutOfBounds"};

error_code_t error_code_to_num(ModuleError code)
{
    return (error_code_t) code;
}

// efficient array lookup requires consecuive numbers but this can be
// generated code
const char* error_code_to_name(error_code_t code)
{
    return errorCodes[code];
}

const char* error_code_to_name(ModuleError code)
{
    return errorCodes[(error_code_t)code];
}
