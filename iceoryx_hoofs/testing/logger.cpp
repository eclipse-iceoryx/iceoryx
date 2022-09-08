// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/logger.hpp"

namespace iox
{
namespace testing
{
void Logger::init()
{
    static Logger logger;
    log::Logger::setActiveLogger(&logger);
    log::Logger::init(log::logLevelFromEnvOr(log::LogLevel::TRACE));
    // disable logger output only after initializing the logger to get error messages from initialization
    // JUSTIFICATION getenv is required for the functionality of the testing logger and will be called only once in main
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    if (const auto* allowLogString = std::getenv("IOX_TESTING_ALLOW_LOG"))
    {
        logger.m_loggerData->allowLog = pbb::equalStrings(allowLogString, "on");
    }
    else
    {
        logger.m_loggerData->allowLog = false;
    }

    auto& listeners = ::testing::UnitTest::GetInstance()->listeners();
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory) required by the callee
    listeners.Append(new LogPrinter);
}

void Logger::clearLogBuffer()
{
    m_loggerData->buffer.clear();
}

void Logger::printLogBuffer()
{
    auto loggerData = m_loggerData.getScopeGuard();
    if (loggerData->buffer.empty())
    {
        return;
    }
    puts("#### Log start ####");
    for (const auto& log : loggerData->buffer)
    {
        puts(log.c_str());
    }
    puts("#### Log end ####");
}

uint64_t Logger::getNumberOfLogMessages()
{
    auto& logger = dynamic_cast<Logger&>(log::Logger::get());
    return logger.m_loggerData->buffer.size();
}

std::vector<std::string> Logger::getLogMessages()
{
    auto& logger = dynamic_cast<Logger&>(log::Logger::get());
    return logger.m_loggerData->buffer;
}

void Logger::flush() noexcept
{
    auto loggerData = m_loggerData.getScopeGuard();
    const auto logBuffer = Base::getLogBuffer();
    loggerData->buffer.emplace_back(logBuffer.buffer, logBuffer.writeIndex);

    if (loggerData->allowLog)
    {
        Base::flush();
    }

    Base::assumeFlushed();
}

void LogPrinter::OnTestStart(const ::testing::TestInfo&)
{
    dynamic_cast<Logger&>(log::Logger::get()).clearLogBuffer();

    /// @todo iox-#1345 register signal handler for sigterm to flush to logger;
    /// there might be tests to register a handler itself and when this is
    /// done at each start of the test only the tests who use their
    /// own signal handler are affected and don't get an log output on termination
}

void LogPrinter::OnTestPartResult(const ::testing::TestPartResult& result)
{
    if (result.failed())
    {
        dynamic_cast<Logger&>(log::Logger::get()).printLogBuffer();
    }

    /// @todo iox-#1345 de-register the signal handler from 'OnTestStart'
}

} // namespace testing
} // namespace iox
