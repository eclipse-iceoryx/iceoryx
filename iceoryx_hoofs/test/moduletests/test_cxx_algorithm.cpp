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

#include "iceoryx_hoofs/cxx/algorithm.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::algorithm;
using namespace iox::cxx;

class algorithm_test : public Test
{
  public:
    void SetUp()
    {
        testing::internal::CaptureStderr();
    }
    virtual void TearDown()
    {
        std::string output = testing::internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

TEST_F(algorithm_test, MaxOfOneElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "3fba70b4-252b-4c13-a98c-87b026254bba");
    EXPECT_THAT(max(12.34f), Eq(12.34f));
}

TEST_F(algorithm_test, MaxOfTwoElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "0443931f-3eb4-4ae2-99b3-029637f94d0f");
    EXPECT_THAT(max(56.78f, 12.34f), Eq(56.78f));
}

TEST_F(algorithm_test, MaxOfManyElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "83c16bb2-90c5-4226-bed2-7e5cc5b34f22");
    EXPECT_THAT(max(56.78f, 33.44f, 12.34f, -0.1f, 5.5f, 10001.f), Eq(10001.f));
}

TEST_F(algorithm_test, MinOfOneElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "384d8139-1a79-40ae-8caf-b468470c48d2");
    EXPECT_THAT(min(0.0123f), Eq(0.0123f));
}

TEST_F(algorithm_test, MinOfTwoElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0ad7d53-03f6-4ee2-9a0b-ee929dc047a7");
    EXPECT_THAT(min(0.0123f, -91.12f), Eq(-91.12f));
}

TEST_F(algorithm_test, MinOfManyElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "8ec6db69-2260-4af9-83fe-73ae58c878b3");
    EXPECT_THAT(min(0.0123f, -91.12f, 123.92f, -1021.2f, 0.0f), Eq(-1021.2f));
}

TEST_F(algorithm_test, DoesContainValue_ValueListOfZeroDoesNotContainValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "b8ef3cdf-8cfa-469c-ac67-7fc4afbc9b64");
    EXPECT_FALSE(doesContainValue(42));
}

TEST_F(algorithm_test, DoesContainValue_ValueListOfOneDoesNotContainValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "9836ad33-da77-476a-a074-1cf7878bcbe6");
    EXPECT_FALSE(doesContainValue(37, 13));
}

TEST_F(algorithm_test, DoesContainValue_ValueListOfOneDoesContainValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "643c842c-2dd2-4741-b344-b58fa5e32a6a");
    EXPECT_TRUE(doesContainValue(73, 73));
}

TEST_F(algorithm_test, DoesContainValue_ValueListOfMultipleValuesDoesNotContainValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "e0131b57-51b9-439f-a372-3725bfa7f24a");
    EXPECT_FALSE(doesContainValue(13, 42, 73, 7337));
}

TEST_F(algorithm_test, DoesContainValue_ValueListOfMultipleValuesDoesContainValueAtFront)
{
    ::testing::Test::RecordProperty("TEST_ID", "ad37f570-e20d-46ca-85ef-a62fdbfeb9c2");
    EXPECT_TRUE(doesContainValue(37, 37, 3773, 7535));
}

TEST_F(algorithm_test, DoesContainValue_ValueListOfMultipleValuesDoesContainValueInTheMiddle)
{
    ::testing::Test::RecordProperty("TEST_ID", "bbc397c4-5d15-4acf-a317-b93a6537571c");
    EXPECT_TRUE(doesContainValue(42, 13, 42, 555));
}

TEST_F(algorithm_test, DoesContainValue_ValueListOfMultipleValuesDoesContainValueAtEnd)
{
    ::testing::Test::RecordProperty("TEST_ID", "64c87a80-e83b-4e70-8f76-476f24804f19");
    EXPECT_TRUE(doesContainValue(7353, 42, 73, 7353));
}

TEST_F(algorithm_test, MergeTwoDisjunctNonEmptySortedContainers)
{
    ::testing::Test::RecordProperty("TEST_ID", "4f39641f-de8a-434a-8a50-cd2b66b476da");
    constexpr int64_t OFFSET = 1337;
    vector<int64_t, 10U> a, b;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i + OFFSET);
    }

    for (int64_t i = 5; i < 10; ++i)
    {
        b.emplace_back(i + OFFSET);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, b);
    auto mergedContainerSwitched = uniqueMergeSortedContainers(b, a);

    ASSERT_THAT(mergedContainer.size(), Eq(10U));
    for (int64_t i = 0; i < 10; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i + OFFSET));
    EXPECT_TRUE(mergedContainer == mergedContainerSwitched);
}

TEST_F(algorithm_test, MergeTwoDisjunctNonEmptySortedContainersWithAGap)
{
    ::testing::Test::RecordProperty("TEST_ID", "15d3c063-8bc5-47eb-84a4-35f055a1d82c");
    constexpr int64_t OFFSET = 41;
    constexpr int64_t GAP = 13;
    vector<int64_t, 10U> a, b;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i + OFFSET);
    }

    for (int64_t i = 5; i < 10; ++i)
    {
        b.emplace_back(i + OFFSET + GAP);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, b);
    auto mergedContainerSwitched = uniqueMergeSortedContainers(b, a);

    ASSERT_THAT(mergedContainer.size(), Eq(10U));
    for (int64_t i = 0; i < 5; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i + OFFSET));
    for (int64_t i = 5; i < 10; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i + OFFSET + GAP));
    EXPECT_TRUE(mergedContainer == mergedContainerSwitched);
}

TEST_F(algorithm_test, MergeTwoAlternatingDisjunctNonEmptySortedContainers)
{
    ::testing::Test::RecordProperty("TEST_ID", "02cc9514-6cfe-4e08-8806-f371561fef41");
    constexpr int64_t OFFSET = 4301;
    vector<int64_t, 10U> a, b;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i * 2 + OFFSET);
    }

    for (int64_t i = 0; i < 5; ++i)
    {
        b.emplace_back(i * 2 + 1 + OFFSET);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, b);
    auto mergedContainerSwitched = uniqueMergeSortedContainers(b, a);

    ASSERT_THAT(mergedContainer.size(), Eq(10U));
    for (int64_t i = 0; i < 10; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i + OFFSET));
    EXPECT_TRUE(mergedContainer == mergedContainerSwitched);
}

TEST_F(algorithm_test, MergingIdenticalContainerResultsInUnchangedContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "50f05cf2-62fa-49b8-8380-1dd0ac2470ec");
    constexpr int64_t OFFSET = 313;
    vector<int64_t, 10U> a;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i * 2 + OFFSET);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, a);

    ASSERT_THAT(mergedContainer.size(), Eq(5U));
    for (int64_t i = 0; i < 5; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i * 2 + OFFSET));
}

TEST_F(algorithm_test, MergingWithOneEmptyContainerResultsInUnchangedContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0a0eb3a-08a3-4898-a8c9-a4f7eff0115c");
    constexpr int64_t OFFSET = 707;
    vector<int64_t, 10U> a;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i * 3 + OFFSET);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, vector<int64_t, 10U>());

    ASSERT_THAT(mergedContainer.size(), Eq(5U));
    for (int64_t i = 0; i < 5; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i * 3 + OFFSET));
}

TEST_F(algorithm_test, MergePartiallyOverlappingSortedContainers)
{
    ::testing::Test::RecordProperty("TEST_ID", "c57dda77-81a5-413f-b54b-e924e67b66a5");
    constexpr int64_t OFFSET = 8055;
    vector<int64_t, 10U> a, b;

    for (int64_t i = 3; i < 10; ++i)
    {
        a.emplace_back(i + OFFSET);
    }

    for (int64_t i = 0; i < 8; ++i)
    {
        b.emplace_back(i + OFFSET);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, b);
    auto mergedContainerSwitched = uniqueMergeSortedContainers(b, a);

    ASSERT_THAT(mergedContainer.size(), Eq(10U));
    for (int64_t i = 0; i < 10; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i + OFFSET));
    EXPECT_TRUE(mergedContainer == mergedContainerSwitched);
}

TEST_F(algorithm_test, MergeWithDisjunctOneElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "7a56b0f9-82d2-4f9a-881f-338dd572a453");
    constexpr int64_t OFFSET = 333331;
    vector<int64_t, 10U> a, b;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i + OFFSET);
    }

    b.emplace_back(5 + OFFSET);

    auto mergedContainer = uniqueMergeSortedContainers(a, b);
    auto mergedContainerSwitched = uniqueMergeSortedContainers(b, a);

    ASSERT_THAT(mergedContainer.size(), Eq(6U));
    for (int64_t i = 0; i < 6; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i + OFFSET));
    EXPECT_TRUE(mergedContainer == mergedContainerSwitched);
}

TEST_F(algorithm_test, MergeWithOverlappingOneElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "05fb7baf-51e9-4ff9-bb35-8ae4174b0216");
    constexpr int64_t OFFSET = 29292929;
    vector<int64_t, 10U> a, b;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i + OFFSET);
    }

    b.emplace_back(0 + OFFSET);

    auto mergedContainer = uniqueMergeSortedContainers(a, b);
    auto mergedContainerSwitched = uniqueMergeSortedContainers(b, a);

    ASSERT_THAT(mergedContainer.size(), Eq(5U));
    for (int64_t i = 0; i < 5; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i + OFFSET));
    EXPECT_TRUE(mergedContainer == mergedContainerSwitched);
}
} // namespace
