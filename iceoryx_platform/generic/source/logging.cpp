// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_platform/logging.hpp"
#include "iceoryx_platform/atomic.hpp"

#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

// NOLINTJUSTIFICATION only used in this file; should be fine
// NOLINTNEXTLINE(readability-function-size)
void iox_platform_detail_default_log_backend(
    const char* file, int line, const char* function, IceoryxPlatformLogLevel log_level, const char* msg)
{
    if (log_level == IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_OFF)
    {
        return;
    }

    std::stringstream stream;

    switch (log_level)
    {
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_FATAL:
        stream << "[Fatal]";
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_ERROR:
        stream << "[Error]";
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_WARN:
        stream << "[Warn ]";
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_INFO:
        stream << "[Info ]";
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_DEBUG:
        stream << "[Debug]";
        break;
    case IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_TRACE:
        stream << "[Trace]";
        break;
    default:
        stream << "[UNDEF]";
        break;
    }

    stream << " " << file << ":" << line << " { " << function << " } " << msg;

    // NOLINTJUSTIFICATION We want to flush the line with each log output; if performance matters a custom log backend can be used, e.g. the logger in hoofs
    // NOLINTNEXTLINE(performance-avoid-endl)
    std::cout << stream.str() << std::endl;
}

namespace
{
enum class LoggerExchangeState : uint8_t
{
    DEFAULT,
    PENDING,
    CUSTOM,
};

struct IceoryxPlatformLogger
{
    iox::concurrent::Atomic<IceoryxPlatformLogBackend> log_backend{&iox_platform_detail_default_log_backend};
    iox::concurrent::Atomic<LoggerExchangeState> logger_exchange_state{LoggerExchangeState::DEFAULT};
};

IceoryxPlatformLogger& active_logger(IceoryxPlatformLogBackend new_log_backend)
{
    static IceoryxPlatformLogger logger;

    if (new_log_backend != nullptr)
    {
        auto exchange_state = LoggerExchangeState::DEFAULT;
        auto successful =
            logger.logger_exchange_state.compare_exchange_strong(exchange_state, LoggerExchangeState::PENDING);
        if (successful)
        {
            logger.log_backend.store(new_log_backend);
            logger.logger_exchange_state.store(LoggerExchangeState::CUSTOM);
        }
        else
        {
            constexpr uint64_t YIELDS_BEFORE_SLEEP{10000};
            uint64_t yield_counter = 0;
            while (logger.logger_exchange_state.load() != LoggerExchangeState::CUSTOM)
            {
                if (yield_counter < YIELDS_BEFORE_SLEEP)
                {
                    std::this_thread::yield();
                    ++yield_counter;
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }

            logger.log_backend.load(std::memory_order_relaxed)(__FILE__,
                                                               __LINE__,
                                                               static_cast<const char*>(__FUNCTION__),
                                                               IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_ERROR,
                                                               "Trying to replace logger after already initialized");

            new_log_backend(__FILE__,
                            __LINE__,
                            static_cast<const char*>(__FUNCTION__),
                            IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_ERROR,
                            "Trying to replace logger after already initialized");
        }
    }

    return logger;
}
} // namespace

// NOLINTJUSTIFICATION only used from a macro which hides most of the parameter; should be fine
// NOLINTNEXTLINE(readability-function-size)
void iox_platform_detail_log(
    const char* file, int line, const char* function, IceoryxPlatformLogLevel log_level, const char* msg)
{
    // NOLINTJUSTIFICATION needed for the functionality
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    static IceoryxPlatformLogger* logger = &active_logger(nullptr);
    thread_local static LoggerExchangeState current_logger_exchange_state = logger->logger_exchange_state.load();
    // NOTE if the logger got updated in between, the current_logger_exchange_state has an old value and we will enter
    // the if statement
    thread_local static IceoryxPlatformLogBackend log_backend = logger->log_backend.load();

    if (current_logger_exchange_state == LoggerExchangeState::PENDING
        || current_logger_exchange_state != logger->logger_exchange_state.load(std::memory_order_relaxed))
    {
        // the logger can be changed only once and if we enter this if statement the 'active_logger' function will only
        // return once 'logger_exchange_state' equals 'CUSTOM'
        logger = &active_logger(nullptr);
        log_backend = logger->log_backend.load();
        current_logger_exchange_state = logger->logger_exchange_state.load();
    }

    log_backend(file, line, function, log_level, msg);
}

void iox_platform_set_log_backend(IceoryxPlatformLogBackend log_backend)
{
    if (log_backend == nullptr)
    {
        IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_ERROR, "'log_backend' must not be a nullptr!");
        return;
    }

    active_logger(log_backend);
}
