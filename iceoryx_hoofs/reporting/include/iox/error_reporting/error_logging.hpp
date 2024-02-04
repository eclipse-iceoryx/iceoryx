// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_HOOFS_REPORTING_ERROR_REPORTING_ERROR_LOGGING_HPP
#define IOX_HOOFS_REPORTING_ERROR_REPORTING_ERROR_LOGGING_HPP

#include "iox/error_reporting/source_location.hpp"
#include "iox/logging.hpp"

// with a log stream interface this could be done with functions, not macros
// NOLINTBEGIN(cppcoreguidelines-macro-usage, bugprone-macro-parentheses) macros are required without logstream interface

/// @brief Log the location of an error.
/// @param location the location of the error
/// @param msg_stream is the log message stream; multiple items can be logged by using the '<<' operator
#define IOX_ERROR_INTERNAL_LOG(location, msg_stream)                                                                   \
    IOX_LOG_INTERNAL(location.file,                                                                                    \
                     location.line,                                                                                    \
                     location.function,                                                                                \
                     iox::log::LogLevel::ERROR,                                                                        \
                     location.file << ":" << location.line << " " << msg_stream)

/// @brief Log the location of a fatal error.
/// @param location the location of the error
/// @param msg_stream is the log message stream; multiple items can be logged by using the '<<' operator
#define IOX_ERROR_INTERNAL_LOG_FATAL(location, msg_stream)                                                             \
    IOX_LOG_INTERNAL(location.file,                                                                                    \
                     location.line,                                                                                    \
                     location.function,                                                                                \
                     iox::log::LogLevel::FATAL,                                                                        \
                     location.file << ":" << location.line << " " << msg_stream)

/// @brief Log a panic invocation.
/// @param location the location of the panic invocation.
/// @param msg_stream is the log message stream; multiple items can be logged by using the '<<' operator
#define IOX_ERROR_INTERNAL_LOG_PANIC(location, msg_stream) IOX_ERROR_INTERNAL_LOG_FATAL(location, msg_stream)

// NOLINTEND(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)

#endif // IOX_HOOFS_REPORTING_ERROR_REPORTING_ERROR_LOGGING_HPP
