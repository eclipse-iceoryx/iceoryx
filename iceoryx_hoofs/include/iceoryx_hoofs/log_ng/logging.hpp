// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_LOG_LOGGING_HPP
#define IOX_HOOFS_LOG_LOGGING_HPP

#include "iceoryx_hoofs/log_ng/logstream.hpp"
namespace iox
{
namespace log
{
// TODO use environment variable to set file or function for custom filter
// ... environment variables for this should be read in initLogger
inline bool custom(const char* file, const char* function)
{
    static_cast<void>(file);
    static_cast<void>(function);
    return false;
}

// TODO how shall the filter work?
// - (ignoreLogLevel || level <= getLogLevel) && custom
// - ignoreLogLevel || (level <= getLogLevel && custom) <- this might be the best
// - ignoreLogLevel || level <= getLogLevel  || custom  <- or this if we pass the log level to the custom
// filter

#define IOX_LOG_INTERNAL(file, line, function, level)                                                                  \
    if ((level) <= iox::log::Logger::minimalLogLevel()                                                                 \
        && (iox::log::Logger::ignoreLogLevel() || (level) <= iox::log::Logger::getLogLevel()                           \
            || iox::log::custom(file, function)))                                                                      \
    iox::log::LogStream(file, line, function, level).self()

// use this
#define IOX_LOG(level) IOX_LOG_INTERNAL(__FILE__, __LINE__, __FUNCTION__, iox::log::LogLevel::level)

} // namespace log
} // namespace iox

#endif // IOX_HOOFS_LOG_LOGGING_HPP
