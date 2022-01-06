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

#include "iceoryx_hoofs/internal/cxx/unique_id.hpp"

#include "iceoryx_hoofs/cxx/attributes.hpp"

#include "test.hpp"

#include <algorithm>
#include <vector>

namespace
{
using namespace ::testing;
using namespace iox::cxx;

TEST(UniqueId_test, ConstructingUniqueIdWorks)
{
    auto sut IOX_MAYBE_UNUSED = UniqueId();
}

TEST(UniqueId_test, ConversionToValueTypeWorks)
{
    auto sut = UniqueId();
    auto value IOX_MAYBE_UNUSED = static_cast<UniqueId::value_type>(sut);
}

TEST(UniqueId_test, TwoConsecutiveCreatedUniqueIdsDifferByOne)
{
    auto sut1 = UniqueId();
    auto sut2 = UniqueId();
    auto value1 = static_cast<UniqueId::value_type>(sut1);
    auto value2 = static_cast<UniqueId::value_type>(sut2);

    EXPECT_THAT(value2 - value1, Eq(1U));
}

TEST(UniqueId_test, ComparingTwoUniqueIdsWorks)
{
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
    auto id = UniqueId();
    auto idValue = static_cast<UniqueId::value_type>(id);
    auto sut{id};
    auto sutValue = static_cast<UniqueId::value_type>(sut);

    EXPECT_THAT(sut, Eq(id));
    EXPECT_THAT(sutValue, Eq(idValue));
}

TEST(UniqueId_test, CopyAssigningUniqueIdsWorks)
{
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
    auto id = UniqueId();
    auto idValue = static_cast<UniqueId::value_type>(id);
    auto sut{std::move(id)};
    auto sutValue = static_cast<UniqueId::value_type>(sut);

    EXPECT_THAT(sutValue, Eq(idValue));
}

TEST(UniqueId_test, MoveAssigningUniqueIdsWorks)
{
    auto id = UniqueId();
    auto idValue = static_cast<UniqueId::value_type>(id);
    auto sut = UniqueId();
    sut = std::move(id);
    auto sutValue = static_cast<UniqueId::value_type>(sut);

    EXPECT_THAT(sutValue, Eq(idValue));
}

TEST(UniqueId_test, UniqueIdsAreMonotonicallyIncreasing)
{
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
