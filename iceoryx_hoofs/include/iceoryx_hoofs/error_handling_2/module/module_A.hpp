#pragma once

#include "codes_A.hpp"
#include "iceoryx_hoofs/error_handling_2/api.hpp"

namespace module_A
{
void function()
{
    IOX_RAISE(ERROR, ErrorCode::OutOfBounds);
}
} // namespace module_A
