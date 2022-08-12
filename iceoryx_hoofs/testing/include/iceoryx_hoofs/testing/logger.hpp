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

#ifndef IOX_HOOFS_TESTING_LOGGER_HPP
#define IOX_HOOFS_TESTING_LOGGER_HPP

#include "iceoryx_hoofs/internal/concurrent/smart_lock.hpp"
#include "iceoryx_hoofs/log/logger.hpp"

#include "test.hpp"

#include <mutex>

namespace iox
{
namespace testing
{
class Logger : public platform::TestingLoggerBase
{
    using Base = platform::TestingLoggerBase;

  public:
    static void init();

    void clearLogBuffer();

    void printLogBuffer();

    static uint64_t getNumberOfLogMessages();

    static std::vector<std::string> getLogMessages();

  private:
    Logger() noexcept = default;

    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;

    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    void flush() noexcept override;

    struct LoggerData
    {
        std::vector<std::string> buffer;
        bool allowLog{true};
    };

    concurrent::smart_lock<LoggerData> m_loggerData;
};

class LogPrinter : public ::testing::EmptyTestEventListener
{
    void OnTestStart(const ::testing::TestInfo&) override;
    void OnTestPartResult(const ::testing::TestPartResult& result) override;
};

} // namespace testing
} // namespace iox

#endif // IOX_HOOFS_TESTING_LOGGER_HPP
