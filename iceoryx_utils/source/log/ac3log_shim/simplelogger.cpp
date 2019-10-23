// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "ac3log/simplelogger.hpp"

#include "iceoryx_utils/log/logmanager.hpp"

uint8_t debuglevel = 99;

namespace
{
iox::log::LogLevel mapIoxLogToAc3LogLevel()
{
    iox::log::LogLevel loglevel;

    switch (debuglevel)
    {
    case L_ERR:
        loglevel = iox::log::LogLevel::kError;
        break;
    case L_MSG:
        loglevel = iox::log::LogLevel::kError;
        break;
    case L_WARN:
        loglevel = iox::log::LogLevel::kWarn;
        break;
    case L_INFO:
        loglevel = iox::log::LogLevel::kInfo;
        break;
    case L_DEBUG:
        loglevel = iox::log::LogLevel::kDebug;
        break;
    default:
        loglevel = iox::log::LogLevel::kVerbose;
        break;
    }

    return loglevel;
}

static iox::log::Logger& logger()
{
    static auto& logger =
        iox::log::CreateLogger("ac3log", "Log context of the ac3log transition library!", mapIoxLogToAc3LogLevel());
    return logger;
}

} // namespace

void log_init()
{
    // if log_init is called, the ac3log API is used, therefore the iox log default log level has to be set
    iox::log::LogManager::GetLogManager().SetDefaultLogLevel(mapIoxLogToAc3LogLevel());

    logger().SetLogLevel(mapIoxLogToAc3LogLevel());
}

void iox_log(const uint8_t debuglevel, char* msg)
{
    /// @todo this is a workaround because the global debuglevel variable from ac3log might be set or changed without
    /// calling log_init, therefore we have to set it here in order to keep the the old behavior;
    /// a better solution is needed if we keep the ac3log interface!
    logger().SetLogLevel(mapIoxLogToAc3LogLevel());

    // with the new logger an new line character is automacially added for each log message, therefore remove the
    // explicit new lines from ac3log calls
    int i = 0;
    while (msg[i] != 0)
    {
        i++;
    }
    if (i != 0 && msg[i - 1] == '\n')
    {
        msg[i - 1] = 0;
    }

    switch (debuglevel)
    {
    case L_ERR:
        logger().LogError() << msg;
        break;
    case L_MSG:
        logger().LogError() << msg;
        break;
    case L_WARN:
        logger().LogWarn() << msg;
        break;
    case L_INFO:
        logger().LogInfo() << msg;
        break;
    case L_DEBUG:
        logger().LogDebug() << msg;
        break;
    default:
        logger().LogVerbose() << msg;
        break;
    }
}
