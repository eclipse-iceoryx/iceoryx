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

#include "iceoryx_platform/stdlib.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

constexpr int IOX_TEST_RET_OK{0};
constexpr int IOX_TEST_RET_NOK{-1};

TEST(STDLIB_test, SetenvWorksWhenEnvVarDoesNotExist)
{
    ::testing::Test::RecordProperty("TEST_ID", "1067d206-67c4-4521-84b7-c64300ba7759");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr int OVERWRITE_FLAGE{0};

    auto env_var = std::getenv(ENV_VAR_NAME);
    ASSERT_THAT(env_var, Eq(nullptr));

    auto ret_val = iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_OK));

    env_var = std::getenv(ENV_VAR_NAME);
    ASSERT_THAT(env_var, Ne(nullptr));
    EXPECT_THAT(env_var, StrEq(ENV_VAR_VALUE));
}

TEST(STDLIB_test, SetenvDoesNotOverrideWhenNotAskedTo)
{
    ::testing::Test::RecordProperty("TEST_ID", "86215812-a8db-49e4-811c-996c09801686");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_SETUP_VALUE{"brain-slug"};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr int OVERWRITE_FLAGE_SETUP{1};
    constexpr int OVERWRITE_FLAGE{0};

    ASSERT_THAT(iox_setenv(ENV_VAR_NAME, ENV_VAR_SETUP_VALUE, OVERWRITE_FLAGE_SETUP), Eq(IOX_TEST_RET_OK));

    auto ret_val = iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_OK));
    auto env_var = std::getenv(ENV_VAR_NAME);
    ASSERT_THAT(env_var, Ne(nullptr));
    EXPECT_THAT(env_var, StrEq(ENV_VAR_SETUP_VALUE));
}

TEST(STDLIB_test, SetenvDoesOverrideWhenAskedTo)
{
    ::testing::Test::RecordProperty("TEST_ID", "e71ca718-121c-4003-8876-006b08692555");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_SETUP_VALUE{"brain-slug"};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr int OVERWRITE_FLAGE_SETUP{1};
    constexpr int OVERWRITE_FLAGE{1};

    ASSERT_THAT(iox_setenv(ENV_VAR_NAME, ENV_VAR_SETUP_VALUE, OVERWRITE_FLAGE_SETUP), Eq(IOX_TEST_RET_OK));

    auto ret_val = iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_OK));
    auto env_var = std::getenv(ENV_VAR_NAME);
    ASSERT_THAT(env_var, Ne(nullptr));
    EXPECT_THAT(env_var, StrEq(ENV_VAR_VALUE));
}

TEST(STDLIB_test, SetenvMakesDeepCopyOfTheValueString)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b272d88-900c-4625-ab04-27f8f0fa1d72");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr const char* ENV_VAR_DUMMY_VALUE{"brain-slug"};
    constexpr int OVERWRITE_FLAGE{1};

    constexpr size_t CAPACITY{100};
    char env_var_value[CAPACITY];
    strncpy(env_var_value, ENV_VAR_VALUE, CAPACITY);

    ASSERT_THAT(iox_setenv(ENV_VAR_NAME, env_var_value, OVERWRITE_FLAGE), Eq(IOX_TEST_RET_OK));

    strncpy(env_var_value, ENV_VAR_DUMMY_VALUE, CAPACITY);

    auto env_var = std::getenv(ENV_VAR_NAME);
    ASSERT_THAT(env_var, Ne(nullptr));
    EXPECT_THAT(env_var, StrEq(ENV_VAR_VALUE));
}

TEST(STDLIB_test, SetenvFailsWhenNameIsANullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "7caa4c23-bfde-4bfd-8a08-b7e3ee43745a");

    constexpr const char* ENV_VAR_NAME{nullptr};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr int OVERWRITE_FLAGE{1};

    auto ret_val = iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_NOK));
    EXPECT_THAT(errno, Eq(EINVAL));
}

TEST(STDLIB_test, SetenvFailsWhenValueIsANullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "cd313bb6-2140-4ef2-80d8-676404f1c208");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_VALUE{nullptr};
    constexpr int OVERWRITE_FLAGE{1};

    auto ret_val = iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_NOK));
    EXPECT_THAT(errno, Eq(EINVAL));
}

TEST(STDLIB_test, UnsetenvWorksWhenEnvVarExists)
{
    ::testing::Test::RecordProperty("TEST_ID", "bf5907f4-9782-4c41-be4d-f7fea2dc1d7f");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr int OVERWRITE_FLAGE{1};

    ASSERT_THAT(iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE), Eq(IOX_TEST_RET_OK));

    auto ret_val = iox_unsetenv(ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_OK));
    auto env_var = std::getenv(ENV_VAR_NAME);
    ASSERT_THAT(env_var, Eq(nullptr));
}

TEST(STDLIB_test, UnsetenvWorksWhenEnvVarDosNotExist)
{
    ::testing::Test::RecordProperty("TEST_ID", "f20ab6b4-c8f5-4cf8-a15a-cfc5dd4b3965");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr int OVERWRITE_FLAGE{1};

    ASSERT_THAT(iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE), Eq(IOX_TEST_RET_OK));
    ASSERT_THAT(iox_unsetenv(ENV_VAR_NAME), Eq(IOX_TEST_RET_OK));

    auto ret_val = iox_unsetenv(ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_OK));
    auto env_var = std::getenv(ENV_VAR_NAME);
    ASSERT_THAT(env_var, Eq(nullptr));
}

TEST(STDLIB_test, UnsetenvFailsWhenNameIsANullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "dbb69dee-94af-4b7c-9b56-da4971fa90d4");

    constexpr const char* ENV_VAR_NAME{nullptr};

    auto ret_val = iox_unsetenv(ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_NOK));
    EXPECT_THAT(errno, Eq(EINVAL));
}

TEST(STDLIB_test, GetenvWorksWhenEnvVarDoesExist)
{
    ::testing::Test::RecordProperty("TEST_ID", "fc369d77-c1c7-4103-9b7a-37038848f1e8");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr int OVERWRITE_FLAGE{1};

    ASSERT_THAT(iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE), Eq(IOX_TEST_RET_OK));

    size_t actual_size_with_null{0};
    constexpr size_t CAPACITY{100};
    char env_var[CAPACITY];
    auto ret_val = iox_getenv_s(&actual_size_with_null, env_var, CAPACITY, ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_OK));
    EXPECT_THAT(actual_size_with_null, Eq(strlen(ENV_VAR_VALUE) + 1));
    EXPECT_THAT(env_var, StrEq(ENV_VAR_VALUE));
}

TEST(STDLIB_test, GetenvWorksWhenEnvVarDoesNotExist)
{
    ::testing::Test::RecordProperty("TEST_ID", "59d3f862-c2ad-4c32-8c01-31f7f29c9804");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};

    ASSERT_THAT(iox_unsetenv(ENV_VAR_NAME), Eq(IOX_TEST_RET_OK));

    size_t actual_size_with_null{0};
    constexpr size_t CAPACITY{100};
    char env_var[CAPACITY];
    auto ret_val = iox_getenv_s(&actual_size_with_null, env_var, CAPACITY, ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_OK));
    EXPECT_THAT(actual_size_with_null, Eq(0));
    EXPECT_THAT(strnlen(env_var, CAPACITY), Eq(0));
}

TEST(STDLIB_test, GetenvWorksWhenActualSizeWithNullParameterIsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "e61d8189-d701-4265-a508-8a121acac27b");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};

    ASSERT_THAT(iox_unsetenv(ENV_VAR_NAME), Eq(IOX_TEST_RET_OK));

    size_t* actual_size_with_null{nullptr};
    constexpr size_t CAPACITY{100};
    char env_var[CAPACITY];
    auto ret_val = iox_getenv_s(actual_size_with_null, env_var, CAPACITY, ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_OK));
    EXPECT_THAT(strnlen(env_var, CAPACITY), Eq(0));
}

TEST(STDLIB_test, GetenvWorksWhenWhenBufferHasExactFittingCapacityForEnvVar)
{
    ::testing::Test::RecordProperty("TEST_ID", "bf3cb865-f377-453a-a23a-cd91b03a8f86");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr int OVERWRITE_FLAGE{1};

    ASSERT_THAT(iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE), Eq(IOX_TEST_RET_OK));

    size_t actual_size_with_null{0};
    constexpr size_t CAPACITY{10};
    constexpr size_t NULL_TERMINATOR_SIZE{1};
    ASSERT_THAT(CAPACITY, Eq(strlen(ENV_VAR_VALUE) + NULL_TERMINATOR_SIZE));
    char env_var[CAPACITY];
    auto ret_val = iox_getenv_s(&actual_size_with_null, env_var, CAPACITY, ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(IOX_TEST_RET_OK));
    EXPECT_THAT(actual_size_with_null, Eq(strlen(ENV_VAR_VALUE) + 1));
    EXPECT_THAT(env_var, StrEq(ENV_VAR_VALUE));
}

TEST(STDLIB_test, GetenvSetsActualSizeWhenBufferIsTooSmallAndFailsWithErrno)
{
    ::testing::Test::RecordProperty("TEST_ID", "59068748-b793-491f-9885-6083b20cc551");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr int OVERWRITE_FLAGE{1};

    ASSERT_THAT(iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE), Eq(IOX_TEST_RET_OK));

    size_t actual_size_with_null{0};
    constexpr size_t CAPACITY{9};
    ASSERT_THAT(CAPACITY, Eq(strlen(ENV_VAR_VALUE)));
    char env_var[CAPACITY];
    auto ret_val = iox_getenv_s(&actual_size_with_null, env_var, CAPACITY, ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(ERANGE));
    constexpr size_t NULL_TERMINATOR_SIZE{1};
    EXPECT_THAT(actual_size_with_null, Eq(strlen(ENV_VAR_VALUE) + NULL_TERMINATOR_SIZE));
    EXPECT_THAT(strnlen(env_var, CAPACITY), Eq(0));
}

TEST(STDLIB_test, GetenvSetsActualSizeWhenBufferIsNullptrAndFailsWithErrno)
{
    ::testing::Test::RecordProperty("TEST_ID", "ffee85d0-a8ca-4028-aea8-0d472924da62");

    constexpr const char* ENV_VAR_NAME{"IOX_PLATFORM_SETENV_TEST"};
    constexpr const char* ENV_VAR_VALUE{"hypnotoad"};
    constexpr int OVERWRITE_FLAGE{1};

    ASSERT_THAT(iox_setenv(ENV_VAR_NAME, ENV_VAR_VALUE, OVERWRITE_FLAGE), Eq(IOX_TEST_RET_OK));

    size_t actual_size_with_null{0};
    char* env_var{nullptr};
    auto ret_val = iox_getenv_s(&actual_size_with_null, env_var, 0, ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(ERANGE));
    constexpr size_t NULL_TERMINATOR_SIZE{1};
    EXPECT_THAT(actual_size_with_null, Eq(strlen(ENV_VAR_VALUE) + NULL_TERMINATOR_SIZE));
}

TEST(STDLIB_test, GetenvFailsWhenNameIsANullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a225ec0-be73-4beb-b317-1611dcad8e77");

    constexpr const char* ENV_VAR_NAME{nullptr};

    size_t actual_size_with_null{0};
    constexpr size_t CAPACITY{100};
    char env_var[CAPACITY];
    auto ret_val = iox_getenv_s(&actual_size_with_null, env_var, CAPACITY, ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(EINVAL));
}

TEST(STDLIB_test, GetenvFailsWhenBufferIsANullptrButBufferSizeIsNotNull)
{
    ::testing::Test::RecordProperty("TEST_ID", "81e31a8c-5393-4429-a9c8-488b64265066");

    constexpr const char* ENV_VAR_NAME{nullptr};

    size_t actual_size_with_null{0};
    constexpr size_t CAPACITY{100};
    char* env_var{nullptr};
    auto ret_val = iox_getenv_s(&actual_size_with_null, env_var, CAPACITY, ENV_VAR_NAME);

    ASSERT_THAT(ret_val, Eq(EINVAL));
}

} // namespace
