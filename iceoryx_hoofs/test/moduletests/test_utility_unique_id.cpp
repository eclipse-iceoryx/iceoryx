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

#include "iox/attributes.hpp"
#include "iox/detail/unique_id.hpp"

#include "test.hpp"

#include <algorithm>
#include <vector>

namespace
{
using namespace ::testing;
using namespace iox;

TEST(UniqueId_test, ConstructingUniqueIdWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7fb13d3-5c3f-4253-b485-482820aded15");
    auto sut [[maybe_unused]] = UniqueId();
}

TEST(UniqueId_test, ConversionToValueTypeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5f68f612-08ef-4994-b954-1af1d1fb151d");
    auto sut = UniqueId();
    auto value [[maybe_unused]] = static_cast<UniqueId::value_type>(sut);
}

TEST(UniqueId_test, TwoConsecutiveCreatedUniqueIdsDifferByOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b01170e-dffa-4ea8-a7ac-f8bc12194b2b");
    auto sut1 = UniqueId();
    auto sut2 = UniqueId();
    auto value1 = static_cast<UniqueId::value_type>(sut1);
    auto value2 = static_cast<UniqueId::value_type>(sut2);

    EXPECT_THAT(value2 - value1, Eq(1U));
}

TEST(UniqueId_test, ComparingTwoUniqueIdsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f73f96c2-7e86-4e58-b246-f5eb9938a09c");
    auto sut1 = UniqueId();
    auto sut2 = UniqueId();

    EXPECT_TRUE(sut1 < sut2);
    EXPECT_TRUE(sut1 <= sut2);
    EXPECT_TRUE(sut1 != sut2);
    EXPECT_TRUE(sut2 > sut1);
    EXPECT_TRUE(sut2 >= sut1);
    EXPECT_FALSE(sut1 == sut2);
}

TEST(UniqueId_test, CopyConstructingUniqueIdsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e498c41f-2f15-4c6a-a2a5-57d6c7be1412");
    auto id = UniqueId();
    auto idValue = static_cast<UniqueId::value_type>(id);

    // NOLINTJUSTIFICATION we test the copy constructor here
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    auto sut{id};
    auto sutValue = static_cast<UniqueId::value_type>(sut);

    EXPECT_THAT(sut, Eq(id));
    EXPECT_THAT(sutValue, Eq(idValue));
}

TEST(UniqueId_test, CopyAssigningUniqueIdsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "79090a19-466b-4b70-8694-e58cdf4419f7");
    auto id = UniqueId();
    auto idValue = static_cast<UniqueId::value_type>(id);
    auto sut = UniqueId();
    sut = id;
    auto sutValue = static_cast<UniqueId::value_type>(sut);

    EXPECT_THAT(sut, Eq(id));
    EXPECT_THAT(sutValue, Eq(idValue));
}

TEST(UniqueId_test, MoveConstructingUniqueIdsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "da614907-acf5-4a26-a432-fa072ac6599d");
    auto id = UniqueId();
    auto idValue = static_cast<UniqueId::value_type>(id);
    auto sut{std::move(id)};
    auto sutValue = static_cast<UniqueId::value_type>(sut);

    EXPECT_THAT(sutValue, Eq(idValue));
}

TEST(UniqueId_test, MoveAssigningUniqueIdsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8c483497-6b20-40bb-bde0-f3900e1e1d91");
    auto id = UniqueId();
    auto idValue = static_cast<UniqueId::value_type>(id);
    auto sut = UniqueId();
    sut = std::move(id);
    auto sutValue = static_cast<UniqueId::value_type>(sut);

    EXPECT_THAT(sutValue, Eq(idValue));
}

TEST(UniqueId_test, UniqueIdsAreMonotonicallyIncreasing)
{
    ::testing::Test::RecordProperty("TEST_ID", "010c14cf-9af7-4a07-ac0a-2da9c7f6adf9");
    auto id1 = UniqueId();
    auto idValue1 = static_cast<UniqueId::value_type>(id1);

    auto idValue2 = [] { return static_cast<UniqueId::value_type>(UniqueId()); }();

    auto id3 = UniqueId();
    auto idValue3 = static_cast<UniqueId::value_type>(id3);

    EXPECT_THAT(idValue2 - idValue1, Eq(1U));
    EXPECT_THAT(idValue3 - idValue2, Eq(1U));
}

TEST(UniqueId_test, SortingUniqueIdsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "34b72dee-6b32-465b-b183-58a3c0f466a0");
    auto id1 = UniqueId();
    auto id2 = UniqueId();
    auto id3 = UniqueId();

    std::vector<UniqueId> sut;
    sut.push_back(id2);
    sut.push_back(id3);
    sut.push_back(id1);

    sort(sut.begin(), sut.end());

    ASSERT_THAT(sut.size(), Eq(3U));
    EXPECT_THAT(sut[0], Eq(id1));
    EXPECT_THAT(sut[1], Eq(id2));
    EXPECT_THAT(sut[2], Eq(id3));
}
} // namespace
