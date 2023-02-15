#pragma once

#include "iceoryx_hoofs/error_reporting/platform/default/error_handler.hpp"


#define ASSERT_NO_PANIC()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_FALSE(iox::err::TestErrorHandler::instance().hasPanicked());                                            \
    } while (false)

#define ASSERT_PANIC()                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::err::TestErrorHandler::instance().hasPanicked());                                             \
    } while (false)

#define ASSERT_ERROR(code)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        auto e = error_code_t(code);                                                                                   \
        ASSERT_TRUE(iox::err::TestErrorHandler::instance().hasError(e));                                               \
    } while (false)