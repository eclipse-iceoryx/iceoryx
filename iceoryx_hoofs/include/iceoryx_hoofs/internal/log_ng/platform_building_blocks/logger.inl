// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGGER_INL
#define IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGGER_INL

#include "iceoryx_hoofs/log_ng/platform_building_blocks/logger.hpp"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>

namespace iox
{
namespace pbb
{
template <uint32_t N>
inline constexpr bool equalStrings(const char* lhs, const char (&rhs)[N]) noexcept
{
    return strncmp(lhs, rhs, N) == 0;
}


template <typename BaseLogger>
inline constexpr LogLevel Logger<BaseLogger>::minimalLogLevel() noexcept
{
    return Logger::MINIMAL_LOG_LEVEL;
}

template <typename BaseLogger>
inline constexpr bool Logger<BaseLogger>::ignoreLogLevel() noexcept
{
    return Logger::IGNORE_ACTIVE_LOG_LEVEL;
}


template <typename BaseLogger>
inline Logger<BaseLogger>& Logger<BaseLogger>::get() noexcept
{
    // TODO is static required with thread_local
    thread_local static Logger* logger = Logger::activeLogger();
    if (!logger->m_isActive.load(std::memory_order_relaxed))
    {
        // no need to loop until m_isActive is true since this is an inherent race
        //   - the logger needs to be active for the whole lifetime of the application anyway
        //   - if the logger was changed again, the next call will update the logger
        //   - furthermore, it is not recommended to change the logger more than once
        logger = Logger::activeLogger();
    }
    return *logger;
}

template <typename BaseLogger>
inline void Logger<BaseLogger>::init(const LogLevel logLevel) noexcept
{
    Logger::get().initLoggerInternal(logLevel);
}

template <typename BaseLogger>
inline void Logger<BaseLogger>::setActiveLogger(Logger<BaseLogger>* newLogger) noexcept
{
    Logger::activeLogger(newLogger);
}

template <typename BaseLogger>
inline Logger<BaseLogger>* Logger<BaseLogger>::activeLogger(Logger<BaseLogger>* newLogger) noexcept
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    static Logger defaultLogger;
    static Logger* logger{&defaultLogger};

    if (newLogger)
    {
        if (logger->m_isFinalized.load(std::memory_order_relaxed))
        {
            logger->createLogMessageHeader(__FILE__, __LINE__, __FUNCTION__, LogLevel::ERROR);
            logger->logString("Trying to replace logger after already initialized!");
            logger->flush();
            newLogger->createLogMessageHeader(__FILE__, __LINE__, __FUNCTION__, LogLevel::ERROR);
            logger->logString("Trying to replace logger after already initialized!");
            newLogger->flush();
            // TODO call error handler
        }
        else
        {
            logger->m_isActive.store(false);
            logger = newLogger;
        }
    }

    return logger;
}

template <typename BaseLogger>
inline void Logger<BaseLogger>::initLoggerInternal(const LogLevel logLevel) noexcept
{
    if (!m_isFinalized.load(std::memory_order_relaxed))
    {
        BaseLogger::setLogLevel(logLevel);
        BaseLogger::initLogger(logLevel);
        m_isFinalized.store(true, std::memory_order_relaxed);
    }
    else
    {
        BaseLogger::setupNewLogMessage(__FILE__, __LINE__, __FUNCTION__, LogLevel::ERROR);
        BaseLogger::logString("Multiple initLogger calls");
        BaseLogger::flush();
        // TODO call error handler
    }
}

} // namespace pbb
} // namespace iox

#endif // IOX_HOOFS_PLATFORM_BUILDING_BLOCKS_LOGGER_INL
