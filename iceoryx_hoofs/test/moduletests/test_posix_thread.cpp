// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/thread.hpp"
#include "test.hpp"

#include <atomic>
#include <thread>

namespace
{
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
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    std::atomic_bool m_run{true};
    std::thread* m_thread;
};

#if !defined(__APPLE__)
TEST(pthreadWrapper_test, CreateJoinableThread)
{
    optional<thread> sut;
    function<void()> callable = []() { std::cout << "Bleib sauber, Kpt. Blaubaer" << std::endl; };
    ASSERT_FALSE(ThreadBuilder().create(sut, callable).has_error());
    EXPECT_TRUE(sut->joinable());
}

TEST(pthreadWrapper_test, CreateThreadWithEmptyCallable)
{
    optional<thread> sut1;
    function<void()> callable;
    auto result = ThreadBuilder().create(sut1, callable);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ThreadError::EMPTY_CALLABLE));

    optional<thread> sut2;
    callable = []() {};
    ASSERT_FALSE(ThreadBuilder().create(sut2, callable).has_error());
}

TEST(pthreadWrapper_test, CreateDetachedThread)
{
    optional<thread> sut;
    function<void()> callable = []() {
        std::cout << "Set the controls for ... " << std::endl;
        std::cout << "... the heart of the sun" << std::endl;
    };
    auto result = ThreadBuilder().detached(true).create(sut, callable);
    ASSERT_FALSE(result.has_error());
    EXPECT_FALSE(sut->joinable());
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST(pthreadWrapper_test, SetAndGetWithEmptyThreadNameIsWorking)
{
    optional<thread> sut;
    function<void()> callable = []() {};
    ASSERT_FALSE(ThreadBuilder().create(sut, callable).has_error());
    ThreadName_t emptyString = "";

    sut->setThreadName(emptyString);
    auto getResult = sut->getThreadName();

    EXPECT_THAT(getResult, StrEq(emptyString));
}

TEST(pthreadWrapper_test, SetAndGetWithThreadNameCapacityIsWorking)
{
    optional<thread> sut;
    function<void()> callable = []() {};
    ASSERT_FALSE(ThreadBuilder().create(sut, callable).has_error());
    ThreadName_t stringEqualToThreadNameCapacitiy = "123456789ABCDEF";
    EXPECT_THAT(stringEqualToThreadNameCapacitiy.capacity(), Eq(stringEqualToThreadNameCapacitiy.size()));

    sut->setThreadName(stringEqualToThreadNameCapacitiy);
    auto getResult = sut->getThreadName();

    EXPECT_THAT(getResult, StrEq(stringEqualToThreadNameCapacitiy));
}

TEST(pthreadWrapper_test, SetAndGetSmallStringIsWorking)
{
    optional<thread> sut;
    function<void()> callable = []() {};
    ASSERT_FALSE(ThreadBuilder().create(sut, callable).has_error());
    char stringShorterThanThreadNameCapacitiy[] = "I'm short";

    sut->setThreadName(stringShorterThanThreadNameCapacitiy);
    auto getResult = sut->getThreadName();

    EXPECT_THAT(getResult, StrEq(stringShorterThanThreadNameCapacitiy));
}
#endif
} // namespace
