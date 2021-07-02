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

#include <set>

namespace
{
using namespace ::testing;

using namespace iox;

// test with non-primitive comparable types
struct Integer
{
    uint32_t value;
};

bool operator==(const Integer& lhs, const Integer& rhs)
{
    return lhs.value == rhs.value;
}

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

    // to have some values in the tree in most (but not all) tests
    void insertTreeDefault()
    {
        sut.insert("abc", Integer{73});
        sut.insert("acb", Integer{37});
        sut.insert("abb", Integer{42});
        sut.insert("bbc", Integer{66});
    }

    void removeTreeDefault()
    {
        sut.remove("acb");
        sut.remove("abb");
        sut.remove("bbc");
        sut.remove("abc");
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
    EXPECT_TRUE(result);
    EXPECT_EQ(sut.size(), 1U);
}

TEST_F(PrefixTree_test, insertionUpToCapacityWorks)
{
    auto result = sut.insert("abc", Integer{73});
    EXPECT_TRUE(result);
    result = sut.insert("acb", Integer{37});
    EXPECT_TRUE(result);
    result = sut.insert("abb", Integer{42});
    EXPECT_TRUE(result);
    result = sut.insert("bbc", Integer{66});
    EXPECT_TRUE(result);

    // add remaining elements up to capacity for a duplicate key
    for (uint i = 4; i < TEST_CAPACITY; ++i)
    {
        result = sut.insert("abcd", Integer{i});
        EXPECT_TRUE(result);
    }

    EXPECT_EQ(sut.size(), TEST_CAPACITY);
}

TEST_F(PrefixTree_test, insertionIntoFullTreeDoesNotWork)
{
    insertTreeDefault();
    for (uint i = 4; i < TEST_CAPACITY; ++i)
    {
        sut.insert("abcd", Integer{i});
    }

    EXPECT_EQ(sut.size(), TEST_CAPACITY);

    auto result = sut.insert("cab", Integer{21});
    EXPECT_FALSE(result);
}

TEST_F(PrefixTree_test, insertionWithMaximumKeyLengthWorks)
{
    insertTreeDefault();
    sut.insert("abcdeeee", Integer{21});

    EXPECT_EQ(sut.size(), 5);
}

TEST_F(PrefixTree_test, insertedValueIsFound)
{
    insertTreeDefault();

    auto searchResult = sut.find("abb");
    ASSERT_EQ(searchResult.size(), 1);
    auto value = searchResult[0]->value;
    EXPECT_EQ(value, 42);
}

TEST_F(PrefixTree_test, searchingNonExistingKeyReturnsNoValue)
{
    insertTreeDefault();

    auto searchResult = sut.find("ab");
    EXPECT_EQ(searchResult.size(), 0);
}

TEST_F(PrefixTree_test, searchingKeyWithMultipleValuesReturnsAllValues)
{
    insertTreeDefault();

    sut.insert("ab", Integer{1});
    sut.insert("ab", Integer{2});

    auto searchResult = sut.find("ab");

    std::set<int> valuesExpected{1, 2};
    EXPECT_EQ(searchResult.size(), valuesExpected.size());

    std::set<int> valuesFound;
    for (auto element : searchResult)
    {
        valuesFound.insert(element->value);
    }
    
    EXPECT_EQ(valuesFound, valuesExpected);
}

// TODO: do we want this?
TEST_F(PrefixTree_test, searchingKeyWithDuplicateValuesReturnsDuplicateValues)
{
    insertTreeDefault();

    sut.insert("ab", Integer{21});
    sut.insert("ab", Integer{21});
    sut.insert("ab", Integer{21});

    auto searchResult = sut.find("ab");
    ASSERT_EQ(searchResult.size(), 3);
    auto value1 = searchResult[0]->value;
    auto value2 = searchResult[1]->value;
    auto value3 = searchResult[2]->value;

    EXPECT_EQ(value1, 21);
    EXPECT_EQ(value2, 21);
    EXPECT_EQ(value3, 21);
}

TEST_F(PrefixTree_test, searchingPrefixReturnsAllValues)
{
    sut.insert("abc", Integer{73});
    sut.insert("acb", Integer{37});
    sut.insert("abb", Integer{42});
    sut.insert("bbc", Integer{66});
    sut.insert("ab", Integer{11});
    sut.insert("abdd", Integer{22});

    auto searchResult = sut.findPrefix("ab");

    std::set<int> valuesExpected{73, 42, 11, 22};
    EXPECT_EQ(searchResult.size(), valuesExpected.size());

    std::set<int> valuesFound;
    for (auto element : searchResult)
    {
        valuesFound.insert(element->value);
    }


    EXPECT_EQ(valuesFound, valuesExpected);
}

TEST_F(PrefixTree_test, removingKeyRemovesAllItsAssociatedValues)
{
    insertTreeDefault();
    auto previousSize = sut.size();

    sut.insert("ab", Integer{1});
    sut.insert("ab", Integer{2});

    {
        auto searchResult = sut.find("ab");
        EXPECT_EQ(searchResult.size(), 2);
    }

    EXPECT_TRUE(sut.remove("ab"));

    {
        auto searchResult = sut.find("ab");
        EXPECT_EQ(searchResult.size(), 0);
    }

    EXPECT_EQ(sut.size(), previousSize);

    // keys with the removed key as prefix are unaffected and still in the structure
    {
        auto searchResult = sut.find("abc");
        EXPECT_EQ(searchResult.size(), 1);
    }
}

TEST_F(PrefixTree_test, removingAllKeysLeadsToEmptyTree)
{
    insertTreeDefault();

    sut.insert("ab", Integer{1});
    sut.insert("ab", Integer{2});

    auto currentSize = sut.size();
    EXPECT_EQ(currentSize, 6);

    removeTreeDefault();

    currentSize = sut.size();
    EXPECT_EQ(currentSize, 2);

    EXPECT_TRUE(sut.remove("ab"));

    currentSize = sut.size();
    EXPECT_EQ(currentSize, 0);
    EXPECT_TRUE(sut.empty());
}

TEST_F(PrefixTree_test, removingNonExistingKeyDoesNothing)
{
    insertTreeDefault();

    sut.insert("ab", Integer{1});
    sut.insert("ab", Integer{2});

    auto previousSize = sut.size();

    EXPECT_FALSE(sut.remove("abd"));

    EXPECT_EQ(sut.size(), previousSize);
}

TEST_F(PrefixTree_test, removingValueFromKeyWithSingleValueRemovesKey)
{
    insertTreeDefault();
    auto previousSize = sut.size();

    sut.insert("ab", Integer{22});

    EXPECT_TRUE(sut.remove("ab", Integer{22}));

    {
        auto searchResult = sut.find("ab");
        EXPECT_EQ(searchResult.size(), 0);
    }

    EXPECT_EQ(sut.size(), previousSize);

    // keys with the removed key as prefix are unaffected and still in the structure
    {
        auto searchResult = sut.find("abc");
        EXPECT_EQ(searchResult.size(), 1);
    }
}

TEST_F(PrefixTree_test, removingValueFromKeyWithMultipleValuesKeepsOtherValues)
{
    insertTreeDefault();
    auto previousSize = sut.size();

    sut.insert("ab", Integer{11});
    sut.insert("ab", Integer{22});

    EXPECT_TRUE(sut.remove("ab", Integer{22}));

    {
        auto searchResult = sut.find("ab");
        ASSERT_EQ(searchResult.size(), 1);
        auto value = searchResult[0]->value;
        EXPECT_EQ(value, 11);
    }

    EXPECT_EQ(sut.size(), previousSize + 1);

    // keys with the removed key as prefix are unaffected and still in the structure
    {
        auto searchResult = sut.find("abc");
        EXPECT_EQ(searchResult.size(), 1);
    }
}

TEST_F(PrefixTree_test, removingValueFromKeyWithDuplicateValuesRemovesAllValues)
{
    insertTreeDefault();
    auto previousSize = sut.size();

    sut.insert("ab", Integer{11});
    sut.insert("ab", Integer{11});

    EXPECT_TRUE(sut.remove("ab", Integer{11}));

    {
        auto searchResult = sut.find("ab");
        EXPECT_EQ(searchResult.size(), 0);
    }

    EXPECT_EQ(sut.size(), previousSize);

    // keys with the removed key as prefix are unaffected and still in the structure
    {
        auto searchResult = sut.find("abc");
        EXPECT_EQ(searchResult.size(), 1);
    }
}

TEST_F(PrefixTree_test, removingNonExistingValueFromExistingKeyDoesNothing)
{
    insertTreeDefault();

    sut.insert("ab", Integer{1});
    sut.insert("ab", Integer{2});

    auto previousSize = sut.size();

    EXPECT_FALSE(sut.remove("ab", Integer{3}));

    EXPECT_EQ(sut.size(), previousSize);
}

TEST_F(PrefixTree_test, removingValueFromNonExistingKeyDoesNothing)
{
    insertTreeDefault();

    sut.insert("ab", Integer{1});
    sut.insert("ab", Integer{2});

    auto previousSize = sut.size();

    EXPECT_FALSE(sut.remove("abd", Integer{1}));

    EXPECT_EQ(sut.size(), previousSize);
}

TEST_F(PrefixTree_test, removingElementsFromFullTreeAllowsInsertionOfNewElements)
{
    insertTreeDefault();
    for (uint i = 4; i < TEST_CAPACITY; ++i)
    {
        sut.insert("abcd", Integer{i});
    }

    EXPECT_EQ(sut.size(), TEST_CAPACITY);
    sut.remove("abc");
    EXPECT_EQ(sut.size(), TEST_CAPACITY - 1);

    // essentially a check whether the internal allocator can reuse the memory for the data
    EXPECT_TRUE(sut.insert("cab", Integer{21}));
}

// this test requires relocate_ptr to be used internally
TEST(PrefixTreeRelocation_test, relocatedTreeIsAnIndependentLogicalCopy)
{
    // TODO: we can use an optional<Tree> to simplify this
    // we want to zero out the memory after copy (and have to own it to do so)
    // we can get the memory from heap as well / + address sanitizer
    using Tree = TestPrefixTree<>;
    uint8_t originalMemory[sizeof(Tree)] alignas(alignof(Tree));

    // create and populate original tree
    auto original = new (&originalMemory) Tree;
    ASSERT_NE(original, nullptr);

    original->insert("abc", Integer{73});
    original->insert("acb", Integer{37});
    original->insert("abb", Integer{42});
    original->insert("bbc", Integer{66});

    // relocate memory by bitwise copy
    uint8_t relocationMemory[sizeof(Tree)] alignas(alignof(Tree));
    std::memcpy(&relocationMemory, original, sizeof(Tree));

    // zero original memory - we do not want a false positive test result
    // if the relocated class references this memory accidentally
    original->~Tree(); // technically not required since PrefixTree is self-contained (hence cannot leak anything)
    std::memset(&originalMemory, 0, sizeof(Tree));

    // relocated version should behave like the original
    // Note that this would also be true if it uses pointers to dynamic memory (which it does not).
    // This kind of relocation works in shared memory as well (with dynamic meory it would not).
    Tree* relocated = reinterpret_cast<Tree*>(&relocationMemory);
    EXPECT_EQ(relocated->size(), 4);

    auto result = relocated->find("abb");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->value, 42);

    // operations on relocated version should work as usual
    relocated->remove("abb");
    EXPECT_EQ(relocated->size(), 3);
    result = relocated->find("abb");
    EXPECT_EQ(result.size(), 0);

    relocated->insert("abcd", Integer{24});

    result = relocated->find("abcd");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->value, 24);

    // operations should not have any effect on original memory
    // (or other memory, but we cannot check this)

    uint8_t zeroedMemory[sizeof(Tree)] alignas(alignof(Tree));
    std::memset(&zeroedMemory, 0, sizeof(Tree));

    auto cmp = std::memcmp(&zeroedMemory, &originalMemory, sizeof(Tree));
    EXPECT_EQ(cmp, 0);

    relocated->~Tree();

    // dtor has no effect on original memory either
    // (we do not need to reset it to zero, test would have already failed
    // if this was needed)

    cmp = std::memcmp(&zeroedMemory, &originalMemory, sizeof(Tree));
    EXPECT_EQ(cmp, 0);
}

} // namespace
