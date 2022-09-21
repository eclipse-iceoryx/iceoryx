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

using iox::testing::Logger_Mock;

using base_t = std::underlying_type<perms>::type;

constexpr base_t toBase(const perms permission) noexcept
{
    return static_cast<base_t>(permission);
}

TEST(filesystem_test, permsBinaryOrEqualToBinaryOrOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b72fcec-c2b3-4a45-801f-542ff3195a2f");
    constexpr perms TEST_VALUE_LHS = perms::others_write;
    constexpr perms TEST_VALUE_RHS = perms::group_all;

    constexpr auto BASE_VALUE_LHS = toBase(TEST_VALUE_LHS);
    constexpr auto BASE_VALUE_RHS = toBase(TEST_VALUE_RHS);

    EXPECT_THAT(toBase(TEST_VALUE_LHS | TEST_VALUE_RHS), Eq(BASE_VALUE_LHS | BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryAndEqualToBinaryAndOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "15a02845-21b0-41fb-80bf-ee2ff9a81427");
    constexpr perms TEST_VALUE_LHS = perms::others_read;
    constexpr perms TEST_VALUE_RHS = perms::mask;

    constexpr auto BASE_VALUE_LHS = toBase(TEST_VALUE_LHS);
    constexpr auto BASE_VALUE_RHS = toBase(TEST_VALUE_RHS);

    EXPECT_THAT(toBase(TEST_VALUE_LHS & TEST_VALUE_RHS), Eq(BASE_VALUE_LHS & BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryExclusiveOrEqualToBinaryExclusiveOrOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "8094a263-2861-45ad-aecd-9312d477bc2d");
    constexpr perms TEST_VALUE_LHS = perms::set_gid;
    constexpr perms TEST_VALUE_RHS = perms::set_uid;

    constexpr auto BASE_VALUE_LHS = toBase(TEST_VALUE_LHS);
    constexpr auto BASE_VALUE_RHS = toBase(TEST_VALUE_RHS);

    EXPECT_THAT(toBase(TEST_VALUE_LHS ^ TEST_VALUE_RHS), Eq(BASE_VALUE_LHS ^ BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryComplementEqualToBinaryComplementOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "c313cf42-4cf0-4836-95ff-129111a707b0");
    constexpr perms TEST_VALUE = perms::owner_read;

    constexpr auto BASE_VALUE = toBase(TEST_VALUE);

    EXPECT_THAT(toBase(~TEST_VALUE), Eq(~BASE_VALUE));
}

TEST(filesystem_test, permsBinaryOrAssignmentEqualToBinaryOrAssignmentOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3611de8-f932-4485-9e64-6cd8af4526dc");
    constexpr perms TEST_VALUE = perms::sticky_bit;
    constexpr perms TEST_VALUE_RHS = perms::group_read;

    auto sutBaseValue = toBase(TEST_VALUE);
    constexpr auto BASE_VALUE_RHS = toBase(TEST_VALUE_RHS);

    perms sut = TEST_VALUE;

    EXPECT_THAT(toBase(sut |= TEST_VALUE_RHS), Eq(sutBaseValue |= BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryAndAssignmentEqualToBinaryAndAssignmentOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "03c139be-e3ec-477e-8598-5da93699ab75");
    constexpr perms TEST_VALUE = perms::others_exec;
    constexpr perms TEST_VALUE_RHS = perms::others_all;

    auto sutBaseValue = toBase(TEST_VALUE);
    constexpr auto BASE_VALUE_RHS = toBase(TEST_VALUE_RHS);

    perms sut = TEST_VALUE;

    EXPECT_THAT(toBase(sut &= TEST_VALUE_RHS), Eq(sutBaseValue &= BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryExclusiveOrAssignmentEqualToBinaryExclusiveOrAssignmentOfUnderylingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "dae75205-a635-4535-8e8d-05541bb05b60");
    constexpr perms TEST_VALUE = perms::none;
    constexpr perms TEST_VALUE_RHS = perms::owner_all;

    auto sutBaseValue = toBase(TEST_VALUE);
    constexpr auto BASE_VALUE_RHS = toBase(TEST_VALUE_RHS);

    perms sut = TEST_VALUE;

    EXPECT_THAT(toBase(sut ^= TEST_VALUE_RHS), Eq(sutBaseValue ^= BASE_VALUE_RHS));
}

TEST(filesystem_test, streamOperatorPrintsCorrectlyWhenEverythingIsSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "2bb4931f-6ef9-4089-88a1-bf263a931559");
    Logger_Mock loggerMock;
    {
        IOX_LOGSTREAM_MOCK(loggerMock) << perms::mask;
    }

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.m_logs[0].message,
                Eq("owner: {read, write, execute},  group: {read, write, execute},  others: {read, write, execute},  "
                   "special bits: {set_uid, set_git, sticky_bit}"));
}

TEST(filesystem_test, streamOperatorPrintsCorrectlyWhenNothingIsSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b50cb56-6dae-4514-bd77-791f81f6adca");
    Logger_Mock loggerMock;
    {
        IOX_LOGSTREAM_MOCK(loggerMock) << perms::none;
    }

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.m_logs[0].message,
                Eq("owner: {none},  group: {none},  others: {none},  special bits: {none}"));
}

TEST(filesystem_test, streamOperatorPrintsCorrectlyWhenPartialPermissionsAreSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "94e647b7-242b-4fe3-bccd-2fde9e091e8e");
    Logger_Mock loggerMock;
    {
        IOX_LOGSTREAM_MOCK(loggerMock) << (perms::owner_write | perms::owner_exec | perms::group_read
                                           | perms::group_exec | perms::others_all | perms::sticky_bit);
    }
    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.m_logs[0].message,
                Eq("owner: {write, execute},  group: {read, execute},  others: {read, write, execute},  special bits: "
                   "{sticky_bit}"));
}

TEST(filesystem_test, streamOperatorPrintsCorrectlyWhenSetToUnknown)
{
    ::testing::Test::RecordProperty("TEST_ID", "bcfd29e1-84d9-11ec-9e17-5405db3a3777");
    Logger_Mock loggerMock;
    {
        IOX_LOGSTREAM_MOCK(loggerMock) << perms::unknown;
    }

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.m_logs[0].message, Eq("unknown permissions"));
}

} // namespace
