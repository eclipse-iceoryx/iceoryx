// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/relative_pointer.hpp"
#include "test.hpp"

#include <cstdint>
#include <cstring>

namespace
{
using namespace ::testing;
using namespace iox;

constexpr uint64_t SHARED_MEMORY_SIZE = 4096UL * 32UL;
constexpr uint64_t NUMBER_OF_MEMORY_PARTITIONS = 2U;
uint8_t memoryPatternValue = 1U;

template <typename T>
class RelativePointer_test : public Test
{
  public:
    void SetUp() override
    {
        memset(
            static_cast<void*>(memoryPartition), memoryPatternValue, NUMBER_OF_MEMORY_PARTITIONS * SHARED_MEMORY_SIZE);
        ++memoryPatternValue;
    }

    void TearDown() override
    {
        UntypedRelativePointer::unregisterAll();
    }

    uint8_t* partitionPtr(uint32_t partition)
    {
        // NOLINTJUSTIFICATION Used only for test purposes
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index,
        // cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        return memoryPartition[partition];
        // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index,
        // cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    }

    // NOLINTJUSTIFICATION Used only for test purposes
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    uint8_t memoryPartition[NUMBER_OF_MEMORY_PARTITIONS][SHARED_MEMORY_SIZE]{{0}};
};

typedef testing::Types<uint8_t, int8_t, double> Types;

TYPED_TEST_SUITE(RelativePointer_test, Types, );

// NOLINTJUSTIFICATION Pointer arithmetic needed for tests
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)

/// @todo iox-#1745 the tests should be reworked
TYPED_TEST(RelativePointer_test, ConstrTests)
{
    ::testing::Test::RecordProperty("TEST_ID", "cae7b4d4-86eb-42f6-b938-90a76f01bea5");
    EXPECT_EQ(RelativePointer<TypeParam>::registerPtrWithId(
                  segment_id_t{1U}, reinterpret_cast<TypeParam*>(this->memoryPartition[0]), SHARED_MEMORY_SIZE),
              true);
    EXPECT_EQ(RelativePointer<TypeParam>::registerPtrWithId(
                  segment_id_t{2U}, reinterpret_cast<TypeParam*>(this->memoryPartition[1]), SHARED_MEMORY_SIZE),
              true);

    auto* ptr0 = this->partitionPtr(0U);
    auto* ptr1 = this->partitionPtr(1U);

    {
        auto offset = SHARED_MEMORY_SIZE / 2U;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr0 + offset);

        RelativePointer<TypeParam> rp;
        rp = typedPtr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1U);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr0);
        RelativePointer<TypeParam> rp(typedPtr);
        EXPECT_EQ(rp.getOffset(), 0U);
        EXPECT_EQ(rp.getId(), 1U);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE / 2U;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr0 + offset);
        RelativePointer<TypeParam> rp(typedPtr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1U);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE - 1U;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr0 + offset);
        RelativePointer<TypeParam> rp(typedPtr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1U);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr1);
        RelativePointer<TypeParam> rp(typedPtr);
        EXPECT_EQ(rp.getOffset(), 0U);
        EXPECT_EQ(rp.getId(), 2U);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE / 2U;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr1 + offset);
        RelativePointer<TypeParam> rp(typedPtr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2U);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = SHARED_MEMORY_SIZE - 1U;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr1 + offset);
        RelativePointer<TypeParam> rp(typedPtr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2U);
        EXPECT_NE(rp, nullptr);
    }

    {
        RelativePointer<TypeParam> rp(nullptr);
        EXPECT_FALSE(rp);
    }

    {
        auto offset = SHARED_MEMORY_SIZE + 1U;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr1 + offset);
        RelativePointer<TypeParam> rp(typedPtr);
        EXPECT_TRUE(rp);
    }
}

TYPED_TEST(RelativePointer_test, AssignmentOperatorTests)
{
    ::testing::Test::RecordProperty("TEST_ID", "cd0c4a6a-7779-4dc3-97dc-58ef40a58715");
    auto* ptr0 = this->partitionPtr(0U);
    auto* ptr1 = this->partitionPtr(1U);

    EXPECT_EQ(RelativePointer<TypeParam>::registerPtrWithId(
                  segment_id_t{1U}, reinterpret_cast<TypeParam*>(ptr0), SHARED_MEMORY_SIZE),
              true);
    EXPECT_EQ(RelativePointer<TypeParam>::registerPtrWithId(
                  segment_id_t{2U}, reinterpret_cast<TypeParam*>(ptr1), SHARED_MEMORY_SIZE),
              true);

    {
        RelativePointer<TypeParam> rp;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr0);
        rp = typedPtr;
        EXPECT_EQ(rp.getOffset(), 0U);
        EXPECT_EQ(rp.getId(), 1U);
        EXPECT_TRUE(rp);
    }

    {
        auto offset = SHARED_MEMORY_SIZE / 2U;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr0 + offset);
        RelativePointer<TypeParam> rp;
        rp = typedPtr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1U);
        EXPECT_TRUE(rp);
    }

    {
        auto offset = SHARED_MEMORY_SIZE - 1U;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr0 + offset);
        RelativePointer<TypeParam> rp;
        rp = typedPtr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1U);
        EXPECT_TRUE(rp);
    }

    {
        RelativePointer<TypeParam> rp;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr1);
        rp = typedPtr;
        EXPECT_EQ(rp.getOffset(), 0U);
        EXPECT_EQ(rp.getId(), 2U);
        EXPECT_TRUE(rp);
    }

    {
        auto offset = SHARED_MEMORY_SIZE / 2U;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr1 + offset);
        RelativePointer<TypeParam> rp;
        rp = typedPtr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2U);
        EXPECT_TRUE(rp);
    }

    {
        auto offset = SHARED_MEMORY_SIZE - 1U;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr1 + offset);
        RelativePointer<TypeParam> rp;
        rp = typedPtr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2U);
        EXPECT_TRUE(rp);
    }

    {
        RelativePointer<TypeParam> rp;
        rp = nullptr;
        EXPECT_FALSE(rp);
    }

    {
        auto offset = SHARED_MEMORY_SIZE + 1;
        auto* typedPtr = reinterpret_cast<TypeParam*>(ptr1 + offset);
        RelativePointer<TypeParam> rp;
        rp = typedPtr;
        EXPECT_TRUE(rp);
    }
}

TYPED_TEST(RelativePointer_test, IdAndOffsetAreTranslatedToRawPointerCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "9a29a074-d68d-4431-88b9-bdd26b1a41f7");
    auto* ptr = this->partitionPtr(0U);
    auto* typedPtr = reinterpret_cast<TypeParam*>(ptr);

    RelativePointer<TypeParam> rp1(typedPtr, segment_id_t{1U});
    EXPECT_EQ(rp1.registerPtrWithId(segment_id_t{1U}, typedPtr), true);
    EXPECT_EQ(rp1.getOffset(), reinterpret_cast<std::ptrdiff_t>(typedPtr));
    EXPECT_EQ(rp1.getId(), 1U);

    int offset = SHARED_MEMORY_SIZE / 2U;
    auto* addressAtOffset = reinterpret_cast<TypeParam*>(ptr + offset);
    RelativePointer<TypeParam> rp2(addressAtOffset, segment_id_t{1U});
    EXPECT_EQ(rp2.getOffset(), offset);
    EXPECT_EQ(rp2.getId(), 1U);
    EXPECT_EQ(rp2.get(), addressAtOffset);
}

TYPED_TEST(RelativePointer_test, GetOffsetReturnsCorrectOffset)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b493337-ee55-498a-9cac-8bb5741f72f0");
    auto* ptr = this->partitionPtr(0U);
    auto* typedPtr = reinterpret_cast<TypeParam*>(ptr);

    RelativePointer<TypeParam> rp1(typedPtr, segment_id_t{1U});
    EXPECT_EQ(rp1.registerPtrWithId(segment_id_t{1U}, typedPtr), true);
    EXPECT_EQ(UntypedRelativePointer::getOffset(segment_id_t{1U}, typedPtr), 0U);

    int offset = SHARED_MEMORY_SIZE / 2U;
    auto* addressAtOffset = reinterpret_cast<TypeParam*>(ptr + offset);
    RelativePointer<TypeParam> rp2(addressAtOffset, segment_id_t{1U});
    EXPECT_EQ(UntypedRelativePointer::getOffset(segment_id_t{1U}, addressAtOffset), offset);
}

TYPED_TEST(RelativePointer_test, GetPtrReturnsAddressWithCorrectOffset)
{
    ::testing::Test::RecordProperty("TEST_ID", "4fadf89f-69c0-4058-8995-a98e2e3334b2");
    auto* ptr = this->partitionPtr(0U);
    auto* typedPtr = reinterpret_cast<TypeParam*>(this->partitionPtr(0U));
    RelativePointer<TypeParam> rp1(typedPtr, segment_id_t{1U});
    EXPECT_EQ(rp1.registerPtrWithId(segment_id_t{1U}, typedPtr), true);
    EXPECT_EQ(UntypedRelativePointer::getPtr(segment_id_t{1U}, 0), typedPtr);

    uint64_t offset = SHARED_MEMORY_SIZE / 2U;
    auto* addressAtOffset = reinterpret_cast<TypeParam*>(ptr + offset);
    RelativePointer<TypeParam> rp2(addressAtOffset, segment_id_t{1});
    EXPECT_EQ(UntypedRelativePointer::getPtr(segment_id_t{1U}, offset), addressAtOffset);
}

TYPED_TEST(RelativePointer_test, RegisteringAndUnregisteringRelativePointerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3f08ab46-c778-468a-bab1-ecd71aa800f4");
    auto* typedPtr = reinterpret_cast<TypeParam*>(this->partitionPtr(0U));

    RelativePointer<TypeParam> rp1(typedPtr, segment_id_t{1U});

    EXPECT_EQ(rp1.registerPtrWithId(segment_id_t{1U}, typedPtr), true);
    EXPECT_EQ(rp1.registerPtrWithId(segment_id_t{1U}, typedPtr), false);
    EXPECT_EQ(rp1.unregisterPtr(segment_id_t{1U}), true);
    EXPECT_EQ(rp1.registerPtrWithId(segment_id_t{1U}, typedPtr), true);
}


TYPED_TEST(RelativePointer_test, UnRegisteringOneRelativePointerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "cc09122e-74e8-4d24-83ec-6500471becac");
    auto* ptr = this->partitionPtr(0U);
    auto* typedPtr = reinterpret_cast<TypeParam*>(ptr);

    RelativePointer<TypeParam> rp1(typedPtr, segment_id_t{1U});

    rp1.registerPtrWithId(segment_id_t{1U}, typedPtr);
    EXPECT_EQ(rp1.unregisterPtr(segment_id_t{1U}), true);
    EXPECT_EQ(rp1.registerPtrWithId(segment_id_t{1U}, typedPtr), true);
}

TYPED_TEST(RelativePointer_test, UnregisteringAllRelativePointerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e793b3e8-5077-499d-b628-608ecfd91b9e");
    auto* typedPtr0 = reinterpret_cast<TypeParam*>(this->partitionPtr(0U));
    auto* typedPtr1 = reinterpret_cast<TypeParam*>(this->partitionPtr(1U));

    RelativePointer<TypeParam> rp1(typedPtr0, segment_id_t{1U});
    RelativePointer<TypeParam> rp2(typedPtr1, segment_id_t{9999U});

    EXPECT_EQ(rp1.registerPtrWithId(segment_id_t{1U}, typedPtr0), true);
    EXPECT_EQ(rp2.registerPtrWithId(segment_id_t{9999U}, typedPtr1), true);
    UntypedRelativePointer::unregisterAll();
    EXPECT_EQ(rp1.registerPtrWithId(segment_id_t{1U}, typedPtr0), true);
    EXPECT_EQ(rp2.registerPtrWithId(segment_id_t{9999U}, typedPtr1), true);
}

TYPED_TEST(RelativePointer_test, RegisterPtrWithIdFailsWhenTooLarge)
{
    ::testing::Test::RecordProperty("TEST_ID", "87521383-6aea-4b43-a182-3a21499be710");
    auto* typedPtr0 = reinterpret_cast<TypeParam*>(this->partitionPtr(0U));
    auto* typedPtr1 = reinterpret_cast<TypeParam*>(this->partitionPtr(1U));

    RelativePointer<TypeParam> rp1(typedPtr0, segment_id_t{1U});
    RelativePointer<TypeParam> rp2(typedPtr1, segment_id_t{10000U});

    EXPECT_EQ(rp1.registerPtrWithId(segment_id_t{1U}, typedPtr0), true);
    EXPECT_EQ(rp2.registerPtrWithId(segment_id_t{10000U}, typedPtr1), false);
}

TYPED_TEST(RelativePointer_test, BasePointerIsSameAfterRegistering)
{
    ::testing::Test::RecordProperty("TEST_ID", "40e649bc-b159-45ab-891f-2194a0dcf0e6");
    auto* typedPtr = reinterpret_cast<TypeParam*>(this->partitionPtr(0U));

    RelativePointer<TypeParam> rp1(typedPtr, segment_id_t{1U});
    EXPECT_EQ(rp1.getBasePtr(segment_id_t{1U}), nullptr);
    rp1.registerPtrWithId(segment_id_t{1U}, typedPtr);
    EXPECT_EQ(typedPtr, rp1.getBasePtr(segment_id_t{1U}));
}

TYPED_TEST(RelativePointer_test, AssignmentOperatorResultsInSameBasePointerIdAndOffset)
{
    ::testing::Test::RecordProperty("TEST_ID", "98e2eb78-ee5d-4d87-9753-5ac42b90b9d6");
    auto* typedPtr = reinterpret_cast<TypeParam*>(this->partitionPtr(0U));

    RelativePointer<TypeParam> rp1(typedPtr, segment_id_t{1U});
    // NOLINTJUSTIFICATION Copy needed for tests
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    RelativePointer<TypeParam> rp2 = rp1;

    EXPECT_EQ(rp1.getBasePtr(), rp2.getBasePtr());
    EXPECT_EQ(rp1.getId(), rp2.getId());
    EXPECT_EQ(rp1.getOffset(), rp2.getOffset());
}

TYPED_TEST(RelativePointer_test, DereferencingOperatorResultsInSameValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "d8c1105e-1041-418f-9327-27958f788119");
    auto* typedPtr = reinterpret_cast<TypeParam*>(this->partitionPtr(0U));

    *typedPtr = static_cast<TypeParam>(88);
    RelativePointer<TypeParam> rp1(typedPtr, segment_id_t{1U});

    EXPECT_EQ(*rp1, *typedPtr);
    *typedPtr = static_cast<TypeParam>(99);
    EXPECT_EQ(*rp1, *typedPtr);
}

/// central use case of the relative pointer:
/// it is tested that changing the (static) lookup table of a relative pointer causes existing
/// relative pointers point to changed locations relative to the new lookup table
TYPED_TEST(RelativePointer_test, MemoryRemappingWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "48452388-a7ac-486d-963d-c8d4e5eb55a0");
    constexpr size_t BLOCK_SIZE = 1024;
    // simulate 3 consecutive memory blocks on the stack
    // NOLINTJUSTIFICATION Used only for test purposes
    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    uint8_t block1[BLOCK_SIZE];
    uint8_t block2[BLOCK_SIZE];
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)

    auto* base1 = static_cast<uint8_t*>(block1);
    auto* base2 = static_cast<uint8_t*>(block2);

    // uint8 write
    *base1 = 37U;
    *base2 = 73U;

    EXPECT_EQ(*base1, 37U);
    EXPECT_EQ(*base2, 73U);

    int offset = BLOCK_SIZE / 2U;
    auto* adr1 = reinterpret_cast<int*>(base1 + offset);
    auto* adr2 = reinterpret_cast<int*>(base2 + offset);

    // int write
    *adr1 = 12;
    *adr2 = 21;

    EXPECT_EQ(*adr1, 12);
    EXPECT_EQ(*adr2, 21);

    EXPECT_EQ(UntypedRelativePointer::registerPtrWithId(segment_id_t{1U}, base1), true);
    EXPECT_EQ(UntypedRelativePointer::registerPtrWithId(segment_id_t{2U}, base2), true);

    {
        // the relative pointers point to base 1 and base 2l
        RelativePointer<uint8_t> rp1(base1, segment_id_t{1U});
        RelativePointer<uint8_t> rp2(base2, segment_id_t{2U});

        EXPECT_EQ(rp1.getId(), 1U);
        EXPECT_EQ(rp2.getId(), 2U);

        EXPECT_EQ(rp1.getOffset(), 0);
        EXPECT_EQ(rp2.getOffset(), 0);

        EXPECT_EQ(*rp1, 37U);
        EXPECT_EQ(*rp2, 73U);
    }

    {
        // now test with a type that is larger than 1 byte
        RelativePointer<int> rp1(adr1, segment_id_t{1U});
        RelativePointer<int> rp2(adr2, segment_id_t{2U});

        EXPECT_EQ(rp1.getId(), 1U);
        EXPECT_EQ(rp2.getId(), 2U);

        // relative to their respective memory block base adress both have the same offset
        EXPECT_EQ(rp1.getOffset(), offset);
        EXPECT_EQ(rp2.getOffset(), offset);

        //*** rp1 points to 12 and rp2 to 21
        EXPECT_EQ(*rp1, 12);
        EXPECT_EQ(*rp2, 21);

        // simulate a remapping, index 1 now refers to base 2 and vice versa ...
        EXPECT_EQ(UntypedRelativePointer::unregisterPtr(segment_id_t{1U}), true);
        EXPECT_EQ(UntypedRelativePointer::unregisterPtr(segment_id_t{2U}), true);

        EXPECT_EQ(UntypedRelativePointer::registerPtrWithId(segment_id_t{1}, base2), true);
        EXPECT_EQ(UntypedRelativePointer::registerPtrWithId(segment_id_t{2}, base1), true);

        // which, despite the relative pointer objects not having changed themselves,
        // leads to them referencing the respective other value now (compared to *** above)
        EXPECT_EQ(*rp1, 21);
        EXPECT_EQ(*rp2, 12);

        // this would also happen in another application where the static base pointer lookup table
        // is might differ from application to application
    }
}

TYPED_TEST(RelativePointer_test, DefaultConstructedRelativePtrIsNull)
{
    ::testing::Test::RecordProperty("TEST_ID", "be25f19c-912c-438e-97b1-6fcacb879453");
    RelativePointer<TypeParam> rp1;
    RelativePointer<const TypeParam> rp2;

    EXPECT_FALSE(rp1);
    EXPECT_FALSE(rp2);
}

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-reinterpret-cast)

} // namespace
