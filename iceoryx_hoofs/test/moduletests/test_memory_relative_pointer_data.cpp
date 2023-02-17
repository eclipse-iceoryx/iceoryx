// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iox/detail/relative_pointer_data.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;

TEST(RelativePointerData_test, DefaultConstructedResultsInNullptrIdAndOffset)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5903170-0166-4458-b481-4a8f55fabe38");
    RelativePointerData sut;

    EXPECT_EQ(sut.id(), RelativePointerData::NULL_POINTER_ID);
    EXPECT_EQ(sut.offset(), RelativePointerData::NULL_POINTER_OFFSET);
}

TEST(RelativePointerData_test, DefaultConstructedResultsInLogicallyNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "1de920b9-e082-40df-bbb2-0a12f4262cb8");
    RelativePointerData sut;

    EXPECT_TRUE(sut.isLogicalNullptr());
}

TEST(RelativePointerData_test, ResetOnDefaultConstructedResultsInLogicallyNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "b75183a1-fbb9-4693-b540-da66523fc7df");
    RelativePointerData sut;

    sut.reset();

    EXPECT_TRUE(sut.isLogicalNullptr());
}

TEST(RelativePointerData_test, ConstructedWithNonZeroIdAndOffsetResultsInSameIdAndOffset)
{
    ::testing::Test::RecordProperty("TEST_ID", "2aab553c-5348-4788-b0df-795d0b683487");
    constexpr uint16_t ID{13U};
    constexpr uint64_t OFFSET{42U};

    RelativePointerData sut{ID, OFFSET};

    EXPECT_EQ(sut.id(), ID);
    EXPECT_EQ(sut.offset(), OFFSET);
}

TEST(RelativePointerData_test, ConstructedWithZeroIdAndOffsetResultsNotInLogicallyNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "84bfc576-b31b-4f97-b862-4711e7257504");
    constexpr uint16_t ID{0U};
    constexpr uint64_t OFFSET{0U};

    RelativePointerData sut{ID, OFFSET};

    EXPECT_FALSE(sut.isLogicalNullptr());
}

TEST(RelativePointerData_test, ConstructedWithNonZeroIdAndOffsetResultsNotInLogicallyNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "84214163-11f8-4693-8827-bb4519c6b377");
    constexpr uint16_t ID{13U};
    constexpr uint64_t OFFSET{42U};

    RelativePointerData sut{ID, OFFSET};

    EXPECT_FALSE(sut.isLogicalNullptr());
}

TEST(RelativePointerData_test, ConstructedWithMaxIdAndOffsetResultsNotInLogicallyNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "28c3eb9f-aedb-4eab-a5e3-ac48b0ef50b6");
    constexpr uint16_t ID{RelativePointerData::MAX_VALID_ID};
    constexpr uint64_t OFFSET{RelativePointerData::MAX_VALID_OFFSET};

    RelativePointerData sut{ID, OFFSET};

    EXPECT_FALSE(sut.isLogicalNullptr());
}

TEST(RelativePointerData_test, ResetOnConstructedWithNonZeroIdAndOffsetResultsInLogicallyNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "7917eb83-2671-4091-a0b1-bd86e9fb9268");
    constexpr uint16_t ID{13U};
    constexpr uint64_t OFFSET{42U};

    RelativePointerData sut{ID, OFFSET};
    sut.reset();

    EXPECT_TRUE(sut.isLogicalNullptr());
}

TEST(RelativePointerData_test, ConstructedWithNullPointerIdResultsLogicallyNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "75a669ee-65ca-41f0-8196-f9dd4260a641");
    constexpr uint16_t ID{RelativePointerData::NULL_POINTER_ID};
    constexpr uint64_t OFFSET{RelativePointerData::MAX_VALID_OFFSET};

    RelativePointerData sut(ID, OFFSET);

    EXPECT_TRUE(sut.isLogicalNullptr());
}

TEST(RelativePointerData_test, ConstructedWithNullPointerOffsetResultsInLogicallyNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3206dfa-fcc3-4f39-9590-daea8a5482f9");
    constexpr uint16_t ID{RelativePointerData::MAX_VALID_ID};
    constexpr uint64_t OFFSET{RelativePointerData::NULL_POINTER_OFFSET};

    RelativePointerData sut(ID, OFFSET);

    EXPECT_TRUE(sut.isLogicalNullptr());
}

TEST(RelativePointerData_test, ConstructedWithNullPointerIdAndOffsetResultsInLogicallyNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "c6dd55da-a70d-409d-b88e-1835db44a163");
    constexpr uint16_t ID{RelativePointerData::NULL_POINTER_ID};
    constexpr uint64_t OFFSET{RelativePointerData::NULL_POINTER_OFFSET};

    RelativePointerData sut(ID, OFFSET);

    EXPECT_TRUE(sut.isLogicalNullptr());
}

} // namespace
