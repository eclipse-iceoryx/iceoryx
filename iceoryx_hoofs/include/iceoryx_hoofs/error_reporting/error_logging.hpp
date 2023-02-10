#pragma once

// TODO: move to platform code, but we use it in the proxy
// TODO: remove proxy

#include "iceoryx_hoofs/error_reporting/error.hpp"
#include "iceoryx_hoofs/error_reporting/location.hpp"

#include "iceoryx_hoofs/log/logger.hpp"
#include "iceoryx_hoofs/log/logging.hpp"

namespace iox
{
namespace err
{
// TODO: we may need smething log related here, check headers that are needed

} // namespace err
} // namespace iox

// TODO: correct location logging


#define IOX_LOG_ERROR(location)                                                                                        \
    IOX_LOG_INTERNAL(location.file, location.line, location.function, iox::log::LogLevel::ERROR)                       \
        << "line " << location.line << " "

#define IOX_LOG_FATAL_ERROR(location)                                                                                  \
    IOX_LOG_INTERNAL(location.file, location.line, location.function, iox::log::LogLevel::FATAL)                       \
        << "line " << location.line << " "

#define IOX_LOG_PANIC() IOX_LOG(FATAL)
