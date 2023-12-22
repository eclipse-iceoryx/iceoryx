// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "test.hpp"

#include "iox/detail/mpmc_lockfree_queue/cyclic_index.hpp"
namespace
{
using namespace ::testing;

using iox::concurrent::CyclicIndex;

template <typename T>
class MpmcLockFreeQueueCyclicIndexTest : public ::testing::Test
{
  public:
    using Index = T;
};

typedef ::testing::Types<CyclicIndex<1>, CyclicIndex<2>, CyclicIndex<10>, CyclicIndex<1000>> TestIndices;

TYPED_TEST_SUITE(MpmcLockFreeQueueCyclicIndexTest, TestIndices, );

// note that in all tests we will check whether the getCycle and getIndex methods
// behave as expected after certain operations (mainly addition),
// ensuring a proper cyclic behavior (modulo CycleLength)
// overflow cases are tested as well


TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, defaultConstructedIndexIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "86401c7a-73bc-4677-9f1a-1d08c38d4793");
    using Index = typename TestFixture::Index;
    Index index;

    EXPECT_EQ(index.getIndex(), 0);
    EXPECT_EQ(index.getCycle(), 0);
    EXPECT_EQ(index.getValue(), 0);
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, explicitIndexConstructionWithZeroWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c015eb36-cbd8-42e2-8083-2e446f4c0f50");
    using Index = typename TestFixture::Index;

    {
        Index index(0, 0);
        EXPECT_EQ(index.getIndex(), 0);
        EXPECT_EQ(index.getCycle(), 0);
        EXPECT_EQ(index.getValue(), 0);
    }

    {
        Index index(0);
        EXPECT_EQ(index.getIndex(), 0);
        EXPECT_EQ(index.getCycle(), 0);
        EXPECT_EQ(index.getValue(), 0);
    }
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, getValueReturnsValueIndexWasConstructedWith)
{
    ::testing::Test::RecordProperty("TEST_ID", "707d066c-f81c-4086-a71a-5b6b5333ecb5");
    using Index = typename TestFixture::Index;
    Index index(73);
    EXPECT_EQ(index.getValue(), 73);
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, explicitConstructionWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e72a059-b297-4e04-9dea-39218595ecac");
    using Index = typename TestFixture::Index;

    // something inbetween max and min
    const auto v = Index::MAX_VALUE / 2;
    const auto m = Index::MAX_INDEX + 1;
    const auto i = v % m;
    const auto c = v / m;

    {
        Index index(i, c);
        EXPECT_EQ(index.getIndex(), i);
        EXPECT_EQ(index.getCycle(), c);
    }

    // check that cycle and index are consistent if constructed by the value v itself
    {
        Index index(v);
        EXPECT_EQ(index.getIndex(), i);
        EXPECT_EQ(index.getCycle(), c);
    }
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, explicitConstructionWithMaxIndexAndCycleWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d15fba8a-c3c0-4820-a087-699feba1064d");
    using Index = typename TestFixture::Index;
    const auto i = Index::INDEX_AT_MAX_VALUE;
    const auto c = Index::MAX_CYCLE;
    const auto v = Index::MAX_VALUE;

    {
        Index index(i, c);
        EXPECT_EQ(index.getIndex(), i);
        EXPECT_EQ(index.getCycle(), c);
    }

    // check that cycle and index are consistent if constructed by the value v itself
    {
        Index index(v);
        EXPECT_EQ(index.getIndex(), i);
        EXPECT_EQ(index.getCycle(), c);
    }
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, copyConstructorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "896617da-a3b7-4667-ae56-a9f5229f548e");
    using Index = typename TestFixture::Index;

    // something inbetween max and min
    const auto i = Index::MAX_INDEX / 2;
    const auto c = Index::MAX_CYCLE / 2;

    Index index(i, c);
    EXPECT_EQ(index.getIndex(), i);
    EXPECT_EQ(index.getCycle(), c);

    Index indexCopy(index);
    EXPECT_EQ(indexCopy.getIndex(), i);
    EXPECT_EQ(indexCopy.getCycle(), c);
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, assignmentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb4ee1ac-5913-481e-aba4-18ef82ad1c2e");
    using Index = typename TestFixture::Index;

    const auto i = Index::MAX_INDEX / 2;
    const auto c = Index::MAX_CYCLE / 2;

    Index index1(i, c);
    Index index2;

    index2 = index1;
    EXPECT_EQ(index2.getIndex(), i);
    EXPECT_EQ(index2.getCycle(), c);
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, selfAssignmentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "56e126b1-43d8-43e7-b483-4a0fa8ac620b");
    using Index = typename TestFixture::Index;

    const auto i = Index::MAX_INDEX / 2;
    const auto c = Index::MAX_CYCLE / 2;

    Index index(i, c);
    // this construct is used to prevent a self-assign warning
    [](Index& a, Index& b) { a = b; }(index, index);

    EXPECT_EQ(index.getIndex(), i);
    EXPECT_EQ(index.getCycle(), c);
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, cyclicAdditionWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e8d5f97f-cb11-47d8-9525-5d41c849d9d3");
    using Index = typename TestFixture::Index;

    const auto i = Index::MAX_INDEX / 2;
    const auto c = Index::MAX_CYCLE - 1;
    const auto m = Index::MAX_INDEX + 1;
    const auto inc = Index::MAX_INDEX + 2;

    const auto expectedIndex = (i + inc) % m;
    const auto expectedCycle = (i + inc) / m + c;

    Index index(i, c);
    Index result = index + inc;

    EXPECT_EQ(result.getIndex(), expectedIndex);
    EXPECT_EQ(result.getCycle(), expectedCycle);
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, cyclicIncrementWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb3ced77-f8f8-4bc9-893a-c8fc4e001482");
    using Index = typename TestFixture::Index;

    const auto i = Index::MAX_INDEX / 2;
    const auto c = Index::MAX_CYCLE - 1;
    const auto m = Index::MAX_INDEX + 1;

    const auto expectedIndex = (i + 1) % m;
    const auto expectedCycle = (i + 1) / m + c;

    Index index(i, c);
    Index next = index.next();

    EXPECT_EQ(next.getIndex(), expectedIndex);
    EXPECT_EQ(next.getCycle(), expectedCycle);
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, cyclicIncrementWraparound)
{
    ::testing::Test::RecordProperty("TEST_ID", "947872d1-c222-47aa-9391-1fd250e8e457");
    using Index = typename TestFixture::Index;

    const auto i = Index::MAX_INDEX;
    const auto c = Index::MAX_CYCLE - 1;

    const auto expectedIndex = 0;
    const auto expectedCycle = c + 1;

    Index index(i, c);
    Index next = index.next();

    EXPECT_EQ(next.getIndex(), expectedIndex);
    EXPECT_EQ(next.getCycle(), expectedCycle);

    // consistency check with operator +
    next = index + 1;
    EXPECT_EQ(next.getIndex(), expectedIndex);
    EXPECT_EQ(next.getCycle(), expectedCycle);
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, cyclicIncrementOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "2582ca2a-9acf-400a-90a6-ac35e23ac362");
    using Index = typename TestFixture::Index;

    const auto v = Index::MAX_VALUE;
    const auto i = Index::INDEX_AT_MAX_VALUE;
    const auto c = Index::MAX_CYCLE;

    const auto expectedIndex = Index::OVERFLOW_START_INDEX;
    const auto expectedCycle = 0;

    Index index(v);
    EXPECT_EQ(index.getIndex(), i);
    EXPECT_EQ(index.getCycle(), c);

    Index next = index.next();
    EXPECT_EQ(next.getIndex(), expectedIndex);
    EXPECT_EQ(next.getCycle(), expectedCycle);

    next = index + 1;
    EXPECT_EQ(next.getIndex(), expectedIndex);
    EXPECT_EQ(next.getCycle(), expectedCycle);
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, cyclicAdditionOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "5578f2c1-b2e8-43db-8943-967b7d77ba65");
    using Index = typename TestFixture::Index;

    const auto v = Index::MAX_VALUE;
    const auto m = Index::MAX_INDEX + 1;

    // general case: overflow more than one cycle, care must be taken for m = 1
    const auto deltaToMax = 1 + m;

    // add deltaToMax to reach Max, + 1 to cause overflow wraparound to 1, + (1+m) to overflow more
    // than one cycle
    const auto inc = deltaToMax + 2 + m;
    const auto expectedIndex = (Index::OVERFLOW_START_INDEX + 1) % m;
    const auto expectedCycle = (1 + m) / m; // this is 1 except for m = 2 where it is 2

    Index index(v - deltaToMax);
    Index result = index + inc;
    EXPECT_EQ(result.getIndex(), expectedIndex);
    EXPECT_EQ(result.getCycle(), expectedCycle);
}


TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, isOneCycleBehindCheckNegative)
{
    ::testing::Test::RecordProperty("TEST_ID", "d60bff1d-bc3f-40be-a555-56871e0b7248");
    using Index = typename TestFixture::Index;

    const auto i = Index::MAX_INDEX;
    const auto c = Index::MAX_CYCLE / 2;
    const auto m = Index::MAX_INDEX + 1;

    Index cycleStart(0, c);
    Index cycleMid(m / 2, c);
    Index cycleEnd(i, c);

    Index nextCycleEnd = cycleEnd + m;

    Index nextCycleStart2 = nextCycleEnd.next();

    // note: we do not iterate over all possible cases because depending on cyclelength
    // there could be too many (test parameterization)

    EXPECT_FALSE(cycleStart.isOneCycleBehind(cycleStart));
    EXPECT_FALSE(cycleEnd.isOneCycleBehind(cycleEnd));
    EXPECT_FALSE(cycleMid.isOneCycleBehind(cycleMid));

    EXPECT_FALSE(cycleMid.isOneCycleBehind(cycleEnd));
    EXPECT_FALSE(cycleEnd.isOneCycleBehind(cycleMid));

    EXPECT_FALSE(cycleStart.isOneCycleBehind(cycleEnd));
    EXPECT_FALSE(cycleEnd.isOneCycleBehind(cycleStart));

    EXPECT_FALSE(cycleEnd.isOneCycleBehind(nextCycleStart2));
    EXPECT_FALSE(nextCycleStart2.isOneCycleBehind(cycleEnd));

    EXPECT_FALSE(cycleMid.isOneCycleBehind(nextCycleStart2));
    EXPECT_FALSE(nextCycleStart2.isOneCycleBehind(cycleMid));

    EXPECT_FALSE(cycleStart.isOneCycleBehind(nextCycleStart2));
    EXPECT_FALSE(nextCycleStart2.isOneCycleBehind(cycleStart));
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, isOneCycleBehindCheckPositive)
{
    ::testing::Test::RecordProperty("TEST_ID", "313f9624-9a37-4376-a3f2-c23b8a29b610");
    using Index = typename TestFixture::Index;

    const auto i = Index::MAX_INDEX;
    const auto c = Index::MAX_CYCLE / 2;
    const auto m = Index::MAX_INDEX + 1;

    Index cycleStart(0, c);
    Index cycleMid(m / 2, c);
    Index cycleEnd(i, c);

    Index nextCycleStart = cycleStart + m;
    Index nextCycleMid = cycleMid + m;
    Index nextCycleEnd = cycleEnd + m;

    EXPECT_TRUE(cycleStart.isOneCycleBehind(nextCycleStart));
    EXPECT_TRUE(cycleStart.isOneCycleBehind(nextCycleMid));
    EXPECT_TRUE(cycleStart.isOneCycleBehind(nextCycleEnd));

    EXPECT_TRUE(cycleMid.isOneCycleBehind(nextCycleStart));
    EXPECT_TRUE(cycleMid.isOneCycleBehind(nextCycleMid));
    EXPECT_TRUE(cycleMid.isOneCycleBehind(nextCycleEnd));

    EXPECT_TRUE(cycleEnd.isOneCycleBehind(nextCycleStart));
    EXPECT_TRUE(cycleEnd.isOneCycleBehind(nextCycleMid));
    EXPECT_TRUE(cycleMid.isOneCycleBehind(nextCycleEnd));
}

TYPED_TEST(MpmcLockFreeQueueCyclicIndexTest, isOneCycleBehindCheckDuringOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "b94e9e40-9c43-46e1-96ee-319052018326");
    using Index = typename TestFixture::Index;

    // last cycle before overflow
    Index lastCycleStart(0, Index::MAX_CYCLE);
    Index lastIndexBeforeOverflow(Index::MAX_VALUE);

    // cycle after overflow, will not necessarily start with index 0
    Index firstIndexAfterOverflow(Index::OVERFLOW_START_INDEX, 0);
    Index firstCycleEnd(Index::MAX_INDEX, 0);

    // second cycle (after overflow)
    Index secondCycleStart = firstCycleEnd.next();

    EXPECT_FALSE(lastCycleStart.isOneCycleBehind(secondCycleStart));
    EXPECT_FALSE(secondCycleStart.isOneCycleBehind(lastCycleStart));

    EXPECT_FALSE(lastIndexBeforeOverflow.isOneCycleBehind(secondCycleStart));
    EXPECT_FALSE(secondCycleStart.isOneCycleBehind(lastIndexBeforeOverflow));

    EXPECT_TRUE(lastCycleStart.isOneCycleBehind(firstIndexAfterOverflow));
    EXPECT_TRUE(lastCycleStart.isOneCycleBehind(firstCycleEnd));

    EXPECT_TRUE(lastIndexBeforeOverflow.isOneCycleBehind(firstIndexAfterOverflow));
    EXPECT_TRUE(lastIndexBeforeOverflow.isOneCycleBehind(firstCycleEnd));
}


} // namespace
