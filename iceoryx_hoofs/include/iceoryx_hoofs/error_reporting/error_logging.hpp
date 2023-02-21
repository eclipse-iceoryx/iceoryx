#ifndef IOX_HOOFS_ERROR_REPORTING_ERROR_LOGGING_HPP
#define IOX_HOOFS_ERROR_REPORTING_ERROR_LOGGING_HPP

#include "iceoryx_hoofs/error_reporting/location.hpp"
#include "iceoryx_hoofs/log/logging.hpp"

/// @todo with a log stream interface this could be doen with functions, not macros

/// @brief Log the location of an error.
/// @param location the location of the error
#define IOX_LOG_ERROR(location)                                                                                        \
    IOX_LOG_INTERNAL(location.file, location.line, location.function, iox::log::LogLevel::ERROR)                       \
        << location.file << " line " << location.line

/// @brief Log the location of a fatal error.
/// @param location the location of the error
#define IOX_LOG_FATAL_ERROR(location)                                                                                  \
    IOX_LOG_INTERNAL(location.file, location.line, location.function, iox::log::LogLevel::FATAL)                       \
        << location.file << " line " << location.line << ": "

/// @brief Log a panic invocation.
/// @param location the location of the panic invocation.
#define IOX_LOG_PANIC(location) IOX_LOG_FATAL_ERROR(location)

#endif
