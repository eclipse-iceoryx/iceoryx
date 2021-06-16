// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/cxx/static_storage.hpp"
#include "test.hpp"

#include <iostream>

using namespace ::testing;
using namespace iox::cxx;


// note we cannot enforce size and alignment at the same time,
// only minimum size and alignment
// i.e. using Bytes = alignas(Align) uint8_t[Size];
// will not be aligned, alignas is ignored) so we have to use a struct

// the actual size will be some multiple of a power of 2 larger than MinSize
// decided by the compiler
template <uint32_t Size, uint32_t Align = 1U>
struct alignas(Align) Bytes
{
    uint8_t data[Size] = {};

    void set(uint8_t value)
    {
        memset(data, value, Size);
    }

    bool hasValue(uint8_t value)
    {
        for (uint32_t i = 0; i < Size; ++i)
        {
            if (data[i] != value)
            {
                return false;
            }
        }
        return true;
    }
};

namespace
{

TEST(static_storage_test, CapacityIsConsistent)
{
    constexpr uint64_t CAPACITY = 16;
    EXPECT_EQ(static_storage<CAPACITY>::capacity(), CAPACITY);
}

TEST(static_storage_test, IsAllocatableWithoutAlignmentRestriction)
{
    using Data = Bytes<16, 1>;
    EXPECT_EQ(sizeof(Data), 16);
    EXPECT_EQ(alignof(Data), 1);
    EXPECT_TRUE(static_storage<16>::is_allocatable<Data>());
}

TEST(static_storage_test, IsNotAllocatableWithoutAlignmentRestriction)
{
    using Data = Bytes<16, 1>;
    EXPECT_FALSE(static_storage<15>::is_allocatable<Data>());
}

TEST(static_storage_test, IsAllocatableWithAlignmentRestriction)
{
    using Data = Bytes<16, 4>;
    EXPECT_EQ(sizeof(Data), 16);
    EXPECT_EQ(alignof(Data), 4);
    EXPECT_TRUE(static_storage<19>::is_allocatable<Data>());
}

TEST(static_storage_test, IsNotAllocatableWithAlignmentRestriction)
{
    using Data = Bytes<16, 4>;
    EXPECT_FALSE(static_storage<18>::is_allocatable<Data>());
}

TEST(static_storage_test, IsAllocatableWithDifferentAlignment)
{
    using Data = Bytes<16, 4>;
    const bool result = static_storage<18, 2>::is_allocatable<Data>();
    EXPECT_TRUE(result);
}

TEST(static_storage_test, IsNotAllocatableWithDifferentAlignment)
{
    using Data = Bytes<16, 4>;
    const bool result = static_storage<17, 2>::is_allocatable<Data>();
    EXPECT_FALSE(result);
}

TEST(static_storage_test, AllocateSucceedsIfSizeIsSufficient)
{
    static_storage<18, 2> sut;
    EXPECT_NE(sut.allocate(16, 4), nullptr);
}

TEST(static_storage_test, AllocateFailsIfSizeIsInsufficient)
{
    static_storage<17, 2> sut;
    EXPECT_NE(sut.allocate(16, 4), nullptr);
}

TEST(static_storage_test, TypedAllocateSucceedsIfSizeIsSufficient)
{
    using Data = Bytes<16, 4>;
    static_storage<18, 2> sut;
    EXPECT_NE(sut.allocate<Data>(), nullptr);
    // when size of storage is insufficent it will not compile and
    // therefore cannot be tested
}

TEST(static_storage_test, DoubleAllocateFails)
{
    static_storage<18, 2> sut;
    sut.allocate(16, 4);
    EXPECT_EQ(sut.allocate(16, 4), nullptr);
}

TEST(static_storage_test, DoubleTypedAllocateFails)
{
    using Data = Bytes<16, 4>;
    static_storage<18, 2> sut;
    sut.allocate<Data>();
    EXPECT_EQ(sut.allocate<Data>(), nullptr);
}

TEST(static_storage_test, AllocateAfterDeallocateSucceeds)
{
    static_storage<18, 2> sut;
    sut.allocate(16, 4);
    sut.deallocate();
    EXPECT_NE(sut.allocate(16, 4), nullptr);
}

TEST(static_storage_test, TypedAllocateAfterDeallocateSucceeds)
{
    using Data = Bytes<16, 4>;
    static_storage<18, 2> sut;
    sut.allocate<Data>();
    sut.deallocate();
    EXPECT_NE(sut.allocate<Data>(), nullptr);
}

TEST(static_storage_test, ClearSetsStorageBytesToZeroIfThereIsNoObjectStored)
{
    using Data = Bytes<16, 4>;
    static_storage<18, 2> sut;
    auto data = sut.allocate<Data>();
    ASSERT_NE(data, nullptr);
    data->set(37);
    EXPECT_TRUE(data->hasValue(37));

    sut.deallocate();
    EXPECT_TRUE(sut.clear());
    EXPECT_TRUE(data->hasValue(0));
}

TEST(static_storage_test, ClearHasNoEffectIfThereIsAnObjectStored)
{
    using Data = Bytes<16, 4>;
    static_storage<18, 2> sut;
    auto data = sut.allocate<Data>();
    ASSERT_NE(data, nullptr);
    data->set(37);
    EXPECT_TRUE(data->hasValue(37));

    EXPECT_FALSE(sut.clear());
    EXPECT_TRUE(data->hasValue(37));
}

TEST(static_storage_test, AllocationIsAligned)
{
    static_storage<17, 2> sut;
    uintptr_t p = reinterpret_cast<uintptr_t>(sut.allocate(16, 4));
    EXPECT_EQ(p % 4, 0);
}

TEST(static_storage_test, TypedAllocationIsAligned)
{
    using Data = Bytes<4, 8>;
    static_storage<17, 2> sut;
    uintptr_t p = reinterpret_cast<uintptr_t>(sut.allocate<Data>());
    EXPECT_EQ(p % 8, 0);
}

TEST(static_storage_test, AllocationSizeReturnsSizeIfTypeIsAlignedWithStorage)
{
    constexpr uint64_t typeAlign = 2;
    constexpr uint64_t storageAlign = 2 * typeAlign;
    using Data = Bytes<4, typeAlign>;

    constexpr auto size = static_storage<17, storageAlign>::allocation_size<Data>();
    EXPECT_EQ(size, sizeof(Data));
}

TEST(static_storage_test, AllocationSizeReturnsMoreThanSizeIfTypeIsNotAlignedWithStorage)
{
    constexpr uint64_t typeAlign = 16;
    constexpr uint64_t storageAlign = 4;
    using Data = Bytes<4, typeAlign>;

    constexpr auto size = static_storage<17, storageAlign>::allocation_size<Data>();
    EXPECT_EQ(size, sizeof(Data) + typeAlign - storageAlign);
}


} // namespace