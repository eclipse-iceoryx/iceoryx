#pragma once

#include "codes_B.hpp"
#include "iceoryx_hoofs/error_handling_2/api.hpp"

namespace module_b
{
void function()
{
    IOX_RAISE(FATAL, ErrorCode::OutOfMemory);
}
} // namespace module_B