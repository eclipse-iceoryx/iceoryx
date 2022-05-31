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

#include "iceoryx_hoofs/platform/fcntl.hpp"
#include "iceoryx_hoofs/posix_wrapper/types.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::posix;

constexpr AccessMode INVALID_ACCESS_MODE = static_cast<AccessMode>(std::numeric_limits<uint64_t>::max());
constexpr OpenMode INVALID_OPEN_MODE = static_cast<OpenMode>(std::numeric_limits<uint64_t>::max());

TEST(TypesTest, ConvertToOflagFromAccessModeWorks)
{
    EXPECT_THAT(convertToOflags(AccessMode::READ_ONLY), Eq(O_RDONLY));
    EXPECT_THAT(convertToOflags(AccessMode::READ_WRITE), Eq(O_RDWR));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE), Eq(0U));
}

TEST(TypesTest, ConvertToOflagFromOpenModeWorks)
{
    EXPECT_THAT(convertToOflags(OpenMode::EXCLUSIVE_CREATE), Eq(O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(OpenMode::PURGE_AND_CREATE), Eq(O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(OpenMode::OPEN_OR_CREATE), Eq(O_CREAT));
    EXPECT_THAT(convertToOflags(OpenMode::OPEN_EXISTING), Eq(0));
    EXPECT_THAT(convertToOflags(INVALID_OPEN_MODE), Eq(0));
}

TEST(TypesTest, ConvertToOflagFromAccessAndOpenModeWorks)
{
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

    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::EXCLUSIVE_CREATE), Eq(O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::PURGE_AND_CREATE), Eq(O_CREAT | O_EXCL));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::OPEN_OR_CREATE), Eq(O_CREAT));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, OpenMode::OPEN_EXISTING), Eq(0));
    EXPECT_THAT(convertToOflags(INVALID_ACCESS_MODE, INVALID_OPEN_MODE), Eq(0));
}
} // namespace
