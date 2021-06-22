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

#include "test.hpp"

#include "iceoryx_hoofs/data_structures/prefix_tree.hpp"

namespace
{
using namespace ::testing;

using namespace iox;

struct Integer
{
    uint32_t value;
};

static constexpr uint32_t TEST_CAPACITY = 8;
static constexpr uint32_t TEST_KEY_LENGTH = 8;
template <uint32_t Capacity = TEST_CAPACITY>
using TestPrefixTree = PrefixTree<Integer, Capacity, TEST_KEY_LENGTH>;

class PrefixTree_test : public ::testing::Test
{
  protected:
    PrefixTree_test()
    {
    }

    ~PrefixTree_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    TestPrefixTree<> sut;
};

TEST_F(PrefixTree_test, ctorConstructsEmptyTree)
{
    EXPECT_TRUE(sut.empty());
}

TEST_F(PrefixTree_test, insertionInEmptyTreeWorks)
{
    auto result = sut.insert("abc", Integer{73});
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(sut.size(), 1U);
}

TEST_F(PrefixTree_test, insertionUpToCapacityWorks)
{
    auto result = sut.insert("abc", Integer{73});
    EXPECT_NE(result, nullptr);
    result = sut.insert("acb", Integer{37});
    EXPECT_NE(result, nullptr);
    result = sut.insert("abb", Integer{42});
    EXPECT_NE(result, nullptr);
    result = sut.insert("bbc", Integer{66});
    EXPECT_NE(result, nullptr);

    // add remaining elements up to capacity for a duplicate key
    for (uint i = 4; i < TEST_CAPACITY; ++i)
    {
        result = sut.insert("abcd", Integer{i});
        EXPECT_NE(result, nullptr);
    }

    EXPECT_EQ(sut.size(), TEST_CAPACITY);
}

TEST_F(PrefixTree_test, insertionIntoFullTreeDoesNotWork)
{
    sut.insert("abc", Integer{73});
    sut.insert("acb", Integer{37});
    sut.insert("abb", Integer{42});
    sut.insert("bbc", Integer{66});

    for (uint i = 4; i < TEST_CAPACITY; ++i)
    {
        sut.insert("abcd", Integer{i});
    }

    EXPECT_EQ(sut.size(), TEST_CAPACITY);

    auto result = sut.insert("cab", Integer{21});
    EXPECT_EQ(result, nullptr);
}

TEST_F(PrefixTree_test, insertionWithMaximumKeyLengthWorks)
{
    sut.insert("abc", Integer{73});
    sut.insert("acb", Integer{37});
    sut.insert("abb", Integer{42});
    sut.insert("bbc", Integer{66});
    sut.insert("abcdeeee", Integer{21});

    EXPECT_EQ(sut.size(), 5);
}

TEST_F(PrefixTree_test, insertedValueIsFound)
{
    sut.insert("abc", Integer{73});
    sut.insert("acb", Integer{37});
    sut.insert("abb", Integer{42});
    sut.insert("bbc", Integer{66});

    auto searchResult = sut.find("abb");
    ASSERT_EQ(searchResult.size(), 1);
    auto value = searchResult[0]->value;
    EXPECT_EQ(value, 42);
}

TEST_F(PrefixTree_test, searchingNonExistingKeyReturnsNoValue)
{
    sut.insert("abc", Integer{73});
    sut.insert("acb", Integer{37});
    sut.insert("abb", Integer{42});
    sut.insert("bbc", Integer{66});

    auto searchResult = sut.find("ab");
    EXPECT_EQ(searchResult.size(), 0);
}

TEST_F(PrefixTree_test, searchingKeyWithMultipleValuesReturnsAllValues)
{
    sut.insert("abc", Integer{73});
    sut.insert("acb", Integer{37});
    sut.insert("abb", Integer{42});
    sut.insert("bbc", Integer{66});

    sut.insert("ab", Integer{1});
    sut.insert("ab", Integer{2});

    auto searchResult = sut.find("ab");
    ASSERT_EQ(searchResult.size(), 2);
    auto value1 = searchResult[0]->value;
    auto value2 = searchResult[1]->value;

    // we do not know in which order we find the values
    bool found1 = value1 == 1 || value2 == 1;
    bool found2 = value1 == 2 || value2 == 2;
    EXPECT_TRUE(found1 && found2);
}

// TODO: do we want this?
TEST_F(PrefixTree_test, searchingKeyWithDuplicateValuesReturnsDuplicateValues)
{
    sut.insert("abc", Integer{73});
    sut.insert("acb", Integer{37});
    sut.insert("abb", Integer{42});
    sut.insert("bbc", Integer{66});

    sut.insert("ab", Integer{21});
    sut.insert("ab", Integer{21});
    sut.insert("ab", Integer{21});

    auto searchResult = sut.find("ab");
    ASSERT_EQ(searchResult.size(), 3);
    auto value1 = searchResult[0]->value;
    auto value2 = searchResult[1]->value;
    auto value3 = searchResult[1]->value;

    EXPECT_EQ(value1, 21);
    EXPECT_EQ(value2, 21);
    EXPECT_EQ(value3, 21);
}

} // namespace
