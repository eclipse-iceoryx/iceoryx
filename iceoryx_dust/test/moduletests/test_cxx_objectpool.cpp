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

#include "iceoryx_dust/cxx/objectpool.hpp"

#include <vector>

#include "test.hpp"

namespace
{
using namespace ::testing;

constexpr int INVALID = -1;


// non primitive type for pool
class Foo
{
  public:
    Foo()
    {
        ++constructionCounter;
    }

    Foo(int& data)
        : m_data(&data)
    {
        *m_data = ++constructionCounter;
    }

    ~Foo()
    {
        if (m_data)
        {
            *m_data = INVALID; // invalidated by destructor, visible outside to check destructor call by Pool
        }
        ++destructionCounter;
    }

    int* m_data{nullptr};

    static void resetConstructionCounter()
    {
        constructionCounter = 0;
    }

    static int getConstructionCounter()
    {
        return constructionCounter;
    }

    static void resetDestructionCounter()
    {
        destructionCounter = 0;
    }

    static int getDestructionCounter()
    {
        return destructionCounter;
    }

    static int constructionCounter; // count the number of Foo objects constructed
    static int destructionCounter;  // count the number of Foo objects constructed
};

int Foo::constructionCounter{0};
int Foo::destructionCounter{0};

// needed signed and unsigned to avoid some sign comparison issues
constexpr int CAPACITY = 3;
constexpr size_t CAPACITY_UNSIGNED = static_cast<size_t>(CAPACITY);

using FooPool = iox::cxx::ObjectPool<Foo, CAPACITY>;
using Index_t = FooPool::Index_t;
constexpr int NO_INDEX = FooPool::NO_INDEX;

struct FooPoolWithPrivateMembersAccess : FooPool
{
    using FooPool::getFirstPtr;
    using FooPool::getLastPtr;
    using FooPool::nextFree;
};

class ObjectPool_test : public Test
{
  public:
    void SetUp() override
    {
        Foo::resetConstructionCounter(); // reset Foo constructionCounter in each test
        Foo::resetDestructionCounter();  // reset Foo destructionCounter in each test
    }
    void TearDown() override
    {
    }

    int data;
    int data1;
    int data2;
    int data3;
    FooPool pool;
    FooPoolWithPrivateMembersAccess poolExposed;
};

// check whether the constructed pool objects (of type Foo) have the intended data and construction/destruction
// behaviour  many other tests depend on this behaviour to track construction/destruction and associated data
TEST_F(ObjectPool_test, poolObjectBehaviour)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8381e4b-b775-47f1-aaac-3c3ac7484df6");
    // check that Foo objects behave correctly (i.e. static constructionCounter works and destructor effects data)
    int data = 73; // databuffer of foo
    {
        Foo foo(data);
        EXPECT_THAT(data, Eq(1));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(data));
        // Foo destructor called -> data invalid
    }
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(1));
    EXPECT_THAT(data, Eq(INVALID));

    {
        Foo foo(data);
        EXPECT_THAT(data, Eq(2));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(data));
    }
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(2));
    EXPECT_THAT(data, Eq(INVALID));

    Foo::resetConstructionCounter();
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0));

    Foo::resetDestructionCounter();
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
}


TEST_F(ObjectPool_test, construction)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b8a278a-7a3c-4b32-8e81-f0072d97e5c2");
    // pool initialized correctly, check size and capacity
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool
}


TEST_F(ObjectPool_test, reserve)
{
    ::testing::Test::RecordProperty("TEST_ID", "a91fc2f1-9d6d-4e85-9a3d-4620e31478ef");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

    for (int i = 1; i <= CAPACITY; ++i)
    {
        auto index = pool.reserve();
        EXPECT_THAT(index, Ne(NO_INDEX));
    }
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));

    // pool is full
    auto index = pool.reserve();
    EXPECT_THAT(index, Eq(NO_INDEX));
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));
}


TEST_F(ObjectPool_test, default_construct)
{
    ::testing::Test::RecordProperty("TEST_ID", "50fad76a-5eec-4812-b509-908f09ed71ac");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

    for (int i = 1; i <= CAPACITY; ++i)
    {
        auto index = pool.construct();
        EXPECT_THAT(index, Ne(NO_INDEX));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(i));
    }
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));

    // pool is full, nothing constructed
    auto index = pool.construct();
    EXPECT_THAT(index, Eq(NO_INDEX));
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));
}

TEST_F(ObjectPool_test, parameter_construct)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b0af6d2-3baf-4620-92e1-fe14770b03fd");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

    data = 0;
    for (int i = 1; i <= CAPACITY; ++i)
    {
        auto index = pool.construct(data);
        EXPECT_THAT(index, Ne(NO_INDEX));
        EXPECT_THAT(data, Eq(i));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(i));
    }
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));

    // pool is full, nothing constructed
    auto index = pool.construct(data);
    EXPECT_THAT(index, Eq(NO_INDEX));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(CAPACITY));
    EXPECT_THAT(data, Eq(CAPACITY));
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));
}

TEST_F(ObjectPool_test, add)
{
    ::testing::Test::RecordProperty("TEST_ID", "e5c09e18-a3dc-46e5-a8d8-fac27c3c8c47");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

    data = 0;
    for (int i = 1; i <= CAPACITY; ++i)
    {
        Foo foo(data);
        EXPECT_THAT(data, Eq(i));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(i));

        auto index = pool.add(foo);
        EXPECT_THAT(index, Ne(NO_INDEX));
    }
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));

    // pool is full, cannot add
    Foo foo(data);
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(CAPACITY + 1));
    EXPECT_THAT(data, Eq(CAPACITY + 1));
    auto index = pool.add(foo);
    EXPECT_THAT(index, Eq(NO_INDEX));
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));
}

TEST_F(ObjectPool_test, size_and_remove)
{
    ::testing::Test::RecordProperty("TEST_ID", "696ba57c-d761-4c8a-bb5f-60a519172a69");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool
    EXPECT_THAT(pool.size(), Eq(0U));

    data1 = 0;
    auto index1 = pool.construct(data1);
    EXPECT_THAT(data1, Eq(1));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(1));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(1U));

    data2 = 0;
    Foo foo(data2);
    auto index2 = pool.add(foo);
    EXPECT_THAT(data2, Eq(2));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(2));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(2U));
    ASSERT_THAT(pool.get(index1), Ne(nullptr));

    pool.remove(index1);
    EXPECT_THAT(pool.get(index1), Eq(nullptr));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(1U));

    ASSERT_THAT(pool.get(index2), Ne(nullptr));
    pool.remove(index2, true);
    EXPECT_THAT(data2, Eq(INVALID));
    EXPECT_THAT(pool.get(index2), Eq(nullptr));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(1));
    EXPECT_THAT(pool.size(), Eq(0U));
}

TEST_F(ObjectPool_test, bracket_operator)
{
    ::testing::Test::RecordProperty("TEST_ID", "79f4e1f0-cfeb-474c-a2f3-7b7b92cad27f");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool
    EXPECT_THAT(pool.size(), Eq(0U));

    data1 = 0;
    auto index1 = pool.construct(data1);
    EXPECT_THAT(data1, Eq(1));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(1));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(1U));
    ASSERT_THAT(pool.get(index1), Ne(nullptr));

    data2 = 0;
    Foo foo(data2);
    auto index2 = pool.add(foo);
    EXPECT_THAT(data2, Eq(2));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(2));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(2U));
    ASSERT_THAT(pool.get(index2), Ne(nullptr));

    Foo& ret1 = pool[index1]; // check whether we get the reference to the correct object (associated with data1)
    EXPECT_THAT(ret1.m_data, Eq(&data1));

    Foo& ret2 = pool[index2]; // check whether we get the reference to the correct object (associated with data2)
    EXPECT_THAT(ret2.m_data, Eq(&data2));
}

TEST_F(ObjectPool_test, allocate)
{
    ::testing::Test::RecordProperty("TEST_ID", "89ed7083-42be-4f71-bf31-9ce3ea5db6cf");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

    for (int i = 1; i <= CAPACITY; ++i)
    {
        auto ptr = pool.allocate();
        EXPECT_THAT(ptr, Ne(nullptr));
    }
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));

    // pool is full
    auto ptr = pool.allocate();
    EXPECT_THAT(ptr, Eq(nullptr));
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));
}

TEST_F(ObjectPool_test, default_create)
{
    ::testing::Test::RecordProperty("TEST_ID", "874342ce-0680-4634-b651-f72c2dee24c4");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

    for (int i = 1; i <= CAPACITY; ++i)
    {
        auto ptr = pool.create();
        EXPECT_THAT(ptr, Ne(nullptr));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(i));
    }
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));

    // pool is full, nothing constructed
    auto ptr = pool.create();
    EXPECT_THAT(ptr, Eq(nullptr));
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));
}

TEST_F(ObjectPool_test, parameter_create)
{
    ::testing::Test::RecordProperty("TEST_ID", "b08141d9-23b9-4a36-a694-236dce2ceaf7");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

    data = 0;
    for (int i = 1; i <= CAPACITY; ++i)
    {
        auto ptr = pool.create(data);
        EXPECT_THAT(ptr, Ne(nullptr));
        EXPECT_THAT(data, Eq(i));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(i));
    }
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));

    // pool is full, nothing constructed
    auto ptr = pool.create(data);
    EXPECT_THAT(ptr, Eq(nullptr));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(CAPACITY));
    EXPECT_THAT(data, Eq(CAPACITY));
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));
}

TEST_F(ObjectPool_test, destruct_free)
{
    ::testing::Test::RecordProperty("TEST_ID", "efd63af2-3c59-491f-b92b-4d9faa548e9b");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool
    EXPECT_THAT(pool.size(), Eq(0U));

    data1 = 0;
    auto ptr1 = pool.create(data1);
    EXPECT_THAT(data1, Eq(1));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(1));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(1U));

    data2 = 0;
    Foo foo(data2);
    auto ptr2 = pool.insert(foo);
    EXPECT_THAT(data2, Eq(2));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(2));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(2U));

    ASSERT_THAT(ptr1, Ne(nullptr));
    pool.free(ptr1, false);
    EXPECT_THAT(pool.get(ptr1), Eq(nullptr));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(1U));

    ASSERT_THAT(ptr2, Ne(nullptr));
    pool.free(ptr2, true);
    EXPECT_THAT(data2, Eq(INVALID));
    EXPECT_THAT(pool.get(ptr2), Eq(nullptr));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(1));
    EXPECT_THAT(pool.size(), Eq(0U));
}

TEST_F(ObjectPool_test, default_free)
{
    ::testing::Test::RecordProperty("TEST_ID", "55b755e5-8195-4fe4-8336-044416e644cd");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool
    EXPECT_THAT(pool.size(), Eq(0U));

    data1 = 0;
    auto ptr1 = pool.create(data1);
    EXPECT_THAT(data1, Eq(1));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(1));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(1U));

    data2 = 0;
    Foo foo(data2);
    auto ptr2 = pool.insert(foo);
    EXPECT_THAT(data2, Eq(2));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(2));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(2U));

    auto ptr3 = pool.allocate();
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(2));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(3U));

    ASSERT_THAT(ptr1, Ne(nullptr));
    pool.free(ptr1);
    EXPECT_THAT(pool.get(ptr1), Eq(nullptr));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(1));
    EXPECT_THAT(pool.size(), Eq(2U));

    ASSERT_THAT(ptr2, Ne(nullptr));
    pool.free(ptr2);
    EXPECT_THAT(data2, Eq(INVALID));
    EXPECT_THAT(pool.get(ptr2), Eq(nullptr));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(2));
    EXPECT_THAT(pool.size(), Eq(1U));

    ASSERT_THAT(ptr3, Ne(nullptr));
    pool.free(ptr3);
    EXPECT_THAT(pool.get(ptr3), Eq(nullptr));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(2));
    EXPECT_THAT(pool.size(), Eq(0U));
}

TEST_F(ObjectPool_test, insert)
{
    ::testing::Test::RecordProperty("TEST_ID", "d362be33-4ac1-420d-8502-c4dad69a66ff");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

    data = 0;
    for (int i = 1; i <= CAPACITY; ++i)
    {
        Foo foo(data);
        EXPECT_THAT(data, Eq(i));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(i));
        auto ptr = pool.insert(foo);
        EXPECT_THAT(ptr, Ne(nullptr));
    }
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));

    // pool is full, cannot add
    Foo foo(data);
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(CAPACITY + 1));
    EXPECT_THAT(data, Eq(CAPACITY + 1));
    auto ptr = pool.insert(foo);
    EXPECT_THAT(ptr, Eq(nullptr));
    EXPECT_THAT(pool.size(), Eq(CAPACITY_UNSIGNED));
}

TEST_F(ObjectPool_test, get)
{
    ::testing::Test::RecordProperty("TEST_ID", "9328ee17-c705-48ac-bcc6-e6a13d73829c");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool
    EXPECT_THAT(pool.size(), Eq(0U));

    data1 = 0;
    auto index1 = pool.construct(data1);
    EXPECT_THAT(data1, Eq(1));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(1));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(1U));

    data2 = 0;
    Foo foo(data2);
    auto ptr2 = pool.insert(foo);
    EXPECT_THAT(data2, Eq(2));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(2));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(2U));

    ASSERT_THAT(index1, Ne(NO_INDEX));
    ASSERT_THAT(ptr2, Ne(nullptr));

    auto ptr1 = pool.get(index1);
    ASSERT_THAT(ptr1, Ne(nullptr));

    auto index2 = pool.get(ptr2);
    auto ptr = pool.get(index2);
    ASSERT_THAT(ptr, Ne(nullptr));

    // check whether indicies/pointers are associated with the correct values
    EXPECT_THAT(ptr1->m_data, Eq(&data1));
    EXPECT_THAT(ptr2->m_data, Eq(&data2));
    EXPECT_THAT(ptr->m_data, Eq(&data2));

    // remove element and check whether we get a nullptr
    pool.free(ptr1, false);
    EXPECT_THAT(pool.get(ptr1), Eq(nullptr));
    EXPECT_THAT(pool.get(index1), Eq(nullptr));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(1U));
}

TEST_F(ObjectPool_test, pointerToIndexConversion)
{
    ::testing::Test::RecordProperty("TEST_ID", "69ece67d-0d9e-44a3-bb16-956bdc591ee3");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool
    EXPECT_THAT(pool.size(), Eq(0U));

    data1 = 0;
    auto index1 = pool.construct(data1);
    EXPECT_THAT(data1, Eq(1));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(1));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(1U));
    ASSERT_THAT(index1, Ne(NO_INDEX));

    auto ptr1 = pool.get(index1);
    ASSERT_THAT(ptr1, Ne(nullptr));

    // check whether indicies/pointers are associated with the correct values
    EXPECT_THAT(ptr1->m_data, Eq(&data1));

    EXPECT_THAT(pool.pointerToIndex(ptr1), Eq(index1));
    EXPECT_THAT(pool.indexToPointer(index1), Eq(ptr1));

    // remove element and check whether we get a nullptr
    pool.free(ptr1);
    EXPECT_THAT(pool.get(ptr1), Eq(nullptr));
    EXPECT_THAT(pool.get(index1), Eq(nullptr));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(1));
    EXPECT_THAT(pool.size(), Eq(0U));

    // conversion does not care about valid data, index1 still corresponds to ptr1
    EXPECT_THAT(pool.pointerToIndex(ptr1), Eq(index1));
    EXPECT_THAT(pool.indexToPointer(index1), Eq(ptr1));
}

TEST_F(ObjectPool_test, pointerToIndexLegalPointerConversion)
{
    ::testing::Test::RecordProperty("TEST_ID", "0e03caaa-1d47-4c19-823a-5456b732bff6");
    data = 0;
    EXPECT_THAT(poolExposed.construct(data), Ne(NO_INDEX));

    auto first = reinterpret_cast<Foo*>(poolExposed.getFirstPtr());
    auto last = reinterpret_cast<Foo*>(poolExposed.getLastPtr());
    auto alignedPtr = reinterpret_cast<Foo*>(poolExposed.getFirstPtr() + sizeof(Foo));

    EXPECT_THAT(poolExposed.pointerToIndex(first), Eq(0));
    EXPECT_THAT(poolExposed.pointerToIndex(last), Eq(CAPACITY - 1));
    EXPECT_THAT(poolExposed.pointerToIndex(alignedPtr), Eq(1));
}

TEST_F(ObjectPool_test, pointerToIndexIllegalPointerConversion)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f127dc6-944d-4723-8e99-cf45ed75e4ff");
    data = 0;
    EXPECT_THAT(poolExposed.construct(data), Ne(NO_INDEX));

    auto lowOutOfMemoryPtr = reinterpret_cast<Foo*>(poolExposed.getFirstPtr() - 1);
    auto highOutOfMemoryPtr = reinterpret_cast<Foo*>(poolExposed.getLastPtr() + 1);

    constexpr size_t ONE_BYTE{1};
    ASSERT_THAT(sizeof(Foo), Gt(ONE_BYTE));
    auto nonAlignedPtr = reinterpret_cast<Foo*>(poolExposed.getFirstPtr() + sizeof(Foo) + ONE_BYTE);

    EXPECT_THAT(poolExposed.pointerToIndex(lowOutOfMemoryPtr), Eq(NO_INDEX));
    EXPECT_THAT(poolExposed.pointerToIndex(highOutOfMemoryPtr), Eq(NO_INDEX));
    EXPECT_THAT(poolExposed.pointerToIndex(nonAlignedPtr), Eq(NO_INDEX));
}

// private API, important for correct behaviour of all other functions
// also test whether finding the next free cell (if it exists) works correctly
TEST_F(ObjectPool_test, nextFree)
{
    ::testing::Test::RecordProperty("TEST_ID", "d554c604-186f-4372-8ce7-633df8aeddcc");
    EXPECT_THAT(poolExposed.size(), Eq(0U));
    EXPECT_THAT(poolExposed.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by poolExposed

    for (int i = 1; i <= CAPACITY; ++i)
    {
        // changes object state but does not matter, if there is some free cell it has to be found
        EXPECT_THAT(poolExposed.nextFree(), Ne(NO_INDEX));

        // populate poolExposed
        auto index = poolExposed.reserve();
        EXPECT_THAT(index, Ne(NO_INDEX));
    }
    EXPECT_THAT(poolExposed.size(), Eq(CAPACITY_UNSIGNED));

    // poolExposed is full
    auto index = poolExposed.reserve();
    EXPECT_THAT(index, Eq(NO_INDEX));
    EXPECT_THAT(poolExposed.size(), Eq(CAPACITY_UNSIGNED));

    EXPECT_THAT(poolExposed.nextFree(), Eq(NO_INDEX));
}

TEST_F(ObjectPool_test, destructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "138bcd54-54e8-47ef-a0d0-dbf464789e41");
    // allocate objects without construction
    {
        FooPool localPool; // local pool, to check destruction of objects

        EXPECT_THAT(localPool.size(), Eq(0U));
        EXPECT_THAT(localPool.capacity(), Eq(CAPACITY_UNSIGNED));
        EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

        for (int i = 1; i <= CAPACITY; ++i)
        {
            // populate pool
            auto index = localPool.reserve();
            EXPECT_THAT(index, Ne(NO_INDEX));
        }
        EXPECT_THAT(localPool.size(), Eq(CAPACITY_UNSIGNED));

        // pool is full
        auto index = localPool.reserve();
        EXPECT_THAT(index, Eq(NO_INDEX));
        EXPECT_THAT(localPool.size(), Eq(CAPACITY_UNSIGNED));
        EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    }
    // localPool destructor was called, but since the objects of the pool
    // were not construced by the pool (merely allocated) no Foo destructors are called
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));

    // default construction of Foo objects by pool
    {
        FooPool localPool; // local pool, to check destruction of objects

        EXPECT_THAT(localPool.size(), Eq(0U));
        EXPECT_THAT(localPool.capacity(), Eq(CAPACITY_UNSIGNED));
        EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

        for (int i = 1; i <= CAPACITY; ++i)
        {
            // populate pool
            auto index = localPool.construct();
            EXPECT_THAT(index, Ne(NO_INDEX));
        }
        EXPECT_THAT(localPool.size(), Eq(CAPACITY_UNSIGNED));

        // pool is full
        auto index = localPool.reserve();
        EXPECT_THAT(index, Eq(NO_INDEX));
        EXPECT_THAT(localPool.size(), Eq(CAPACITY_UNSIGNED));
        EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    }
    // localPool destructor was called, and since the objects were constructed by the pool
    // Foo destructors are called CAPACITY times
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(CAPACITY));

    Foo::resetConstructionCounter();
    Foo::resetDestructionCounter();
    data = 0;

    // default construction of Foo objects by pool
    {
        FooPool localPool; // local pool, to check destruction of objects

        EXPECT_THAT(localPool.size(), Eq(0U));
        EXPECT_THAT(localPool.capacity(), Eq(CAPACITY_UNSIGNED));
        EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
        EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool

        for (int i = 1; i <= CAPACITY; ++i)
        {
            // populate pool
            auto index = localPool.construct(data);
            EXPECT_THAT(index, Ne(NO_INDEX));
        }
        EXPECT_THAT(localPool.size(), Eq(CAPACITY_UNSIGNED));

        // pool is full
        auto index = localPool.reserve();
        EXPECT_THAT(index, Eq(NO_INDEX));
        EXPECT_THAT(localPool.size(), Eq(CAPACITY_UNSIGNED));
        EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    }
    // localPool destructor was called, and since the objects were constructed by the pool
    // Foo destructors are called CAPACITY times
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(CAPACITY));
    EXPECT_THAT(data, Eq(INVALID));
}

// test all iterator functions in this test since they are closely related
TEST_F(ObjectPool_test, iterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "2a8ffc21-1ec3-4319-ac2e-89db82f80c98");
    EXPECT_THAT(pool.size(), Eq(0U));
    EXPECT_THAT(pool.capacity(), Eq(CAPACITY_UNSIGNED));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(0)); // no Foo objects constructed by pool
    EXPECT_THAT(pool.size(), Eq(0U));

    data1 = 0;
    auto index1 = pool.construct(data1);
    EXPECT_THAT(index1, Ne(NO_INDEX));
    EXPECT_THAT(data1, Eq(1));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(1));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(1U));

    data2 = 0;
    auto index2 = pool.construct(data2);
    EXPECT_THAT(index2, Ne(NO_INDEX));
    EXPECT_THAT(data2, Eq(2));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(2));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(2U));

    data3 = 0;
    auto index3 = pool.construct(data3);
    EXPECT_THAT(index3, Ne(NO_INDEX));
    EXPECT_THAT(data3, Eq(3));
    EXPECT_THAT(Foo::getConstructionCounter(), Eq(3));
    EXPECT_THAT(Foo::getDestructionCounter(), Eq(0));
    EXPECT_THAT(pool.size(), Eq(3U));

    // container is full

    // construct iterators

    auto iter1 = pool.iterator(index1);
    auto iter2 = pool.iterator(index2);
    auto iter3 = pool.iterator(index3);
    auto iterBegin = pool.begin();
    auto iterEnd = pool.end();


    // comparison test (operator!=)
    EXPECT_THAT(iterBegin, Ne(iterEnd));
    EXPECT_THAT(iter1, Ne(iterEnd));
    EXPECT_THAT(iter2, Ne(iterEnd));
    EXPECT_THAT(iter3, Ne(iterEnd));

    // pairwise not equal
    EXPECT_THAT(iter1, Ne(iter2));
    EXPECT_THAT(iter1, Ne(iter3));
    EXPECT_THAT(iter2, Ne(iter3));

    EXPECT_THAT(iter1, Eq(iter1));
    EXPECT_THAT(iter2, Eq(iter2));
    EXPECT_THAT(iter3, Eq(iter3));
    EXPECT_THAT(iterBegin, Eq(iterBegin));
    EXPECT_THAT(iterEnd, Eq(iterEnd));

    // operator++

    // post increment
    auto iter = iter1; // we need the copy since operator++ changes the iterator
    EXPECT_THAT(iter++, Eq(iter1));

    iter = iter2;
    EXPECT_THAT(iter++, Eq(iter2));

    iter = iter3;
    EXPECT_THAT(iter++, Eq(iter3));

    iter = iterBegin;
    EXPECT_THAT(iter++, Eq(iterBegin));

    iter = iterEnd;
    EXPECT_THAT(iter++, Eq(iterEnd));


    // pre increment
    iter = iter1;
    EXPECT_THAT(++iter, Ne(iter1));

    iter = iter2;
    EXPECT_THAT(++iter, Ne(iter2));

    iter = iter3;
    EXPECT_THAT(++iter, Ne(iter3));

    iter = iterBegin;
    EXPECT_THAT(++iter, Ne(iterBegin));

    iter = iterEnd;
    EXPECT_THAT(++iter, Eq(iterEnd));

    // operator*
    EXPECT_THAT(((*iter1).m_data), Eq(&data1));
    EXPECT_THAT(((*iter2).m_data), Eq(&data2));
    EXPECT_THAT(((*iter3).m_data), Eq(&data3));

    // operator->
    EXPECT_THAT(iter1->m_data, Eq(&data1));
    EXPECT_THAT(iter2->m_data, Eq(&data2));
    EXPECT_THAT(iter3->m_data, Eq(&data3));
    EXPECT_THAT(iterEnd.operator->(), Eq(nullptr));

    // check that after CAPACITY increments we have reached end
    //(to reduce potential for unbounded loops)
    auto iterPre = pool.begin();
    auto iterPost = pool.begin();

    for (size_t i = 0; i < pool.size(); ++i)
    {
        ++iterPre;
        iterPost++;
    }
    ASSERT_THAT(iterPre, Eq(iterEnd));
    ASSERT_THAT(iterPost, Eq(iterEnd));

    // we now know that the iterator increment does not lead to unbounded loops...
    // test range based loop which relies on iterators internally

    std::vector<int> count(4, 0);
    int numElements = 0;
    for (auto& foo : pool)
    {
        auto value = *foo.m_data;
        if (value >= 1 && value <= 3)
        {
            count[static_cast<uint64_t>(value)]++;
        }
        ++numElements;
    }

    EXPECT_THAT(numElements, Eq(3));
    for (uint64_t i = 1U; i <= 3U; ++i)
    {
        EXPECT_THAT(count[i], Eq(1)); // expect each value exactly once
    }

    // remove an element and iterate over pool

    ASSERT_THAT(pool.get(index2), Ne(nullptr));
    pool.remove(index2);
    EXPECT_THAT(pool.size(), Eq(2U));

    // only 1 and 3 remain in the pool
    numElements = 0;
    for (auto& foo : pool)
    {
        auto value = *foo.m_data;
        if (value >= 1 && value <= 3)
        {
            count[static_cast<uint64_t>(value)]++;
        }
        ++numElements;
    }

    EXPECT_THAT(numElements, Eq(2));
    EXPECT_THAT(count[1], Eq(2));
    EXPECT_THAT(count[2], Eq(1)); // count unchanged because element 2 was removed
    EXPECT_THAT(count[3], Eq(2));

    // remove remaining elements

    ASSERT_THAT(pool.get(index1), Ne(nullptr));
    pool.remove(index1);
    EXPECT_THAT(pool.size(), Eq(1U));


    // only 3 remains in the pool
    numElements = 0;
    for (auto& foo : pool)
    {
        auto value = *foo.m_data;
        if (value >= 1 && value <= 3)
        {
            count[static_cast<uint64_t>(value)]++;
        }
        ++numElements;
    }

    EXPECT_THAT(numElements, Eq(1));
    EXPECT_THAT(count[1], Eq(2)); // count unchanged because element 1 was removed
    EXPECT_THAT(count[2], Eq(1)); // count unchanged because element 2 was removed
    EXPECT_THAT(count[3], Eq(3));

    ASSERT_THAT(pool.get(index3), Ne(nullptr));
    pool.remove(index3);
    EXPECT_THAT(pool.size(), Eq(0U));

    // pool is empty
    numElements = 0;
    for (auto& foo : pool)
    {
        auto value = *foo.m_data;
        if (value >= 1 && value <= 3)
        {
            count[static_cast<uint64_t>(value)]++;
        }
        ++numElements;
    }

    EXPECT_THAT(numElements, Eq(0));
    // all counts unchanged
    EXPECT_THAT(count[1], Eq(2));
    EXPECT_THAT(count[2], Eq(1));
    EXPECT_THAT(count[3], Eq(3));

    // empty pool, begin equals end
    EXPECT_THAT(pool.begin(), Eq(pool.end()));
}
} // namespace
