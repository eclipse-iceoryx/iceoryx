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

#if 0

#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/log/logstream.hpp"
#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::roudi;

class MonitoringModeLogStreamTest : public Test
{
  public:
    void TearDown()
    {
        m_loggerMock.m_logs.clear();
    }

    Logger_Mock m_loggerMock;
};

TEST_F(MonitoringModeLogStreamTest, MonitoringModeOffLeadsToCorrectString)
{
    ::testing::Test::RecordProperty("TEST_ID", "09670bca-f358-475e-b202-cb56c09d825a");
    auto sut = MonitoringMode::OFF;

    {
        auto logstream = iox::log::LogStream(m_loggerMock);
        logstream << sut;
    }

    ASSERT_THAT(m_loggerMock.m_logs.size(), Eq(1U));
    EXPECT_THAT(m_loggerMock.m_logs[0].message, Eq("MonitoringMode::OFF"));
    EXPECT_THAT(m_loggerMock.m_logs[0].level, Eq(iox::log::LogLevel::kWarn));
}

TEST_F(MonitoringModeLogStreamTest, MonitoringModeOnLeadsToCorrectString)
{
    ::testing::Test::RecordProperty("TEST_ID", "0747ce58-58de-449a-ac07-9bffe2b33435");
    auto sut = MonitoringMode::ON;

    {
        auto logstream = iox::log::LogStream(m_loggerMock);
        logstream << sut;
    }

    ASSERT_THAT(m_loggerMock.m_logs.size(), Eq(1U));
    EXPECT_THAT(m_loggerMock.m_logs[0].message, Eq("MonitoringMode::ON"));
    EXPECT_THAT(m_loggerMock.m_logs[0].level, Eq(iox::log::LogLevel::kWarn));
}

} // namespace

#endif
