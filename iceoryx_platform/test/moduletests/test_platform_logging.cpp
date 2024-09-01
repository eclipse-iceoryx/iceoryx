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

#include "iceoryx_platform/atomic.hpp"
#include "iceoryx_platform/logging.hpp"

#include "test.hpp"

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>

namespace
{
using namespace ::testing;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) for testing only
iox::concurrent::Atomic<bool> has_custom_backend{false};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) for testing only
iox::concurrent::Atomic<uint64_t> dummy_backend_output_counter{0};

class LogOutput
{
  public:
    void set(IceoryxPlatformLogLevel log_level, const char* msg) noexcept
    {
        std::lock_guard<std::mutex> lock(m_mtx);

        m_log_level = log_level;
        m_log_msg = msg;
    }

    void clear() noexcept
    {
        std::lock_guard<std::mutex> lock(m_mtx);

        m_log_level = IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_TRACE;
        m_log_msg.clear();
    }

    std::pair<IceoryxPlatformLogLevel, std::string> get() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_mtx);

        return std::make_pair(m_log_level, m_log_msg);
    }

  private:
    mutable std::mutex m_mtx;
    IceoryxPlatformLogLevel m_log_level{IceoryxPlatformLogLevel::IOX_PLATFORM_LOG_LEVEL_TRACE};
    std::string m_log_msg;
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) for testing only
LogOutput last_log_output;

// NOLINTNEXTLINE(readability-function-size) not directly called; just for testing
void custom_log_backend(
    const char* file, int line, const char* function, IceoryxPlatformLogLevel log_level, const char* msg)
{
    has_custom_backend = true;
    last_log_output.set(log_level, msg);
    iox_platform_detail_default_log_backend(file, line, function, log_level, msg);
}

// NOLINTNEXTLINE(readability-function-size) not directly called; just for testing
void dummy_log_backend(
    const char* file, int line, const char* function, IceoryxPlatformLogLevel log_level, const char* msg)
{
    dummy_backend_output_counter.fetch_add(1);
    last_log_output.set(log_level, msg);
    iox_platform_detail_default_log_backend(file, line, function, log_level, msg);
}

TEST(Logging_test, DefaultBackendLogsToCout)
{
    ::testing::Test::RecordProperty("TEST_ID", "a1d5fbb5-38e3-4829-bf1a-4ea312115f12");

    ASSERT_FALSE(has_custom_backend);

    std::stringstream buffer;
    std::streambuf* original_cout_buffer = std::cout.rdbuf(buffer.rdbuf());

    constexpr const char* MESSAGE{"Hypnotoad will rock you"};

    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_INFO, MESSAGE);

    std::string log = buffer.str();
    EXPECT_THAT(log, HasSubstr(MESSAGE));

    std::cout.rdbuf(original_cout_buffer);
}

TEST(Logging_test, SettingBackendWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "85029c9d-3be2-463c-838a-9fce01e10640");

    ASSERT_FALSE(has_custom_backend);

    iox_platform_set_log_backend(nullptr);

    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_INFO, "Hypnotoad");
    ASSERT_FALSE(has_custom_backend);
}

TEST(Logging_test, SettingCustomBackendWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9348e6ef-30a4-4cd7-a3d3-264efe378fba");

    ASSERT_FALSE(has_custom_backend);

    iox_platform_set_log_backend(&custom_log_backend);

    last_log_output.clear();

    constexpr const char* MESSAGE{"Who will rock you?"};
    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_INFO, MESSAGE);

    auto [log_level, log_msg] = last_log_output.get();
    EXPECT_THAT(log_level, Eq(IOX_PLATFORM_LOG_LEVEL_INFO));
    EXPECT_THAT(log_msg, StrEq(MESSAGE));
}

TEST(Logging_test, SettingCustomBackendTwiceFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "cbc3446e-5c02-4f46-933f-c91627f669d3");

    ASSERT_TRUE(has_custom_backend);

    constexpr uint64_t DUMMY_MESSAGE_COUNT_AFTER_FAILED_SETUP{1};
    ASSERT_THAT(dummy_backend_output_counter.load(), Eq(0));
    iox_platform_set_log_backend(&dummy_log_backend);
    ASSERT_THAT(dummy_backend_output_counter.load(), Eq(DUMMY_MESSAGE_COUNT_AFTER_FAILED_SETUP));

    IOX_PLATFORM_LOG(IOX_PLATFORM_LOG_LEVEL_INFO, "Hypnotoad");

    ASSERT_THAT(dummy_backend_output_counter.load(), Eq(DUMMY_MESSAGE_COUNT_AFTER_FAILED_SETUP));
}

} // namespace
