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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_HOOFS_MOCKS_LOGGER_MOCK_HPP
#define IOX_HOOFS_MOCKS_LOGGER_MOCK_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/log/logger.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/log/logstream.hpp"

#include <mutex>
#include <vector>

namespace iox
{
namespace testing
{
#define IOX_LOGSTREAM_MOCK(logger)                                                                                     \
    iox::log::LogStream((logger), "file", 42, "function", iox::log::LogLevel::TRACE).self()

class Logger_Mock : public platform::TestingLoggerBase
{
    using Base = platform::TestingLoggerBase;

  public:
    Logger_Mock() noexcept = default;

    void createLogMessageHeader(const char* file,
                                const int line,
                                const char* function,
                                log::LogLevel logLevel) noexcept override
    {
        Base::assumeFlushed();

        LogEntry logEntry;
        logEntry.file = file;
        logEntry.line = line;
        logEntry.function = function;
        logEntry.logLevel = logLevel;

        m_logs.emplace_back(std::move(logEntry));
    }

    void flush() noexcept override
    {
        const auto logBuffer = Base::getLogBuffer();
        m_logs.back().message = logBuffer.buffer;
        Base::assumeFlushed();
    }

    struct LogEntry
    {
        std::string file;
        int line{0};
        std::string function;
        log::LogLevel logLevel{iox::log::LogLevel::OFF};
        std::string message;
    };

    std::vector<LogEntry> m_logs;
};

} // namespace testing
} // namespace iox

#endif // IOX_HOOFS_MOCKS_LOGGER_MOCK_HPP
