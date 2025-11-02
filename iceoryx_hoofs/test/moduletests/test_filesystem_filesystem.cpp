// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex AI Inc. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/mman.hpp"
#include "iox/filesystem.hpp"
#include "test.hpp"

#include <array>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::internal;

using iox::testing::Logger_Mock;

// BEGIN AccessMode and OpenMode tests

constexpr AccessMode INVALID_ACCESS_MODE =
    static_cast<AccessMode>(std::numeric_limits<std::underlying_type_t<AccessMode>>::max());
constexpr OpenMode INVALID_OPEN_MODE =
    static_cast<OpenMode>(std::numeric_limits<std::underlying_type_t<OpenMode>>::max());

TEST(TypesTest, ConvertToOflagFromAccessModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9eb74e8c-7498-4400-9248-92aa6bd15142");
    EXPECT_THAT(convertToOflags(AccessMode::ReadOnly), Eq(O_RDONLY));
    EXPECT_THAT(convertToOflags(AccessMode::ReadWrite), Eq(O_RDWR));
    EXPECT_THAT(convertToOflags(AccessMode::WriteOnly), Eq(O_WRONLY));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE), Eq(0U));
}

TEST(TypesTest, ConvertToProtflagFromAccessModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7a5c699e-16e6-471f-80b6-a325644e60d3");
    EXPECT_THAT(convertToProtFlags(AccessMode::ReadOnly), Eq(PROT_READ));
    EXPECT_THAT(convertToProtFlags(AccessMode::ReadWrite), Eq(PROT_READ | PROT_WRITE));
    EXPECT_THAT(convertToProtFlags(AccessMode::WriteOnly), Eq(PROT_WRITE));
    EXPECT_THAT(convertToProtFlags(INVALID_ACCESS_MODE), Eq(PROT_NONE));
}

TEST(TypesTest, ConvertToOflagFromOpenModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "95fa55c9-2d64-4296-8bbb-41ff3c9dac3f");
    // used for test purposes; operands have positive values and result is within integer range
    // NOLINTBEGIN(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(OpenMode::ExclusiveCreate), Eq(O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(OpenMode::PurgeAndCreate), Eq(O_CREAT | O_EXCL));
    // NOLINTEND(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(OpenMode::OpenOrCreate), Eq(O_CREAT));
    EXPECT_THAT(convertToOflags(OpenMode::OpenExisting), Eq(0));
    EXPECT_THAT(convertToOflags(INVALID_OPEN_MODE), Eq(0));
}

TEST(TypesTest, ConvertToOflagFromAccessAndOpenModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ea6823c-2ecd-48a5-bcea-0ea0585bee72");
    // used for test purposes; operands have positive values and result is within integer range
    // NOLINTBEGIN(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(AccessMode::ReadOnly, OpenMode::ExclusiveCreate), Eq(O_RDONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::ReadOnly, OpenMode::PurgeAndCreate), Eq(O_RDONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::ReadOnly, OpenMode::OpenOrCreate), Eq(O_RDONLY | O_CREAT));
    EXPECT_THAT(convertToOflags(AccessMode::ReadOnly, OpenMode::OpenExisting), Eq(O_RDONLY));
    EXPECT_THAT(convertToOflags(AccessMode::ReadOnly, INVALID_OPEN_MODE), Eq(O_RDONLY));

    EXPECT_THAT(convertToOflags(AccessMode::ReadWrite, OpenMode::ExclusiveCreate), Eq(O_RDWR | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::ReadWrite, OpenMode::PurgeAndCreate), Eq(O_RDWR | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::ReadWrite, OpenMode::OpenOrCreate), Eq(O_RDWR | O_CREAT));
    EXPECT_THAT(convertToOflags(AccessMode::ReadWrite, OpenMode::OpenExisting), Eq(O_RDWR));
    EXPECT_THAT(convertToOflags(AccessMode::ReadWrite, INVALID_OPEN_MODE), Eq(O_RDWR));

    EXPECT_THAT(convertToOflags(AccessMode::WriteOnly, OpenMode::ExclusiveCreate), Eq(O_WRONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::WriteOnly, OpenMode::PurgeAndCreate), Eq(O_WRONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::WriteOnly, OpenMode::OpenOrCreate), Eq(O_WRONLY | O_CREAT));
    EXPECT_THAT(convertToOflags(AccessMode::WriteOnly, OpenMode::OpenExisting), Eq(O_WRONLY));
    EXPECT_THAT(convertToOflags(AccessMode::WriteOnly, INVALID_OPEN_MODE), Eq(O_WRONLY));

    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::ExclusiveCreate), Eq(O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::PurgeAndCreate), Eq(O_CREAT | O_EXCL));
    // NOLINTEND(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::OpenOrCreate), Eq(O_CREAT));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::OpenExisting), Eq(0));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, INVALID_OPEN_MODE), Eq(0));
}

TEST(TypesTest, OpenModeAsStringLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "830756de-b3c9-4285-b42a-e0c6c5a315a9");
    EXPECT_THAT(asStringLiteral(OpenMode::ExclusiveCreate), StrEq("OpenMode::ExclusiveCreate"));
    EXPECT_THAT(asStringLiteral(OpenMode::PurgeAndCreate), StrEq("OpenMode::PurgeAndCreate"));
    EXPECT_THAT(asStringLiteral(OpenMode::OpenOrCreate), StrEq("OpenMode::OpenOrCreate"));
    EXPECT_THAT(asStringLiteral(OpenMode::OpenExisting), StrEq("OpenMode::OpenExisting"));
    EXPECT_THAT(asStringLiteral(INVALID_OPEN_MODE), StrEq("OpenMode::UndefinedValue"));
}

TEST(TypesTest, AccessModeAsStringLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5a09ee7-df2c-4a28-929c-7de743f1e423");
    EXPECT_THAT(asStringLiteral(AccessMode::ReadOnly), StrEq("AccessMode::ReadOnly"));
    EXPECT_THAT(asStringLiteral(AccessMode::ReadWrite), StrEq("AccessMode::ReadWrite"));
    EXPECT_THAT(asStringLiteral(AccessMode::WriteOnly), StrEq("AccessMode::WriteOnly"));
    EXPECT_THAT(asStringLiteral(INVALID_ACCESS_MODE), StrEq("AccessMode::UndefinedValue"));
}

// END AccessMode and OpenMode tests


// BEGIN access_rights tests

TEST(filesystem_test, accessRightsFromValueSanitizedWorksForValueInRangeOfPermsMask)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a6c2ece-9cd7-4779-ad0b-16372aebd407");
    constexpr auto TEST_VALUE = access_rights::detail::OWNER_READ;

    EXPECT_THAT(access_rights::from_value_sanitized(TEST_VALUE).value(), Eq(TEST_VALUE));
}

TEST(filesystem_test, accessRightsFromValueSanitizedWorksForValueOutOfRangeOfPermsMask)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a291709-a75c-4afa-96d8-d57a0d40696c");
    constexpr auto TEST_VALUE_SANITIZED = access_rights::detail::OWNER_WRITE;
    constexpr auto TEST_VALUE = TEST_VALUE_SANITIZED | static_cast<access_rights::value_type>(010000);

    EXPECT_THAT(access_rights::from_value_sanitized(TEST_VALUE).value(), Eq(TEST_VALUE_SANITIZED));
}

TEST(filesystem_test, permsBinaryOrEqualToBinaryOrOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b72fcec-c2b3-4a45-801f-542ff3195a2f");
    constexpr access_rights TEST_VALUE_LHS = perms::others_write;
    constexpr access_rights TEST_VALUE_RHS = perms::group_all;

    constexpr auto BASE_VALUE_LHS = iox::access_rights::detail::OTHERS_WRITE;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::GROUP_ALL;

    EXPECT_THAT((TEST_VALUE_LHS | TEST_VALUE_RHS).value(), Eq(BASE_VALUE_LHS | BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryAndEqualToBinaryAndOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "15a02845-21b0-41fb-80bf-ee2ff9a81427");
    constexpr access_rights TEST_VALUE_LHS = perms::others_read;
    constexpr access_rights TEST_VALUE_RHS = perms::mask;

    constexpr auto BASE_VALUE_LHS = iox::access_rights::detail::OTHERS_READ;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::MASK;

    EXPECT_THAT((TEST_VALUE_LHS & TEST_VALUE_RHS).value(), Eq(BASE_VALUE_LHS & BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryExclusiveOrEqualToBinaryExclusiveOrOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "8094a263-2861-45ad-aecd-9312d477bc2d");
    constexpr access_rights TEST_VALUE_LHS = perms::set_gid;
    constexpr access_rights TEST_VALUE_RHS = perms::set_uid;

    constexpr auto BASE_VALUE_LHS = iox::access_rights::detail::SET_GID;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::SET_UID;

    EXPECT_THAT((TEST_VALUE_LHS ^ TEST_VALUE_RHS).value(), Eq(BASE_VALUE_LHS ^ BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryComplementEqualToBinaryComplementOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "c313cf42-4cf0-4836-95ff-129111a707b0");
    constexpr access_rights TEST_VALUE = perms::owner_read;

    constexpr auto BASE_VALUE = iox::access_rights::detail::OWNER_READ;
    constexpr auto EXPECTED_VALUE = static_cast<access_rights::value_type>(~BASE_VALUE);

    ASSERT_THAT(TEST_VALUE.value(), Eq(BASE_VALUE));

    EXPECT_THAT((~TEST_VALUE).value(), Eq(EXPECTED_VALUE));
}

TEST(filesystem_test, permsBinaryOrAssignmentEqualToBinaryOrAssignmentOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3611de8-f932-4485-9e64-6cd8af4526dc");
    constexpr access_rights TEST_VALUE = perms::sticky_bit;
    constexpr access_rights TEST_VALUE_RHS = perms::group_read;

    auto sutBaseValue = iox::access_rights::detail::STICKY_BIT;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::GROUP_READ;

    access_rights sut = TEST_VALUE;

    EXPECT_THAT((sut |= TEST_VALUE_RHS).value(), Eq(sutBaseValue |= BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryAndAssignmentEqualToBinaryAndAssignmentOfUnderlyingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "03c139be-e3ec-477e-8598-5da93699ab75");
    constexpr access_rights TEST_VALUE = perms::others_exec;
    constexpr access_rights TEST_VALUE_RHS = perms::others_all;

    auto sutBaseValue = iox::access_rights::detail::OTHERS_EXEC;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::OTHERS_ALL;

    access_rights sut = TEST_VALUE;

    EXPECT_THAT((sut &= TEST_VALUE_RHS).value(), Eq(sutBaseValue &= BASE_VALUE_RHS));
}

TEST(filesystem_test, permsBinaryExclusiveOrAssignmentEqualToBinaryExclusiveOrAssignmentOfUnderylingType)
{
    ::testing::Test::RecordProperty("TEST_ID", "dae75205-a635-4535-8e8d-05541bb05b60");
    constexpr access_rights TEST_VALUE = perms::none;
    constexpr access_rights TEST_VALUE_RHS = perms::owner_all;

    auto sutBaseValue = iox::access_rights::detail::NONE;
    constexpr auto BASE_VALUE_RHS = iox::access_rights::detail::OWNER_ALL;

    access_rights sut = TEST_VALUE;

    EXPECT_THAT((sut ^= TEST_VALUE_RHS).value(), Eq(sutBaseValue ^= BASE_VALUE_RHS));
}

TEST(filesystem_test, streamOperatorPrintsCorrectlyWhenEverythingIsSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "2bb4931f-6ef9-4089-88a1-bf263a931559");
    Logger_Mock loggerMock;
    {
        IOX_LOGSTREAM_MOCK(loggerMock) << perms::mask;
    }

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message,
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

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message,
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
    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message,
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

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, Eq("unknown permissions"));
}

// END access_rights tests

} // namespace
