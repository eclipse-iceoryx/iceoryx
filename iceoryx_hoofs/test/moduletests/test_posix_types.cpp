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

#include "iceoryx_hoofs/posix_wrapper/types.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/mman.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::posix;

constexpr AccessMode INVALID_ACCESS_MODE = static_cast<AccessMode>(std::numeric_limits<uint64_t>::max());
constexpr OpenMode INVALID_OPEN_MODE = static_cast<OpenMode>(std::numeric_limits<uint64_t>::max());

TEST(TypesTest, ConvertToOflagFromAccessModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9eb74e8c-7498-4400-9248-92aa6bd15142");
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY), Eq(O_RDONLY));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE), Eq(O_RDWR));
    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY), Eq(O_WRONLY));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE), Eq(0U));
}

TEST(TypesTest, ConvertToProtflagFromAccessModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7a5c699e-16e6-471f-80b6-a325644e60d3");
    EXPECT_THAT(convertToProtFlags(AccessMode::READ_ONLY), Eq(PROT_READ));
    EXPECT_THAT(convertToProtFlags(AccessMode::READ_WRITE), Eq(PROT_READ | PROT_WRITE));
    EXPECT_THAT(convertToProtFlags(AccessMode::WRITE_ONLY), Eq(PROT_WRITE));
    EXPECT_THAT(convertToProtFlags(INVALID_ACCESS_MODE), Eq(PROT_NONE));
}

TEST(TypesTest, ConvertToOflagFromOpenModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "95fa55c9-2d64-4296-8bbb-41ff3c9dac3f");
    // used for test purposes; operands have positive values and result is within integer range
    // NOLINTBEGIN(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(OpenMode::EXCLUSIVE_CREATE), Eq(O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(OpenMode::PURGE_AND_CREATE), Eq(O_CREAT | O_EXCL));
    // NOLINTEND(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(OpenMode::OPEN_OR_CREATE), Eq(O_CREAT));
    EXPECT_THAT(convertToOflags(OpenMode::OPEN_EXISTING), Eq(0));
    EXPECT_THAT(convertToOflags(INVALID_OPEN_MODE), Eq(0));
}

TEST(TypesTest, ConvertToOflagFromAccessAndOpenModeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4ea6823c-2ecd-48a5-bcea-0ea0585bee72");
    // used for test purposes; operands have positive values and result is within integer range
    // NOLINTBEGIN(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY, OpenMode::EXCLUSIVE_CREATE), Eq(O_RDONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY, OpenMode::PURGE_AND_CREATE), Eq(O_RDONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY, OpenMode::OPEN_OR_CREATE), Eq(O_RDONLY | O_CREAT));
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY, OpenMode::OPEN_EXISTING), Eq(O_RDONLY));
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY, INVALID_OPEN_MODE), Eq(O_RDONLY));

    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE, OpenMode::EXCLUSIVE_CREATE), Eq(O_RDWR | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE, OpenMode::PURGE_AND_CREATE), Eq(O_RDWR | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE, OpenMode::OPEN_OR_CREATE), Eq(O_RDWR | O_CREAT));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE, OpenMode::OPEN_EXISTING), Eq(O_RDWR));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE, INVALID_OPEN_MODE), Eq(O_RDWR));

    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY, OpenMode::EXCLUSIVE_CREATE), Eq(O_WRONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY, OpenMode::PURGE_AND_CREATE), Eq(O_WRONLY | O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY, OpenMode::OPEN_OR_CREATE), Eq(O_WRONLY | O_CREAT));
    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY, OpenMode::OPEN_EXISTING), Eq(O_WRONLY));
    EXPECT_THAT(convertToOflags(AccessMode::WRITE_ONLY, INVALID_OPEN_MODE), Eq(O_WRONLY));

    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::EXCLUSIVE_CREATE), Eq(O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::PURGE_AND_CREATE), Eq(O_CREAT | O_EXCL));
    // NOLINTEND(hicpp-signed-bitwise)
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::OPEN_OR_CREATE), Eq(O_CREAT));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::OPEN_EXISTING), Eq(0));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, INVALID_OPEN_MODE), Eq(0));
}

TEST(TypesTest, OpenModeAsStringLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "830756de-b3c9-4285-b42a-e0c6c5a315a9");
    EXPECT_THAT(asStringLiteral(OpenMode::EXCLUSIVE_CREATE), StrEq("OpenMode::EXCLUSIVE_CREATE"));
    EXPECT_THAT(asStringLiteral(OpenMode::PURGE_AND_CREATE), StrEq("OpenMode::PURGE_AND_CREATE"));
    EXPECT_THAT(asStringLiteral(OpenMode::OPEN_OR_CREATE), StrEq("OpenMode::OPEN_OR_CREATE"));
    EXPECT_THAT(asStringLiteral(OpenMode::OPEN_EXISTING), StrEq("OpenMode::OPEN_EXISTING"));
    EXPECT_THAT(asStringLiteral(INVALID_OPEN_MODE), StrEq("OpenMode::UNDEFINED_VALUE"));
}

TEST(TypesTest, AccessModeAsStringLiteral)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5a09ee7-df2c-4a28-929c-7de743f1e423");
    EXPECT_THAT(asStringLiteral(AccessMode::READ_ONLY), StrEq("AccessMode::READ_ONLY"));
    EXPECT_THAT(asStringLiteral(AccessMode::READ_WRITE), StrEq("AccessMode::READ_WRITE"));
    EXPECT_THAT(asStringLiteral(AccessMode::WRITE_ONLY), StrEq("AccessMode::WRITE_ONLY"));
    EXPECT_THAT(asStringLiteral(INVALID_ACCESS_MODE), StrEq("AccessMode::UNDEFINED_VALUE"));
}
} // namespace
