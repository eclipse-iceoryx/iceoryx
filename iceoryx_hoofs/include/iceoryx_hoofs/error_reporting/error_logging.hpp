#pragma once

#include "iceoryx_hoofs/error_reporting/location.hpp"
#include "iceoryx_hoofs/log/logging.hpp"

/// @todo location is not logged correctly (logger issue?)

#define IOX_LOG_ERROR(location)                                                                                        \
    IOX_LOG_INTERNAL(location.file, location.line, location.function, iox::log::LogLevel::ERROR)                       \
        << "line " << location.line << " "

#define IOX_LOG_FATAL_ERROR(location)                                                                                  \
    IOX_LOG_INTERNAL(location.file, location.line, location.function, iox::log::LogLevel::FATAL)                       \
        << "line " << location.line << " "

#define IOX_LOG_PANIC() IOX_LOG(FATAL)
