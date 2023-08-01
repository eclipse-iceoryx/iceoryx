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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/shared_pointer.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iox/bump_allocator.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox::mepoo;

class SharedPointer_Test : public Test
{
  public:
    struct counter_t
    {
        uint64_t ctor{0};
        uint64_t dtor{0};
        uint64_t moveCtor{0};
        uint64_t copyCtor{0};
        uint64_t moveAssignment{0};
        uint64_t copyAssignment{0};
    };


    class TestClass
    {
      public:
        TestClass()
        {
            counter.ctor++;
        }

        TestClass(int a, int b)
            : a{a}
            , b{b}
        {
            counter.ctor++;
        }

        TestClass(const TestClass& v)
        {
            this->a = v.a;
            this->b = v.b;
            counter.copyCtor++;
        }

        TestClass(TestClass&& v)
        {
            this->a = v.a;
            this->b = v.b;
            counter.moveCtor++;
        }

        TestClass& operator=(const TestClass& v)
        {
            this->a = v.a;
            this->b = v.b;
            counter.copyAssignment++;
            return *this;
        }

        TestClass& operator=(TestClass&& v)
        {
            this->a = v.a;
            this->b = v.b;
            counter.moveAssignment++;
            return *this;
        }

        ~TestClass()
        {
            counter.dtor++;
        }

        void Increase()
        {
            a += 12;
            b += 819;
        }

        int a;
        int b;
        static counter_t counter;
    };

    void SetUp() override
    {
        iox::UntypedRelativePointer::registerPtr(memory, 4096);
    }
    void TearDown() override
    {
        iox::UntypedRelativePointer::unregisterAll();
    }

    ChunkManagement* GetChunkManagement(void* memoryChunk)
    {
        ChunkManagement* v = static_cast<ChunkManagement*>(chunkMgmtPool.getChunk());

        auto chunkSettingsResult = ChunkSettings::create(USER_PAYLOAD_SIZE, iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT);
        EXPECT_FALSE(chunkSettingsResult.has_error());
        if (chunkSettingsResult.has_error())
        {
            return nullptr;
        }
        auto& chunkSettings = chunkSettingsResult.value();

        ChunkHeader* chunkHeader = new (memoryChunk) ChunkHeader(mempool.getChunkSize(), chunkSettings);
        new (v) ChunkManagement{chunkHeader, &mempool, &chunkMgmtPool};
        return v;
    }

    int ResetCounter()
    {
        TestClass::counter = counter_t();
        return 0;
    }

    int resetCounter = ResetCounter();

    static constexpr uint32_t USER_PAYLOAD_SIZE{64U};

    char memory[4096U];
    iox::BumpAllocator allocator{memory, 4096U};
    MemPool mempool{sizeof(ChunkHeader) + USER_PAYLOAD_SIZE, 10U, allocator, allocator};
    MemPool chunkMgmtPool{64U, 10U, allocator, allocator};

    void* memoryChunk{mempool.getChunk()};
    ChunkManagement* chunkManagement = GetChunkManagement(memoryChunk);
    SharedChunk chunk{chunkManagement};

    void* memoryChunk2{mempool.getChunk()};
    ChunkManagement* chunkManagement2 = GetChunkManagement(memoryChunk2);
    SharedChunk chunk2{chunkManagement2};

    void* memoryChunk3{mempool.getChunk()};
    ChunkManagement* chunkManagement3 = GetChunkManagement(memoryChunk3);
    SharedChunk chunk3{chunkManagement3};

    void* memoryChunk4{mempool.getChunk()};
    ChunkManagement* chunkManagement4 = GetChunkManagement(memoryChunk4);
    SharedChunk chunk4{chunkManagement4};

    SharedPointer<int> sut = SharedPointer<int>::create(chunk, 42).value();
    SharedPointer<TestClass> sutComplex = SharedPointer<TestClass>::create(chunk2, 1337, 851).value();
};

SharedPointer_Test::counter_t SharedPointer_Test::TestClass::counter;

TEST_F(SharedPointer_Test, DefaultCTor)
{
    ::testing::Test::RecordProperty("TEST_ID", "035f3c9f-4b42-4a67-baae-a4347cd482cc");
    EXPECT_THAT(SharedPointer_Test::TestClass::counter.ctor, Eq(1));
}

TEST_F(SharedPointer_Test, ConstGetMethod)
{
    ::testing::Test::RecordProperty("TEST_ID", "c97a8b0c-bbf0-4ad1-8bb8-4c85c806478a");
    EXPECT_THAT(*const_cast<const SharedPointer<int>&>(sut).get(), Eq(42));
}

TEST_F(SharedPointer_Test, GetMethod)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b3069e1-3ded-46b4-b1cd-bfc44098adcb");
    *sut.get() = 7781;
    EXPECT_THAT(*sut.get(), Eq(7781));
}

TEST_F(SharedPointer_Test, ConstArrowOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "d44bb68e-d87f-4996-a20f-7ee5602c184a");
    EXPECT_THAT(const_cast<const SharedPointer<TestClass>&>(sutComplex)->a, Eq(1337));
}

TEST_F(SharedPointer_Test, ArrowOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "17ca6efe-ba9e-4fed-af2d-676939873250");
    sutComplex->Increase();
    EXPECT_THAT(sutComplex->a, Eq(1349));
}

TEST_F(SharedPointer_Test, ConstStarOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "587cc118-094c-4390-8fce-66ae3a95e93d");
    EXPECT_THAT((*const_cast<const SharedPointer<TestClass>&>(sutComplex)).b, Eq(851));
}

TEST_F(SharedPointer_Test, StarOperator)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec938d1e-dcef-40fb-b1e8-f56e7265c014");
    (*sut)++;
    EXPECT_THAT(*sut, Eq(43));
}

TEST_F(SharedPointer_Test, CopyConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "0e2ed79c-d4a5-4702-9aa1-240f205274e6");
    {
        auto sut3 = SharedPointer<TestClass>::create(chunk3, 313, 1313).value();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2)); // sutComplex is 1
        {
            SharedPointer<TestClass> sut4(sut3);
            EXPECT_THAT(sut4->a, Eq(313));
            EXPECT_THAT(sut4->b, Eq(1313));

            EXPECT_THAT(TestClass::counter.ctor, Eq(2));
            EXPECT_THAT(TestClass::counter.dtor, Eq(0));
            EXPECT_THAT(TestClass::counter.moveCtor, Eq(0));
            EXPECT_THAT(TestClass::counter.copyCtor, Eq(0));
            EXPECT_THAT(TestClass::counter.moveAssignment, Eq(0));
            EXPECT_THAT(TestClass::counter.copyAssignment, Eq(0));
        }
        EXPECT_THAT(TestClass::counter.dtor, Eq(0));
    }
    EXPECT_THAT(TestClass::counter.dtor, Eq(1));
}

TEST_F(SharedPointer_Test, MoveConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "183d0ce3-a078-4bab-95f5-87b0da1e9842");
    {
        auto sut3 = SharedPointer<TestClass>::create(chunk3, 15, 25).value();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2)); // sutComplex is 1
        {
            SharedPointer<TestClass> sut4(std::move(sut3));
            EXPECT_THAT(sut4->a, Eq(15));
            EXPECT_THAT(sut4->b, Eq(25));

            EXPECT_THAT(TestClass::counter.ctor, Eq(2));
            EXPECT_THAT(TestClass::counter.dtor, Eq(0));
            EXPECT_THAT(TestClass::counter.moveCtor, Eq(0));
            EXPECT_THAT(TestClass::counter.copyCtor, Eq(0));
            EXPECT_THAT(TestClass::counter.moveAssignment, Eq(0));
            EXPECT_THAT(TestClass::counter.copyAssignment, Eq(0));
        }
        EXPECT_THAT(TestClass::counter.dtor, Eq(1));
    }
    EXPECT_THAT(TestClass::counter.dtor, Eq(1));
}

TEST_F(SharedPointer_Test, CopyAssignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "fafcefb2-b599-4862-bf56-6ad6d593d1a5");
    {
        auto sut3 = SharedPointer<TestClass>::create(chunk3, 1, 2).value();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2)); // sutComplex is 1

        auto sut4 = SharedPointer<TestClass>::create(chunk4, 3, 4).value();
        EXPECT_THAT(TestClass::counter.ctor, Eq(3));

        EXPECT_THAT(TestClass::counter.dtor, Eq(0));
        sut4 = sut3;
        EXPECT_THAT(TestClass::counter.dtor, Eq(1));
        EXPECT_THAT(sut4->a, Eq(1));
        EXPECT_THAT(sut4->b, Eq(2));
    }
    EXPECT_THAT(TestClass::counter.dtor, Eq(2));
}

TEST_F(SharedPointer_Test, MoveAssignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "3be4dd7a-66cf-458c-9b9c-55459b02e75c");
    {
        auto sut3 = SharedPointer<TestClass>::create(chunk3, 1, 2).value();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2)); // sutComplex is 1

        auto sut4 = SharedPointer<TestClass>::create(chunk4, 3, 4).value();
        EXPECT_THAT(TestClass::counter.ctor, Eq(3));

        EXPECT_THAT(TestClass::counter.dtor, Eq(0));
        sut4 = std::move(sut3);
        EXPECT_THAT(TestClass::counter.dtor, Eq(1));

        EXPECT_THAT(sut4->a, Eq(1));
        EXPECT_THAT(sut4->b, Eq(2));
    }
    EXPECT_THAT(TestClass::counter.dtor, Eq(2));
}

TEST_F(SharedPointer_Test, CopyToEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "94ab4d3f-1faf-4be8-bd04-b78c7f4850a8");
    {
        auto sut3 = SharedPointer<TestClass>::create(chunk3, 1, 2).value();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2)); // sutComplex is 1

        auto sut4 = SharedPointer<TestClass>();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2));

        EXPECT_THAT(TestClass::counter.dtor, Eq(0));
        sut4 = sut3;
        EXPECT_THAT(TestClass::counter.dtor, Eq(0));
        EXPECT_THAT(sut4->a, Eq(1));
        EXPECT_THAT(sut4->b, Eq(2));
    }
    EXPECT_THAT(TestClass::counter.dtor, Eq(1));
}

TEST_F(SharedPointer_Test, CopyFromEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "ee9dddea-0c80-44df-9dd0-5c9d4d4116ae");
    {
        auto sut3 = SharedPointer<TestClass>::create(chunk3, 1, 2).value();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2)); // sutComplex is 1

        auto sut4 = SharedPointer<TestClass>();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2));

        EXPECT_THAT(TestClass::counter.dtor, Eq(0));
        sut3 = sut4;
        EXPECT_THAT(TestClass::counter.dtor, Eq(1));
    }
    EXPECT_THAT(TestClass::counter.dtor, Eq(1));
}

TEST_F(SharedPointer_Test, MoveToEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "fae2118e-4a8d-4b59-bfcb-6819c5728e0c");
    {
        auto sut3 = SharedPointer<TestClass>::create(chunk3, 1, 2).value();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2)); // sutComplex is 1

        auto sut4 = SharedPointer<TestClass>();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2));

        EXPECT_THAT(TestClass::counter.dtor, Eq(0));
        sut4 = std::move(sut3);
        EXPECT_THAT(TestClass::counter.dtor, Eq(0));
        EXPECT_THAT(sut4->a, Eq(1));
        EXPECT_THAT(sut4->b, Eq(2));
    }
    EXPECT_THAT(TestClass::counter.dtor, Eq(1));
}

TEST_F(SharedPointer_Test, MoveFromEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "d9b1a79c-a949-4532-a86b-ed49e7b5a0bb");
    {
        auto sut3 = SharedPointer<TestClass>::create(chunk3, 1, 2).value();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2)); // sutComplex is 1

        auto sut4 = SharedPointer<TestClass>();
        EXPECT_THAT(TestClass::counter.ctor, Eq(2));

        EXPECT_THAT(TestClass::counter.dtor, Eq(0));
        sut3 = std::move(sut4);
        EXPECT_THAT(TestClass::counter.dtor, Eq(1));
    }
    EXPECT_THAT(TestClass::counter.dtor, Eq(1));
}

TEST_F(SharedPointer_Test, DefaultCTorProvidesInvalidSharedPointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1183195-6829-48d9-96fd-caddaa7e9155");
    EXPECT_THAT(static_cast<bool>(SharedPointer<int>()), Eq(false));
}

TEST_F(SharedPointer_Test, SharedPointerWithContentIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "df031cdb-2cbe-4ef9-84ae-e177594e1941");
    auto sut3 = SharedPointer<TestClass>::create(chunk3, 1, 2).value();
    EXPECT_THAT(static_cast<bool>(sut3), Eq(true));
}

} // namespace
