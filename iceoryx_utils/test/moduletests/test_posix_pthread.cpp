// Copyright (c) 2020 by Apex.AI. All rights reserved.
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

#include "iceoryx_utils/posix_wrapper/pthread.hpp"
#include "test.hpp"

#include <atomic>
#include <thread>

using namespace ::testing;
using namespace iox::posix;

class PThread_test : public Test
{
  public:
    PThread_test()
    {
    }

    void SetUp()
    {
        m_run = true;
        m_thread = std::thread(threadFunc, this);
    }

    void TearDown()
    {
        m_run = false;
        m_thread.join();
    }

    ~PThread_test()
    {
    }

    void threadFunc()
    {
        while (!m_run)
        {
        }
    };

    std::atomic_bool m_run{true};
    std::thread m_thread;
};

TEST_F(PThread_test, LargeStringIsTruncated)
{
    constexpr char stringLongerThan16Chars[] =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
        "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
        "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.";
    char threadName[15];

    auto result = setThreadName(m_thread.native_handle(), stringLongerThan16Chars);
    pthread_getname_np(m_thread.native_handle(), threadName, 16);

    EXPECT_THAT(stringLongerThan16Chars, StrNe(threadName));
    EXPECT_THAT(result.has_error(), Eq(false));
}

TEST_F(PThread_test, SmallStringIsNotTruncated)
{
    constexpr char stringShorterThan16Chars[] = "I'm short";
    char threadName[15];

    auto result = setThreadName(m_thread.native_handle(), stringShorterThan16Chars);
    pthread_getname_np(m_thread.native_handle(), threadName, 16);

    EXPECT_THAT(stringShorterThan16Chars, StrEq(threadName));
    EXPECT_THAT(result.has_error(), Eq(false));
}