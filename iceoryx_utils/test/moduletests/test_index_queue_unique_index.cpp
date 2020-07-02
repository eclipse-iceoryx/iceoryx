// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "test.hpp"

#include "iceoryx_utils/internal/concurrent/lockfree_queue/index_queue.hpp"
using namespace ::testing;

namespace
{
using iox::concurrent::IndexQueue;

// by design, we need an IndexQueue to construct valid UniqueIndices,
// this protects against certain errors but in turn requires
// an Indexqueue to test the UniqueIndex

// since the implementation detail of using the unique<T> template is subject to
// further change, it is not tested yet
// however, except for that it allows construction and general types T,
// it has similar semantics so tests could be adapted
// the idea is, that each resource of type T constructed in such a way is only movable
// and not copyable (maybe rename in move_only?)

class UniqueIndexTest : public ::testing::Test
{
  public:
    using Queue = IndexQueue<2>;
    using UniqueIndex = Queue::UniqueIndex;

  protected:
    UniqueIndexTest()
    {
    }

    ~UniqueIndexTest()
    {
    }

    void SetUp()
    {
        indexQueue.pop(); // discards the index 0
    }

    void TearDown()
    {
    }

    Queue indexQueue{Queue::ConstructFull};

    UniqueIndex acquireIndex()
    {
        // returns the index 1
        // better for some tests due to default zero initilization of some types
        // which makes false positives in tests much less likely
        return indexQueue.pop();
    }

    void returnIndex(UniqueIndex& index)
    {
        indexQueue.push(index);
    }
};

// we *cannot* acquire a valid unique index in any other way since the constructor
// is private and only accessible by the friend IndexQueue
// this is the main use case of the IndexQueue
TEST_F(UniqueIndexTest, indexQueueConstructsValidIndexWhenAvailable)
{
    auto index1 = acquireIndex();
    EXPECT_TRUE(index1.isValid());
    EXPECT_EQ(index1, 1); // returned index has value 0 by design of the IndexQueue

    // capacity exhausted, no valid indices left until we return one
    auto index2 = acquireIndex();
    EXPECT_FALSE(index2.isValid());

    returnIndex(index1);
    EXPECT_FALSE(index1.isValid());

    auto index3 = acquireIndex();
    EXPECT_TRUE(index3.isValid());
    EXPECT_EQ(index3, 1);
}

TEST_F(UniqueIndexTest, explicitlyInvalidConstructedIndexIsInvalid)
{
    UniqueIndex index(UniqueIndex::invalid);
    EXPECT_FALSE(index.isValid());
}

TEST_F(UniqueIndexTest, moveInvalidatesValidIndex)
{
    auto index1 = acquireIndex();
    EXPECT_TRUE(index1.isValid());

    UniqueIndex index2(std::move(index1));
    EXPECT_TRUE(index2.isValid());
    EXPECT_EQ(index2, 1);

    EXPECT_FALSE(index1.isValid());
}

TEST_F(UniqueIndexTest, moveAssignmentInvalidatesValidIndex)
{
    auto index1 = acquireIndex();
    EXPECT_TRUE(index1.isValid());

    UniqueIndex index2(UniqueIndex::invalid);
    index2 = std::move(index1);

    EXPECT_TRUE(index2.isValid());
    EXPECT_EQ(index2, 1);

    EXPECT_FALSE(index1.isValid());
}

TEST_F(UniqueIndexTest, selfMoveAssignmentDoesNotInvalidateValidIndex)
{
    auto index = acquireIndex();
    /// we are testing self move here therefore we do not need a warning that we do
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
    index = std::move(index);
#pragma GCC diagnostic pop
    EXPECT_TRUE(index.isValid());
    EXPECT_EQ(index, 1);
}

TEST_F(UniqueIndexTest, selfMoveAssignedInvalidIndexStaysInvalid)
{
    UniqueIndex index(UniqueIndex::invalid);

    /// we are testing self move here therefore we do not need a warning that we do
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
    index = std::move(index);
#pragma GCC diagnostic pop
    EXPECT_FALSE(index.isValid());
}

TEST_F(UniqueIndexTest, movedInvalidIndexStaysInvalid)
{
    UniqueIndex index1(UniqueIndex::invalid);
    EXPECT_FALSE(index1.isValid());

    UniqueIndex index2(std::move(index1));

    EXPECT_FALSE(index2.isValid());
    EXPECT_FALSE(index1.isValid());
}

TEST_F(UniqueIndexTest, moveAssignedInvalidIndexStaysInvalid)
{
    UniqueIndex index1(UniqueIndex::invalid);
    EXPECT_FALSE(index1.isValid());

    UniqueIndex index2(UniqueIndex::invalid);
    index2 = std::move(index1);

    EXPECT_FALSE(index2.isValid());
    EXPECT_FALSE(index1.isValid());
}

TEST_F(UniqueIndexTest, moveAssignmentOfInvalidIndexInvalidatesDestination)
{
    UniqueIndex index1(UniqueIndex::invalid);

    auto index2 = acquireIndex();
    EXPECT_TRUE(index2.isValid());

    index2 = std::move(index1);

    EXPECT_FALSE(index2.isValid());
    EXPECT_FALSE(index1.isValid());
}

TEST_F(UniqueIndexTest, readAccessDoesNotInvalidateIndex)
{
    auto index = acquireIndex();
    const auto& ref = *index;

    EXPECT_EQ(ref, 1);
    EXPECT_TRUE(index.isValid());
}

TEST_F(UniqueIndexTest, releaseInvalidatesIndex)
{
    auto index = acquireIndex();
    auto value = index.release();

    EXPECT_EQ(value, 1);
    EXPECT_FALSE(index.isValid());
}

TEST_F(UniqueIndexTest, conversionToValueTypeDoesNotInvalidateIndex)
{
    auto index = acquireIndex();
    UniqueIndex::value_t value{73};
    value = index;

    EXPECT_EQ(value, 1);
    EXPECT_TRUE(index.isValid());
}


} // namespace
