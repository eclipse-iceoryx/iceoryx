// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include <gtest/gtest.h>

#if defined(_WIN32) || defined(_WIN64)
#define GTEST_SKIP_FOR_WINDOWS() GTEST_SKIP() << "Skipping this test on Windows."
#else
#define GTEST_SKIP_FOR_WINDOWS() (void)0
#endif

#ifdef USE_SYSTEMD_TEST
#define GTEST_SKIP_NOT_SUPPORT_SYSTEMD() (void)0
#else
#define GTEST_SKIP_NOT_SUPPORT_SYSTEMD() GTEST_SKIP() << "Skipping this test when systemd is not use."
#endif


TEST(RoudiSystemD, CreateObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "aa77b5f6-ffb3-4267-982d-dfe85da384ca");
    GTEST_SKIP_NOT_SUPPORT_SYSTEMD();
    std::unique_ptr<SendMessageServiceManagement> roudiSendMessage;
    ASSERT_NO_THROW(roudiSendMessage = std::make_unique<SendMessageServiceManagement>());
}

TEST(RoudiSystemD, CheckConstantsSizeThreadName)
{
    ::testing::Test::RecordProperty("TEST_ID", "9c39f45c-a63c-43ec-9606-e50c33247b3f");
    GTEST_SKIP_NOT_SUPPORT_SYSTEMD();
    std::unique_ptr<SendMessageServiceManagement> roudiSendMessage;
    ASSERT_NO_THROW(roudiSendMessage = std::make_unique<SendMessageServiceManagement>());
    ASSERT_EQ(roudiSendMessage->SIZE_THREAD_NAME, 15) << "Size thread must equal 15 simbols";
}

TEST(RoudiSystemD, CheckConstantsSizeString)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b3e3058-6052-49cc-8a67-723f3775a745");
    GTEST_SKIP_NOT_SUPPORT_SYSTEMD();
    std::unique_ptr<SendMessageServiceManagement> roudiSendMessage;
    ASSERT_NO_THROW(roudiSendMessage = std::make_unique<SendMessageServiceManagement>());
    ASSERT_EQ(roudiSendMessage->SIZE_STRING, 4096) << "Size string must equal 4096 simbols";
}

TEST(RoudiSystemD, SetThreadNameHelper)
{
    ::testing::Test::RecordProperty("TEST_ID", "b9ff9e83-9dde-4221-bd1e-c1016ec2d5ff");
    GTEST_SKIP_FOR_WINDOWS();
    GTEST_SKIP_NOT_SUPPORT_SYSTEMD();
#ifdef USE_SYSTEMD_TEST
    std::unique_ptr<SendMessageServiceManagement> roudiSendMessage;
    bool result = true;

    ASSERT_NO_THROW(roudiSendMessage = std::make_unique<SendMessageServiceManagement>());
    iox::string<SendMessageServiceManagement::SIZE_THREAD_NAME> nameThread = "test";
    ASSERT_NO_THROW(result = roudiSendMessage->setThreadNameHelper(nameThread));
    ASSERT_EQ(result, true) << "Can not change name thread";
#else
    /* need add test (other OS) */
    ASSERT_EQ(true, true);
#endif
}

TEST(RoudiSystemD, GetEnvironmentVariableReturnsCorrectValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "12dfa746-d1f1-4b4e-864d-2cb28ee49f70");
    GTEST_SKIP_FOR_WINDOWS();
    GTEST_SKIP_NOT_SUPPORT_SYSTEMD();
#ifdef USE_SYSTEMD_TEST
    const char* const env_var_name = "TEST_ENV_VAR";
    const char* const env_var_value = "test_value";

    auto set_env = IOX_POSIX_CALL(setenv)(env_var_name, env_var_value, 1).failureReturnValue(-1).evaluate();
    EXPECT_FALSE(set_env.has_error()) << "setenv failed with error: " << set_env.get_error().errnum;

    SendMessageServiceManagement sut;

    std::string result = sut.getEnvironmentVariable(env_var_name);
    if (result != "no implement")
    {
        EXPECT_EQ(result, env_var_value);
    }
    else
    {
        EXPECT_EQ(result, "no implement");
    }
#else
    /* need add test (other OS) */
    ASSERT_EQ(true, true);
#endif
}

TEST(RoudiSystemD, GetEnvironmentVariableHandlesNonExistentVar)
{
    ::testing::Test::RecordProperty("TEST_ID", "9595728f-a504-46e3-8672-b074696326a4");
    GTEST_SKIP_FOR_WINDOWS();
    GTEST_SKIP_NOT_SUPPORT_SYSTEMD();
#ifdef USE_SYSTEMD_TEST
    SendMessageServiceManagement sut;

    std::string result = sut.getEnvironmentVariable("NON_EXISTENT_VAR");
    if (result != "no implement")
    {
        EXPECT_TRUE(result.empty());
    }
    else
    {
        EXPECT_EQ(result, "no implement");
    }
#else
    /* need add test (other OS) */
    ASSERT_EQ(true, true);
#endif
}