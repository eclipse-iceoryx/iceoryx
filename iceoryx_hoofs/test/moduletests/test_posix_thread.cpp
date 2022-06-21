// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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
    }

    void TearDown()
    {
    }

    ~Thread_test()
    {
    }

    optional<Thread> sut;
};

#if !defined(_WIN32) && !defined(__APPLE__)
TEST_F(Thread_test, CreateThreadWithNonEmptyCallableSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d1e439d-c84e-4a46-ac45-dc8be7530c32");
    bool callableWasCalled = false;
    Thread::callable_t callable = [&] { callableWasCalled = true; };
    ASSERT_FALSE(ThreadBuilder().create(sut, callable).has_error());
    sut.reset();
    EXPECT_TRUE(callableWasCalled);
}

TEST_F(Thread_test, CreateThreadWithEmptyCallableFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "8058c282-ce33-42eb-80ed-4421ebac5652");
    Thread::callable_t callable;
    auto result = ThreadBuilder().create(sut, callable);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(ThreadError::EMPTY_CALLABLE));
}

TEST_F(Thread_test, SetAndGetWithEmptyThreadNameIsWorking)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba2ed4d9-f051-4ad1-a2df-6741134c494f");
    ASSERT_FALSE(
        ThreadBuilder().create(sut, [] { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }).has_error());
    ThreadName_t emptyString = "";

    sut->setName(emptyString);
    auto getResult = sut->getName();

    EXPECT_THAT(getResult, StrEq(emptyString));
}

TEST_F(Thread_test, SetAndGetWithThreadNameCapacityIsWorking)
{
    ::testing::Test::RecordProperty("TEST_ID", "a67128fe-a779-4bdb-a849-3bcbfed4b20f");
    ASSERT_FALSE(
        ThreadBuilder().create(sut, [] { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }).has_error());
    ThreadName_t stringEqualToThreadNameCapacitiy = "123456789ABCDEF";
    EXPECT_THAT(stringEqualToThreadNameCapacitiy.capacity(), Eq(stringEqualToThreadNameCapacitiy.size()));

    sut->setName(stringEqualToThreadNameCapacitiy);
    auto getResult = sut->getName();

    EXPECT_THAT(getResult, StrEq(stringEqualToThreadNameCapacitiy));
}

TEST_F(Thread_test, SetAndGetSmallStringIsWorking)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5141d3c-2721-478c-b3d1-f35fb3321117");
    ASSERT_FALSE(
        ThreadBuilder().create(sut, [] { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }).has_error());
    char stringShorterThanThreadNameCapacitiy[] = "I'm short";

    sut->setName(stringShorterThanThreadNameCapacitiy);
    auto getResult = sut->getName();

    EXPECT_THAT(getResult, StrEq(stringShorterThanThreadNameCapacitiy));
}
#endif
} // namespace
