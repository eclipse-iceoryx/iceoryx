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

#include "iceoryx_hoofs/cxx/filesystem.hpp"
#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::cxx;

using permsBaseType_t = std::underlying_type<perms>::type;

TEST(filesystem_test, permsSatisfiesBinaryOrOperationCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b72fcec-c2b3-4a45-801f-542ff3195a2f");
    constexpr perms TEST_VALUE_LHS = perms::others_write;
    constexpr perms TEST_VALUE_RHS = perms::group_all;

    constexpr auto BASE_VALUE_LHS = static_cast<permsBaseType_t>(TEST_VALUE_LHS);
    constexpr auto BASE_VALUE_RHS = static_cast<permsBaseType_t>(TEST_VALUE_RHS);

    EXPECT_THAT(static_cast<permsBaseType_t>(TEST_VALUE_LHS | TEST_VALUE_RHS), Eq(BASE_VALUE_LHS | BASE_VALUE_RHS));
}

TEST(filesystem_test, permsSatisfiesBinaryAndOperationCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "15a02845-21b0-41fb-80bf-ee2ff9a81427");
    constexpr perms TEST_VALUE_LHS = perms::others_read;
    constexpr perms TEST_VALUE_RHS = perms::mask;

    constexpr auto BASE_VALUE_LHS = static_cast<permsBaseType_t>(TEST_VALUE_LHS);
    constexpr auto BASE_VALUE_RHS = static_cast<permsBaseType_t>(TEST_VALUE_RHS);

    EXPECT_THAT(static_cast<permsBaseType_t>(TEST_VALUE_LHS & TEST_VALUE_RHS), Eq(BASE_VALUE_LHS & BASE_VALUE_RHS));
}

TEST(filesystem_test, permsSatisfiesBinaryExclusiveOrOperationCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "8094a263-2861-45ad-aecd-9312d477bc2d");
    constexpr perms TEST_VALUE_LHS = perms::set_gid;
    constexpr perms TEST_VALUE_RHS = perms::set_uid;

    constexpr auto BASE_VALUE_LHS = static_cast<permsBaseType_t>(TEST_VALUE_LHS);
    constexpr auto BASE_VALUE_RHS = static_cast<permsBaseType_t>(TEST_VALUE_RHS);

    EXPECT_THAT(static_cast<permsBaseType_t>(TEST_VALUE_LHS ^ TEST_VALUE_RHS), Eq(BASE_VALUE_LHS ^ BASE_VALUE_RHS));
}

TEST(filesystem_test, permsSatisfiesBinaryComplementOperationCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "c313cf42-4cf0-4836-95ff-129111a707b0");
    constexpr perms TEST_VALUE = perms::owner_read;

    constexpr auto BASE_VALUE = static_cast<permsBaseType_t>(TEST_VALUE);

    EXPECT_THAT(static_cast<permsBaseType_t>(~TEST_VALUE), Eq(~BASE_VALUE));
}

TEST(filesystem_test, permsSatisfiesBinaryOrAssignmentOperationCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3611de8-f932-4485-9e64-6cd8af4526dc");
    constexpr perms TEST_VALUE = perms::sticky_bit;
    constexpr perms TEST_VALUE_RHS = perms::group_read;

    auto sutBaseValue = static_cast<permsBaseType_t>(TEST_VALUE);
    constexpr auto BASE_VALUE_RHS = static_cast<permsBaseType_t>(TEST_VALUE_RHS);

    perms sut = TEST_VALUE;

    EXPECT_THAT(static_cast<permsBaseType_t>(sut |= TEST_VALUE_RHS), Eq(sutBaseValue |= BASE_VALUE_RHS));
}

TEST(filesystem_test, permsSatisfiesBinaryAndAssignmentOperationCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "03c139be-e3ec-477e-8598-5da93699ab75");
    constexpr perms TEST_VALUE = perms::others_exec;
    constexpr perms TEST_VALUE_RHS = perms::others_all;

    auto sutBaseValue = static_cast<permsBaseType_t>(TEST_VALUE);
    constexpr auto BASE_VALUE_RHS = static_cast<permsBaseType_t>(TEST_VALUE_RHS);

    perms sut = TEST_VALUE;

    EXPECT_THAT(static_cast<permsBaseType_t>(sut &= TEST_VALUE_RHS), Eq(sutBaseValue &= BASE_VALUE_RHS));
}

TEST(filesystem_test, permsSatisfiesBinaryExclusiveOrAssignmentOperationCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "dae75205-a635-4535-8e8d-05541bb05b60");
    constexpr perms TEST_VALUE = perms::none;
    constexpr perms TEST_VALUE_RHS = perms::owner_all;

    auto sutBaseValue = static_cast<permsBaseType_t>(TEST_VALUE);
    constexpr auto BASE_VALUE_RHS = static_cast<permsBaseType_t>(TEST_VALUE_RHS);

    perms sut = TEST_VALUE;

    EXPECT_THAT(static_cast<permsBaseType_t>(sut ^= TEST_VALUE_RHS), Eq(sutBaseValue ^= BASE_VALUE_RHS));
}

TEST(filesystem_test, permsWhenEverythingIsSetTheOutputPrintsEverything)
{
    ::testing::Test::RecordProperty("TEST_ID", "2bb4931f-6ef9-4089-88a1-bf263a931559");
    Logger_Mock loggerMock;
    {
        auto logStream = iox::log::LogStream(loggerMock);
        logStream << perms::mask;
    }

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.m_logs[0].message,
                Eq("owner: {read, write, execute},  group: {read, write, execute},  others: {read, write, execute},  "
                   "special bits: {set_uid, set_git, sticky_bit}"));
}

TEST(filesystem_test, permsWhenNothingIsSetEveryEntryIsNone)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b50cb56-6dae-4514-bd77-791f81f6adca");
    Logger_Mock loggerMock;
    {
        auto logStream = iox::log::LogStream(loggerMock);
        logStream << perms::none;
    }

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.m_logs[0].message,
                Eq("owner: {none},  group: {none},  others: {none},  special bits: {none}"));
}

TEST(filesystem_test, permsWhenSomeOrSetTheOutputIsCorrect)
{
    ::testing::Test::RecordProperty("TEST_ID", "94e647b7-242b-4fe3-bccd-2fde9e091e8e");
    Logger_Mock loggerMock;
    {
        auto logStream = iox::log::LogStream(loggerMock);
        logStream << (perms::owner_write | perms::owner_exec | perms::group_read | perms::group_exec | perms::others_all
                      | perms::sticky_bit);
    }
    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.m_logs[0].message,
                Eq("owner: {write, execute},  group: {read, execute},  others: {read, write, execute},  special bits: "
                   "{sticky_bit}"));
}
} // namespace
