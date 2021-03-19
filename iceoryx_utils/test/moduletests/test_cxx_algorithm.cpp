// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/cxx/algorithm.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::algorithm;
using namespace iox::cxx;

namespace
{
class algorithm_test : public Test
{
  public:
    void SetUp()
    {
        internal::CaptureStderr();
    }
    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

} // namespace

TEST_F(algorithm_test, MaxOfOneElement)
{
    EXPECT_THAT(max(12.34f), Eq(12.34f));
}

TEST_F(algorithm_test, MaxOfTwoElements)
{
    EXPECT_THAT(max(56.78f, 12.34f), Eq(56.78f));
}

TEST_F(algorithm_test, MaxOfManyElements)
{
    EXPECT_THAT(max(56.78f, 33.44f, 12.34f, -0.1f, 5.5f, 10001.f), Eq(10001.f));
}

TEST_F(algorithm_test, MinOfOneElement)
{
    EXPECT_THAT(min(0.0123f), Eq(0.0123f));
}

TEST_F(algorithm_test, MinOfTwoElements)
{
    EXPECT_THAT(min(0.0123f, -91.12f), Eq(-91.12f));
}

TEST_F(algorithm_test, MinOfManyElements)
{
    EXPECT_THAT(min(0.0123f, -91.12f, 123.92f, -1021.2f, 0.0f), Eq(-1021.2f));
}

TEST_F(algorithm_test, MergeTwoDisjunctNonEmptySortedContainers)
{
    vector<int64_t, 10U> a, b;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i);
    }

    for (int64_t i = 5; i < 10; ++i)
    {
        b.emplace_back(i);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, b);

    ASSERT_THAT(mergedContainer.size(), Eq(10U));
    for (int64_t i = 0; i < 10; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i));
}

TEST_F(algorithm_test, MergeTwoAlternatingDisjunctNonEmptySortedContainers)
{
    vector<int64_t, 10U> a, b;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i * 2);
    }

    for (int64_t i = 0; i < 5; ++i)
    {
        b.emplace_back(i * 2 + 1);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, b);

    ASSERT_THAT(mergedContainer.size(), Eq(10U));
    for (int64_t i = 0; i < 10; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i));
}

TEST_F(algorithm_test, MergingIdenticalContainerResultsInUnchangedContainer)
{
    vector<int64_t, 10U> a;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i * 2);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, a);

    ASSERT_THAT(mergedContainer.size(), Eq(5U));
    for (int64_t i = 0; i < 5; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i * 2));
}

TEST_F(algorithm_test, MergingWithOneEmptyContainerResultsInUnchangedContainer)
{
    vector<int64_t, 10U> a;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i * 3);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, vector<int64_t, 10U>());

    ASSERT_THAT(mergedContainer.size(), Eq(5U));
    for (int64_t i = 0; i < 5; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i * 3));
}

TEST_F(algorithm_test, MergePartiallyOverlappingSortedContainers)
{
    vector<int64_t, 10U> a, b;

    for (int64_t i = 3; i < 10; ++i)
    {
        a.emplace_back(i);
    }

    for (int64_t i = 0; i < 8; ++i)
    {
        b.emplace_back(i);
    }

    auto mergedContainer = uniqueMergeSortedContainers(a, b);

    ASSERT_THAT(mergedContainer.size(), Eq(10U));
    for (int64_t i = 0; i < 10; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i));
}

TEST_F(algorithm_test, MergeWithDisjunctOneElementContainer)
{
    vector<int64_t, 10U> a, b;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i);
    }

    b.emplace_back(5);

    auto mergedContainer = uniqueMergeSortedContainers(a, b);

    ASSERT_THAT(mergedContainer.size(), Eq(6U));
    for (int64_t i = 0; i < 6; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i));
}

TEST_F(algorithm_test, MergeWithOverlappingOneElementContainer)
{
    vector<int64_t, 10U> a, b;

    for (int64_t i = 0; i < 5; ++i)
    {
        a.emplace_back(i);
    }

    b.emplace_back(0);

    auto mergedContainer = uniqueMergeSortedContainers(a, b);

    ASSERT_THAT(mergedContainer.size(), Eq(5U));
    for (int64_t i = 0; i < 5; ++i)
        EXPECT_THAT(mergedContainer[i], Eq(i));
}
