// Copyright (c) 2021 - 2022 Apex.AI Inc. All rights reserved.
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

#include "iox/logging.hpp"

extern "C" {
#include "iceoryx_binding_c/log.h"
}

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::log;

TEST(iox_log_test, LogLevelIsSetCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "cbfc1b9d-d770-441c-b53d-11b1685ca0b4");
    auto& logger = iox::log::Logger::get();

    iox_set_loglevel(Iceoryx_LogLevel_Off);
    EXPECT_EQ(logger.getLogLevel(), LogLevel::OFF);

    iox_set_loglevel(Iceoryx_LogLevel_Fatal);
    EXPECT_EQ(logger.getLogLevel(), LogLevel::FATAL);

    iox_set_loglevel(Iceoryx_LogLevel_Error);
    EXPECT_EQ(logger.getLogLevel(), LogLevel::ERROR);

    iox_set_loglevel(Iceoryx_LogLevel_Warn);
    EXPECT_EQ(logger.getLogLevel(), LogLevel::WARN);

    iox_set_loglevel(Iceoryx_LogLevel_Info);
    EXPECT_EQ(logger.getLogLevel(), LogLevel::INFO);

    iox_set_loglevel(Iceoryx_LogLevel_Debug);
    EXPECT_EQ(logger.getLogLevel(), LogLevel::DEBUG);

    iox_set_loglevel(Iceoryx_LogLevel_Trace);
    EXPECT_EQ(logger.getLogLevel(), LogLevel::TRACE);
}

} // namespace
