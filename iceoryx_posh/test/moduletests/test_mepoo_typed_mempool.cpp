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

#include "iceoryx_posh/internal/mepoo/typed_mem_pool.hpp"

#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"

#include "test.hpp"

using namespace ::testing;
using namespace iox::mepoo;
class alignas(32) TypedMemPool_test : public Test
{
  public:
    class TestClass
    {
      public:
        TestClass(int a, int b)
            : a(a)
            , b(b)
        {
        }

        int a;
        int b;
    };
    static constexpr uint32_t NumberOfChunks{3};
    static constexpr uint32_t ChunkSize{128};

    static constexpr uint32_t LoFFLiMemoryRequirement{
        MemPool::freeList_t::requiredMemorySize(NumberOfChunks) + 100000};

    TypedMemPool_test()
        : allocator(m_rawMemory, NumberOfChunks * ChunkSize + LoFFLiMemoryRequirement)
        , sut(NumberOfChunks, &allocator, &allocator)
    {
    }

    void SetUp(){};
    void TearDown(){};

    alignas(32) uint8_t m_rawMemory[NumberOfChunks * ChunkSize + LoFFLiMemoryRequirement];
    iox::posix::Allocator allocator;

    TypedMemPool<TestClass> sut;
};

TEST_F(TypedMemPool_test, GetOneObject)
{
    auto object = sut.createObject(1, 223);
    ASSERT_THAT(object.has_error(), Eq(false));
    EXPECT_THAT(object.get_value()->a, Eq(1));
    EXPECT_THAT(object.get_value()->b, Eq(223));
}

TEST_F(TypedMemPool_test, ReleaseChunkWhenGoingOutOfScope)
{
    {
        auto object = sut.createObject(1, 234);
        EXPECT_THAT(sut.getUsedChunks(), Eq(1));
    }
    EXPECT_THAT(sut.getUsedChunks(), Eq(0));
}

TEST_F(TypedMemPool_test, OutOfChunksErrorWhenFull)
{
    auto object1 = sut.createObject(0xaffe, 0xdead);
    auto object2 = sut.createObject(0xaffe, 0xdead);
    auto object3 = sut.createObject(0xaffe, 0xdead);
    auto object4 = sut.createObject(0xaffe, 0xdead);

    EXPECT_THAT(object4.has_error(), Eq(true));
    EXPECT_THAT(object4.get_error(), Eq(TypedMemPoolError::OutOfChunks));
}
