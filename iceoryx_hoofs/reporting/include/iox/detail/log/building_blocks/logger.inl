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

#ifndef IOX_HOOFS_REPORTING_LOG_BUILDING_BLOCKS_LOGGER_INL
#define IOX_HOOFS_REPORTING_LOG_BUILDING_BLOCKS_LOGGER_INL

#include "iox/log/building_blocks/logger.hpp"

#include <cstdint>
#include <cstring>
#include <mutex>

namespace iox
{
namespace log
{
template <uint32_t N>
// NOLINTJUSTIFICATION See at declaration in header
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
inline bool equalStrings(const char* lhs, const char (&rhs)[N]) noexcept
{
    return strncmp(lhs, rhs, N) == 0;
}

namespace internal
{
template <typename BaseLogger>
inline Logger<BaseLogger>& Logger<BaseLogger>::get() noexcept
{
    /// @todo iox-#1755 use the PolymorphicHandler for the handling with the logger exchange once available

    // NOLINTJUSTIFICATION needed for the functionality
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    thread_local static Logger* logger = &Logger::activeLogger();
    if (!logger->m_isActive.load(std::memory_order_relaxed))
    {
        // no need to loop until m_isActive is true since this is an inherent race
        //   - the logger needs to be active for the whole lifetime of the application anyway
        //   - if the logger was changed again, the next call will update the logger
        //   - furthermore, it is not recommended to change the logger more than once
        logger = &Logger::activeLogger();
    }
    return *logger;
}

template <typename BaseLogger>
inline void Logger<BaseLogger>::init(const LogLevel logLevel) noexcept
{
    Logger::get().initLoggerInternal(logLevel);
}

template <typename BaseLogger>
inline void Logger<BaseLogger>::setActiveLogger(Logger<BaseLogger>& newLogger) noexcept
{
    Logger::activeLogger(&newLogger);
}

template <typename BaseLogger>
inline Logger<BaseLogger>& Logger<BaseLogger>::activeLogger(Logger<BaseLogger>* newLogger) noexcept
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    static Logger defaultLogger;

    // NOLINTJUSTIFICATION needed for the functionality
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
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
            /// @todo iox-#1755 call error handler after the error handler refactoring was merged
        }
        else
        {
            logger->m_isActive.store(false);
            logger = newLogger;
            logger->m_isActive.store(true);
        }
    }

    return *logger;
}

template <typename BaseLogger>
inline void Logger<BaseLogger>::initLoggerInternal(const LogLevel logLevel) noexcept
{
    if (!m_isFinalized.load(std::memory_order_relaxed))
    {
        BaseLogger::setLogLevel(logLevel);
        BaseLogger::initLogger(logLevel);
        iox_platform_set_log_backend(&platform_log_backend);
        m_isFinalized.store(true, std::memory_order_relaxed);
    }
    else
    {
        BaseLogger::createLogMessageHeader(__FILE__, __LINE__, __FUNCTION__, LogLevel::ERROR);
        BaseLogger::logString("Multiple initLogger calls");
        BaseLogger::flush();
        /// @todo iox-#1755 call error handler after the error handler refactoring was merged
    }
}

} // namespace internal
} // namespace log
} // namespace iox

#endif // IOX_HOOFS_REPORTING_LOG_BUILDING_BLOCKS_LOGGER_INL
