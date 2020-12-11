// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_utils/posix_wrapper/thread.hpp"
#include "test.hpp"

#include <atomic>
#include <thread>

using namespace ::testing;
using namespace iox::posix;
using namespace iox::cxx;

class Thread_test : public Test
{
  public:
    Thread_test()
    {
    }

    void SetUp()
    {
        m_run = true;
        m_thread = new std::thread(&Thread_test::threadFunc, this);
    }

    void TearDown()
    {
        m_run = false;
        m_thread->join();
        delete m_thread;
    }

    ~Thread_test()
    {
    }

    void threadFunc()
    {
        while (m_run)
        {
        }
    };

    std::atomic_bool m_run{true};
    std::thread* m_thread;
};

TEST_F(Thread_test, DISABLED_LargeStringIsTruncated)
{
/// @todo Renable this test, once "does not compile" tests are possible
#if 0
    constexpr char stringLongerThan16Chars[] =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
        "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
        "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.";

    auto setResult = setThreadName(m_thread->native_handle(), stringLongerThan16Chars);
#endif
}

TEST_F(Thread_test, SmallStringIsNotTruncated)
{
    char stringShorterThan16Chars[] = "I'm short";
    ThreadName_t threadName;

    auto setResult = setThreadName(m_thread->native_handle(), stringShorterThan16Chars);
    auto getResult = getThreadName(m_thread->native_handle());

    EXPECT_THAT(setResult.has_error(), Eq(false));
    EXPECT_THAT(getResult.has_error(), Eq(false));
    EXPECT_THAT(stringShorterThan16Chars, StrEq(getResult.value()));
}
