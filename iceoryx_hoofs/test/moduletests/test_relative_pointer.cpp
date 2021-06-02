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

#include "iceoryx_hoofs/internal/relocatable_pointer/relative_pointer.hpp"

#include "test.hpp"

#include <cstdint>
#include <cstring>

namespace
{
using namespace ::testing;
using namespace iox::rp;

struct Data
{
    Data(uint32_t i, uint32_t j)
        : Data1(i)
        , Data2(j)
    {
    }
    uint32_t Data1 = 27;
    uint32_t Data2 = 72;
};

static constexpr uint64_t SHARED_MEMORY_SIZE = 4096 * 32;
static constexpr uint64_t NUMBER_OF_MEMORY_PARTITIONS = 2U;
static uint8_t memoryPatternValue = 1U;

template <typename T>
class base_relative_ptr_test : public Test
{
  public:
    void SetUp() override
    {
        internal::CaptureStderr();
        memset(memoryPartition, memoryPatternValue, NUMBER_OF_MEMORY_PARTITIONS * SHARED_MEMORY_SIZE);
        ++memoryPatternValue;
    }

    void TearDown() override
    {
        BaseRelativePointer::unregisterAll();
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    uint8_t memoryPartition[NUMBER_OF_MEMORY_PARTITIONS][SHARED_MEMORY_SIZE];
};

typedef testing::Types<uint8_t, int8_t, double> Types;

/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(base_relative_ptr_test, Types);
#pragma GCC diagnostic pop

TYPED_TEST(base_relative_ptr_test, ConstrTests)
{
    EXPECT_EQ(BaseRelativePointer::registerPtr(1, this->memoryPartition[0], SHARED_MEMORY_SIZE), true);
    EXPECT_EQ(BaseRelativePointer::registerPtr(2, this->memoryPartition[1], SHARED_MEMORY_SIZE), true);

    {
        auto offset = SHARED_MEMORY_SIZE / 2;
        void* adr = this->memoryPartition[0] + offset;
        RelativePointer<TypeParam> rp;
        rp = adr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        RelativePointer<TypeParam> rp(this->memoryPartition[0]);
        EXPECT_EQ(rp.getOffset(), 0);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE / 2;
        void* adr = this->memoryPartition[0] + offset;
        RelativePointer<TypeParam> rp(adr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE - 1;
        void* adr = this->memoryPartition[0] + offset;
        RelativePointer<TypeParam> rp(adr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        RelativePointer<TypeParam> rp(this->memoryPartition[1]);
        EXPECT_EQ(rp.getOffset(), 0);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE / 2;
        void* adr = this->memoryPartition[1] + offset;
        RelativePointer<TypeParam> rp(adr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE - 1;
        void* adr = this->memoryPartition[1] + offset;
        RelativePointer<TypeParam> rp(adr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        RelativePointer<TypeParam> rp(nullptr);
        EXPECT_EQ(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE + 1;
        void* adr = static_cast<uint8_t*>(this->memoryPartition[1]) + offset;
        RelativePointer<TypeParam> rp(adr);
        EXPECT_NE(rp, nullptr);
    }
}

TYPED_TEST(base_relative_ptr_test, AssignmentOperatorTests)
{
    EXPECT_EQ(BaseRelativePointer::registerPtr(1, this->memoryPartition[0], SHARED_MEMORY_SIZE), true);
    EXPECT_EQ(BaseRelativePointer::registerPtr(2, this->memoryPartition[1], SHARED_MEMORY_SIZE), true);

    {
        RelativePointer<TypeParam> rp;
        rp = this->memoryPartition[0];
        EXPECT_EQ(rp.getOffset(), 0);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        RelativePointer<TypeParam> rp;
        rp = this->memoryPartition[0];
        BaseRelativePointer basePointer(rp);
        RelativePointer<TypeParam> recovered(basePointer);

        EXPECT_EQ(rp, recovered);
        EXPECT_EQ(rp.getOffset(), recovered.getOffset());
        EXPECT_EQ(rp.getId(), recovered.getId());

        recovered = basePointer;
        EXPECT_EQ(rp, recovered);
        EXPECT_EQ(rp.getOffset(), recovered.getOffset());
        EXPECT_EQ(rp.getId(), recovered.getId());
    }

    {
        auto offset = SHARED_MEMORY_SIZE / 2;
        void* adr = this->memoryPartition[0] + offset;
        RelativePointer<TypeParam> rp;
        rp = adr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE - 1;
        void* adr = this->memoryPartition[0] + offset;
        RelativePointer<TypeParam> rp;
        rp = adr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        RelativePointer<TypeParam> rp;
        rp = this->memoryPartition[1];
        EXPECT_EQ(rp.getOffset(), 0);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE / 2;
        void* adr = this->memoryPartition[1] + offset;
        RelativePointer<TypeParam> rp;
        rp = adr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE - 1;
        void* adr = this->memoryPartition[1] + offset;
        RelativePointer<TypeParam> rp;
        rp = adr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        RelativePointer<TypeParam> rp;
        rp = nullptr;
        EXPECT_EQ(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE + 1;
        void* adr = static_cast<uint8_t*>(this->memoryPartition[1]) + offset;
        RelativePointer<TypeParam> rp;
        rp = adr;
        EXPECT_NE(rp, nullptr);
    }
}

TYPED_TEST(base_relative_ptr_test, IdAndOffset)
{
    void* basePtr1 = this->memoryPartition[0];

    RelativePointer<TypeParam> rp1(this->memoryPartition[0], 1);
    EXPECT_EQ(rp1.registerPtr(1, this->memoryPartition[0]), true);
    EXPECT_EQ(rp1.getOffset(), reinterpret_cast<std::ptrdiff_t>(basePtr1));
    EXPECT_EQ(rp1.getId(), 1);

    int offset = SHARED_MEMORY_SIZE / 2;
    auto addressAtOffset = reinterpret_cast<TypeParam*>(this->memoryPartition[0] + offset);
    RelativePointer<TypeParam> rp2(addressAtOffset, 1);
    EXPECT_EQ(rp2.getOffset(), offset);
    EXPECT_EQ(rp2.getId(), 1);
    EXPECT_EQ(rp2.get(), addressAtOffset);
}

TYPED_TEST(base_relative_ptr_test, getOffset)
{
    RelativePointer<TypeParam> rp1(this->memoryPartition[0], 1);
    EXPECT_EQ(rp1.registerPtr(1, this->memoryPartition[0]), true);
    EXPECT_EQ(BaseRelativePointer::getOffset(1, this->memoryPartition[0]), 0);

    int offset = SHARED_MEMORY_SIZE / 2;
    auto addressAtOffset = reinterpret_cast<TypeParam*>(this->memoryPartition[0] + offset);
    RelativePointer<TypeParam> rp2(addressAtOffset, 1);
    EXPECT_EQ(BaseRelativePointer::getOffset(1, addressAtOffset), offset);
}

TYPED_TEST(base_relative_ptr_test, getPtr)
{
    RelativePointer<TypeParam> rp1(this->memoryPartition[0], 1);
    EXPECT_EQ(rp1.registerPtr(1, this->memoryPartition[0]), true);
    EXPECT_EQ(BaseRelativePointer::getPtr(1, 0), this->memoryPartition[0]);

    int offset = SHARED_MEMORY_SIZE / 2;
    auto addressAtOffset = reinterpret_cast<TypeParam*>(this->memoryPartition[0] + offset);
    RelativePointer<TypeParam> rp2(addressAtOffset, 1);
    EXPECT_EQ(BaseRelativePointer::getPtr(1, offset), addressAtOffset);
}

TYPED_TEST(base_relative_ptr_test, registerPtr)
{
    RelativePointer<TypeParam> rp1(this->memoryPartition[0], 1);

    EXPECT_EQ(rp1.registerPtr(1, this->memoryPartition[0]), true);
    EXPECT_EQ(rp1.registerPtr(1, this->memoryPartition[0]), false);
    EXPECT_EQ(rp1.unregisterPtr(1), true);
    EXPECT_EQ(rp1.registerPtr(1, this->memoryPartition[0]), true);
}

TYPED_TEST(base_relative_ptr_test, unRegisterPointerTest_Valid)
{
    RelativePointer<TypeParam> rp1(this->memoryPartition[0], 1);

    rp1.registerPtr(1, this->memoryPartition[0]);
    EXPECT_EQ(rp1.unregisterPtr(1), true);
    EXPECT_EQ(rp1.registerPtr(1, this->memoryPartition[0]), true);
}

TYPED_TEST(base_relative_ptr_test, unregisterPointerAll)
{
    RelativePointer<TypeParam> rp1(this->memoryPartition[0], 1);
    RelativePointer<TypeParam> rp2(this->memoryPartition[1], 9999);

    EXPECT_EQ(rp1.registerPtr(1, this->memoryPartition[0]), true);
    EXPECT_EQ(rp2.registerPtr(9999, this->memoryPartition[1]), true);
    BaseRelativePointer::unregisterAll();
    EXPECT_EQ(rp1.registerPtr(1, this->memoryPartition[0]), true);
    EXPECT_EQ(rp2.registerPtr(9999, this->memoryPartition[1]), true);
}

TYPED_TEST(base_relative_ptr_test, registerPtrWithId)
{
    RelativePointer<TypeParam> rp1(this->memoryPartition[0], 1);
    RelativePointer<TypeParam> rp2(this->memoryPartition[1], 10000);

    EXPECT_EQ(rp1.registerPtr(1, this->memoryPartition[0]), true);
    EXPECT_EQ(rp2.registerPtr(10000, this->memoryPartition[1]), false);
}

TYPED_TEST(base_relative_ptr_test, basePointerValid)
{
    void* basePtr1 = this->memoryPartition[0];

    RelativePointer<TypeParam> rp1(this->memoryPartition[0], 1);
    EXPECT_EQ(rp1.getBasePtr(1), nullptr);
    rp1.registerPtr(1, this->memoryPartition[0]);
    EXPECT_EQ(basePtr1, rp1.getBasePtr(1));
}

TYPED_TEST(base_relative_ptr_test, assignmentOperator)
{
    RelativePointer<TypeParam> rp1(this->memoryPartition[0], 1);
    RelativePointer<TypeParam> rp2 = rp1;

    EXPECT_EQ(rp1.getBasePtr(), rp2.getBasePtr());
    EXPECT_EQ(rp1.getId(), rp2.getId());
    EXPECT_EQ(rp1.getOffset(), rp2.getOffset());
}

TYPED_TEST(base_relative_ptr_test, pointerOperator)
{
    auto baseAddr = reinterpret_cast<TypeParam*>(this->memoryPartition[0]);
    *baseAddr = static_cast<TypeParam>(88);
    RelativePointer<TypeParam> rp1(this->memoryPartition[0], 1);

    EXPECT_EQ(*rp1, *baseAddr);
    *baseAddr = static_cast<TypeParam>(99);
    EXPECT_EQ(*rp1, *baseAddr);
}

/// central use case of the relative pointer:
/// it is tested that changing the (static) lookup table of a relative pointer causes existing
/// relative pointers point to changed locations relative to the new lookup table
TYPED_TEST(base_relative_ptr_test, memoryRemapping)
{
    constexpr size_t BLOCK_SIZE = 1024;
    // simulate 3 consecutive memory blocks on the stack
    uint8_t block1[BLOCK_SIZE];
    uint8_t block2[BLOCK_SIZE];

    uint8_t* base1 = block1;
    uint8_t* base2 = block2;

    // uint8 write
    *base1 = 37U;
    *base2 = 73U;

    EXPECT_EQ(*base1, 37U);
    EXPECT_EQ(*base2, 73U);

    int offset = BLOCK_SIZE / 2;
    auto adr1 = reinterpret_cast<int*>(base1 + offset);
    auto adr2 = reinterpret_cast<int*>(base2 + offset);

    // int write
    *adr1 = 12;
    *adr2 = 21;

    EXPECT_EQ(*adr1, 12);
    EXPECT_EQ(*adr2, 21);

    EXPECT_EQ(BaseRelativePointer::registerPtr(1, base1), true);
    EXPECT_EQ(BaseRelativePointer::registerPtr(2, base2), true);

    {
        // the relative pointers point to base 1 and base 2l
        RelativePointer<uint8_t> rp1(base1, 1);
        RelativePointer<uint8_t> rp2(base2, 2);

        EXPECT_EQ(rp1.getId(), 1);
        EXPECT_EQ(rp2.getId(), 2);

        EXPECT_EQ(rp1.getOffset(), 0);
        EXPECT_EQ(rp2.getOffset(), 0);

        EXPECT_EQ(*rp1, 37U);
        EXPECT_EQ(*rp2, 73U);
    }

    {
        // now test with a type that is larger than 1 byte
        RelativePointer<int> rp1(adr1, 1);
        RelativePointer<int> rp2(adr2, 2);

        EXPECT_EQ(rp1.getId(), 1);
        EXPECT_EQ(rp2.getId(), 2);

        // relative to their respective memory block base adress both have the same offset
        EXPECT_EQ(rp1.getOffset(), offset);
        EXPECT_EQ(rp2.getOffset(), offset);

        //*** rp1 points to 12 and rp2 to 21
        EXPECT_EQ(*rp1, 12);
        EXPECT_EQ(*rp2, 21);

        // simulate a remapping, index 1 now refers to base 2 and vice versa ...
        EXPECT_EQ(BaseRelativePointer::unregisterPtr(1), true);
        EXPECT_EQ(BaseRelativePointer::unregisterPtr(2), true);

        EXPECT_EQ(BaseRelativePointer::registerPtr(1, base2), true);
        EXPECT_EQ(BaseRelativePointer::registerPtr(2, base1), true);

        // which, despite the relative pointer objects not having changed themselves,
        // leads to them referencing the respective other value now (compared to *** above)
        EXPECT_EQ(*rp1, 21);
        EXPECT_EQ(*rp2, 12);

        // this would also happen in another application where the static base pointer lookup table
        // is might differ from application to application
    }
}

TYPED_TEST(base_relative_ptr_test, compileTest)
{
    // No functional test. Tests if code compiles
    RelativePointer<TypeParam> p1;
    RelativePointer<const TypeParam> p2;
}

} // namespace
