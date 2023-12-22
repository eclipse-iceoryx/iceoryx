// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/named_semaphore.hpp"
#include "test.hpp"
#include "test_posix_semaphore_common.hpp"

namespace
{
using namespace ::testing;
using namespace iox;

class NamedSemaphoreTest : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    optional<NamedSemaphore> sut;
    NamedSemaphore::Name_t sutName{TruncateToCapacity, "dr.peacock_rocks"};
    const iox::access_rights sutPermission = iox::perms::owner_all;
};

TEST_F(NamedSemaphoreTest, DefaultInitialValueIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "71bf2c59-f0cd-4946-93ba-b578b99ac596");
    ASSERT_FALSE(NamedSemaphoreBuilder()
                     .name(sutName)
                     .openMode(OpenMode::PURGE_AND_CREATE)
                     .permissions(sutPermission)
                     .create(sut)
                     .has_error());
    EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut, 0U));
}

TEST_F(NamedSemaphoreTest, InitialValueIsSetOnCreation)
{
    ::testing::Test::RecordProperty("TEST_ID", "c7ba4432-1c16-4b14-bd98-391600c2b87d");

    constexpr uint32_t INITIAL_VALUE = 18739U;
    ASSERT_FALSE(NamedSemaphoreBuilder()
                     .name(sutName)
                     .openMode(OpenMode::PURGE_AND_CREATE)
                     .permissions(sutPermission)
                     .initialValue(INITIAL_VALUE)
                     .create(sut)
                     .has_error());

    EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut, INITIAL_VALUE));
}

TEST_F(NamedSemaphoreTest, OpenExistingSemaphoreDoesNotOverrideInitialValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "08100ca3-86eb-436b-aa63-32957c2cf849");

    constexpr uint32_t INITIAL_VALUE = 8812U;
    ASSERT_FALSE(NamedSemaphoreBuilder()
                     .name(sutName)
                     .openMode(OpenMode::PURGE_AND_CREATE)
                     .permissions(sutPermission)
                     .initialValue(INITIAL_VALUE)
                     .create(sut)
                     .has_error());

    {
        iox::optional<NamedSemaphore> sut2;
        ASSERT_FALSE(NamedSemaphoreBuilder()
                         .name(sutName)
                         .openMode(OpenMode::OPEN_EXISTING)
                         .initialValue(INITIAL_VALUE + 512)
                         .create(sut2)
                         .has_error());
        EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut2, INITIAL_VALUE));
    }
}

TEST_F(NamedSemaphoreTest, OpenExistingSemaphoreWorksWithoutDestroyingItInTheDtor)
{
    ::testing::Test::RecordProperty("TEST_ID", "01274f45-74c0-43c2-95b1-f80afc78f017");

    constexpr uint32_t INITIAL_VALUE = 1872U;
    ASSERT_FALSE(NamedSemaphoreBuilder()
                     .name(sutName)
                     .openMode(OpenMode::PURGE_AND_CREATE)
                     .permissions(sutPermission)
                     .initialValue(INITIAL_VALUE)
                     .create(sut)
                     .has_error());

    {
        iox::optional<NamedSemaphore> sut2;
        ASSERT_FALSE(NamedSemaphoreBuilder().name(sutName).openMode(OpenMode::OPEN_EXISTING).create(sut2).has_error());
        EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut2, INITIAL_VALUE));
        ASSERT_TRUE(setSemaphoreValueTo(*sut2, INITIAL_VALUE));
    }

    // if the dtor of sut2 unlinks the semaphore we should be unable to open it again
    {
        iox::optional<NamedSemaphore> sut2;
        ASSERT_FALSE(NamedSemaphoreBuilder().name(sutName).openMode(OpenMode::OPEN_EXISTING).create(sut2).has_error());
        EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut2, INITIAL_VALUE));
        ASSERT_TRUE(setSemaphoreValueTo(*sut2, INITIAL_VALUE));
    }

    // verify that the original semaphore which has the ownership still works
    EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut, INITIAL_VALUE));
}

TEST_F(NamedSemaphoreTest, OpenNonExistingSemaphoreFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec124aaf-aad1-4375-a288-b3b06dbc6ac2");

    auto result =
        NamedSemaphoreBuilder().name(sutName).openMode(OpenMode::OPEN_EXISTING).permissions(sutPermission).create(sut);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(SemaphoreError::NO_SEMAPHORE_WITH_THAT_NAME_EXISTS));
}

TEST_F(NamedSemaphoreTest, ExclusiveCreateFailsWhenSemaphoreAlreadyExists)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec2c4ab4-dc63-4a4c-b66f-fded785ef4f0");

    ASSERT_FALSE(NamedSemaphoreBuilder()
                     .name(sutName)
                     .openMode(OpenMode::PURGE_AND_CREATE)
                     .permissions(sutPermission)
                     .create(sut)
                     .has_error());

    iox::optional<NamedSemaphore> sut2;
    auto result = NamedSemaphoreBuilder()
                      .name(sutName)
                      .openMode(OpenMode::EXCLUSIVE_CREATE)
                      .permissions(sutPermission)
                      .create(sut2);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(SemaphoreError::ALREADY_EXIST));
}

TEST_F(NamedSemaphoreTest, SemaphoreWithInvalidNameFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "07ef3fdf-0e22-45f8-bb6b-68647f1211b6");

    auto result =
        NamedSemaphoreBuilder().name("///").openMode(OpenMode::PURGE_AND_CREATE).permissions(sutPermission).create(sut);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(SemaphoreError::INVALID_NAME));
}

TEST_F(NamedSemaphoreTest, OpenOrCreateOpensExistingSemaphoreWithoutDestroyingItInTheDtor)
{
    ::testing::Test::RecordProperty("TEST_ID", "8686f905-d4c3-4432-ad6d-e31572907897");

    constexpr uint32_t INITIAL_VALUE = 655U;
    ASSERT_FALSE(NamedSemaphoreBuilder()
                     .name(sutName)
                     .openMode(OpenMode::OPEN_OR_CREATE)
                     .permissions(sutPermission)
                     .initialValue(INITIAL_VALUE)
                     .create(sut)
                     .has_error());

    {
        iox::optional<NamedSemaphore> sut2;
        ASSERT_FALSE(NamedSemaphoreBuilder()
                         .name(sutName)
                         .initialValue(0U)
                         .openMode(OpenMode::OPEN_OR_CREATE)
                         .create(sut2)
                         .has_error());
        // value should be INITIAL_VALUE since we opened an existing semaphore
        EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut2, INITIAL_VALUE));
        ASSERT_TRUE(setSemaphoreValueTo(*sut2, INITIAL_VALUE));
    }

    // if the dtor of sut2 unlinks the semaphore we should be unable to open it again
    {
        iox::optional<NamedSemaphore> sut2;
        ASSERT_FALSE(NamedSemaphoreBuilder()
                         .name(sutName)
                         .initialValue(0U)
                         .openMode(OpenMode::OPEN_OR_CREATE)
                         .create(sut2)
                         .has_error());
        // value should be INITIAL_VALUE since we opened an existing semaphore
        EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut2, INITIAL_VALUE));
        ASSERT_TRUE(setSemaphoreValueTo(*sut2, INITIAL_VALUE));
    }

    // verify that the original semaphore which has the ownership still works
    EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut, INITIAL_VALUE));
}

TEST_F(NamedSemaphoreTest, OpenOrCreateRemovesSemaphoreWhenItHasTheOwnership)
{
    ::testing::Test::RecordProperty("TEST_ID", "06252f80-42ee-4d81-b25a-5f3b196b9d7c");

    ASSERT_FALSE(NamedSemaphoreBuilder()
                     .name(sutName)
                     .openMode(OpenMode::OPEN_OR_CREATE)
                     .permissions(sutPermission)
                     .create(sut)
                     .has_error());

    sut.reset();

    // should fail since the previous sut was deleted and had the ownership
    auto result = NamedSemaphoreBuilder().name(sutName).initialValue(0U).openMode(OpenMode::OPEN_EXISTING).create(sut);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(SemaphoreError::NO_SEMAPHORE_WITH_THAT_NAME_EXISTS));
}

TEST_F(NamedSemaphoreTest, WhenOwningSemaphoreIsClosedBeforeOpenedSemaphoreTheOpenedSemaphoreRemainsUsable)
{
    ::testing::Test::RecordProperty("TEST_ID", "077fff6c-97d8-4888-b28f-cd45d5289be8");

    constexpr uint32_t INITIAL_VALUE = 84U;
    ASSERT_FALSE(NamedSemaphoreBuilder()
                     .name(sutName)
                     .openMode(OpenMode::EXCLUSIVE_CREATE)
                     .permissions(sutPermission)
                     .initialValue(INITIAL_VALUE)
                     .create(sut)
                     .has_error());

    iox::optional<NamedSemaphore> sut2;
    ASSERT_FALSE(NamedSemaphoreBuilder().name(sutName).openMode(OpenMode::OPEN_EXISTING).create(sut2).has_error());

    sut.reset();

    EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut2, INITIAL_VALUE));
}

// Windows does not support this since the named semaphore is automatically deleted
// as soon as the last handle was closed with CloseHandle
#if !defined(_WIN32)
TEST_F(NamedSemaphoreTest, PurgeAndCreateCreatesNewSemaphore)
{
    ::testing::Test::RecordProperty("TEST_ID", "4b9d055c-b980-4ed6-941b-f93b3810e0e5");

    constexpr uint32_t FIRST_INITIAL_VALUE = 891U;
    ASSERT_FALSE(NamedSemaphoreBuilder()
                     .name(sutName)
                     .openMode(OpenMode::PURGE_AND_CREATE)
                     .permissions(sutPermission)
                     .initialValue(FIRST_INITIAL_VALUE)
                     .create(sut)
                     .has_error());

    iox::optional<NamedSemaphore> sut2;
    constexpr uint32_t INITIAL_VALUE = 97U;
    ASSERT_FALSE(NamedSemaphoreBuilder()
                     .name(sutName)
                     .openMode(OpenMode::PURGE_AND_CREATE)
                     .permissions(sutPermission)
                     .initialValue(INITIAL_VALUE)
                     .create(sut2)
                     .has_error());

    // the newly created semaphore should have the INITIAL_VALUE
    EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut2, INITIAL_VALUE));

    // the original semaphore cannot be opened anymore but the current open handle should
    // still be valid
    EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut, FIRST_INITIAL_VALUE));
}
#endif
} // namespace
