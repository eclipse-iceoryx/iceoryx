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

#include "test.hpp"

#include "iceoryx_hoofs/containers/uninitialized_array.hpp"

namespace
{
using namespace ::testing;

using iox::containers::UninitializedArray;

template <typename T>
class UninitializedArrayTest : public ::testing::Test
{
  public:
    using Buffer = T;
    Buffer buffer{};

    void fillBuffer(int startValue)
    {
        auto capacity = buffer.capacity();

        int value = startValue;
        for (uint64_t i = 0; i < capacity; ++i)
        {
            buffer[i] = value++;
        }
    }
};

struct Integer
{
    // NOLINTNEXTLINE(hicpp-explicit-conversions) required for typed tests
    Integer(int value = 0)
        : value(value)
    {
    }

    int value{0};

    // so that it behaves like an int for comparison purposes
    // NOLINTNEXTLINE(hicpp-explicit-conversions) required for typed tests
    operator int() const
    {
        return value;
    }
};

TEST(UninitializedArrayTest, capacityIsCorrect)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ac31a08-77b2-4fd2-a214-81298cada00c");
    constexpr uint64_t capacity = 7;
    // NOLINTNEXTLINE(hicpp-member-init, cppcoreguidelines-pro-type-member-init) false positive
    UninitializedArray<int, capacity> sut;
    EXPECT_EQ(sut.capacity(), capacity);
}

typedef ::testing::Types<UninitializedArray<int, 10>,
                         UninitializedArray<Integer, 10>,
                         UninitializedArray<int, 10, iox::containers::FirstElementZeroed>,
                         UninitializedArray<Integer, 10, iox::containers::FirstElementZeroed>>
    TestArrays;

TYPED_TEST_SUITE(UninitializedArrayTest, TestArrays, );

TYPED_TEST(UninitializedArrayTest, accessElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8621711-9c4c-45b5-979d-404357b664a4");
    auto& buffer = this->buffer;
    auto capacity = buffer.capacity();

    int startValue = 73;
    this->fillBuffer(startValue);

    int expected = startValue;
    for (uint64_t i = 0; i < capacity; ++i)
    {
        EXPECT_EQ(buffer[i], expected++);
    }
}

TYPED_TEST(UninitializedArrayTest, accessElementsOfConstUinitializedArray)
{
    ::testing::Test::RecordProperty("TEST_ID", "50575bac-cff9-4896-89da-b03753370b18");
    auto& buffer = this->buffer;
    auto capacity = buffer.capacity();

    int startValue = 37;
    this->fillBuffer(startValue);

    const auto& constBuffer = this->buffer;
    int expected = startValue;
    for (uint64_t i = 0; i < capacity; ++i)
    {
        EXPECT_EQ(constBuffer[i], expected++);
    }
}

TEST(UninitializedArray, FirstElementIsInitializedWithZeroWhenBufferSetToFirstElementZeroed)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb213516-ab37-43e3-b2ec-098c98d777d1");
    UninitializedArray<uint32_t, 2, iox::containers::FirstElementZeroed> buffer;
    EXPECT_THAT(buffer[0], Eq(0));
}

TYPED_TEST(UninitializedArrayTest, BeginReturnsIteratorToBeginningOfUninitializedArray)
{
    ::testing::Test::RecordProperty("TEST_ID", "6434e308-e24f-41e1-a1e1-949da01b2cbb");
    auto& buffer = this->buffer;
    EXPECT_THAT(buffer.begin(), Eq(&buffer[0]));
}

TYPED_TEST(UninitializedArrayTest, ConstBeginReturnsIteratorToBeginningOfUninitializedArray)
{
    ::testing::Test::RecordProperty("TEST_ID", "7387b043-db44-47ac-a2da-c40040bb9baa");
    auto& buffer = this->buffer;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) reuse of buffer to test const method
    EXPECT_THAT(const_cast<const decltype(buffer)>(buffer).begin(), Eq(&buffer[0]));
}

TYPED_TEST(UninitializedArrayTest, EndReturnsIteratorToEndOfUninitializedArray)
{
    ::testing::Test::RecordProperty("TEST_ID", "52447fba-0c7f-40df-8b7f-64d8b3ffcc49");
    auto& buffer = this->buffer;
    EXPECT_THAT(buffer.end(), Eq(&buffer[buffer.capacity()]));
}

TYPED_TEST(UninitializedArrayTest, ConstEndReturnsIteratorToEndOfUninitializedArray)
{
    ::testing::Test::RecordProperty("TEST_ID", "2946ad83-b782-4c54-966b-c94b482335cc");
    auto& buffer = this->buffer;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) reuse of buffer to test const method
    EXPECT_THAT(const_cast<const decltype(buffer)>(buffer).end(), Eq(&buffer[buffer.capacity()]));
}

TYPED_TEST(UninitializedArrayTest, BeginIteratorComesBeforeEndIteratorWhenNotEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a180d15-7e77-4234-ad88-04673cbf9fc9");
    auto& buffer = this->buffer;
    buffer[0] = 1;
    EXPECT_THAT(buffer.begin(), Lt(buffer.end()));
}

TYPED_TEST(UninitializedArrayTest, BeginConstIteratorComesBeforeEndConstIteratorWhenNotEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "33e3d5b4-4762-421a-829f-455fe44b8e3b");
    auto& buffer = this->buffer;
    buffer[0] = 2;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) reuse of buffer to test const methods
    EXPECT_THAT(const_cast<const decltype(buffer)>(buffer).begin(),
                Lt(const_cast<const decltype(buffer)>(buffer).end()));
}

TYPED_TEST(UninitializedArrayTest, BeginIteratorComesBeforeEndIteratorWhenFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ac41459-1055-47c0-8dc1-ea43c50827bf");
    auto& buffer = this->buffer;
    this->fillBuffer(0);
    EXPECT_THAT(buffer.begin(), Lt(buffer.end()));
}

TYPED_TEST(UninitializedArrayTest, BeginConstIteratorComesBeforeEndConstIteratorWhenFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "01a5d1cd-ba6d-422e-a807-1ffe5787f4af");
    auto& buffer = this->buffer;
    this->fillBuffer(2);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) reuse of buffer to test const methods
    EXPECT_THAT(const_cast<const decltype(buffer)>(buffer).begin(),
                Lt(const_cast<const decltype(buffer)>(buffer).end()));
}

TEST(UninitializedArrayTest, IteratorIteratesThroughNonEmptyUninitializedArray)
{
    ::testing::Test::RecordProperty("TEST_ID", "b42d93c9-cbe8-481f-8a0b-5b3fb8e9020c");
    constexpr uint64_t CAPACITY = 3;
    // NOLINTNEXTLINE(hicpp-member-init, cppcoreguidelines-pro-type-member-init) false positive
    UninitializedArray<uint32_t, CAPACITY> buffer;
    constexpr uint64_t INITIAL_VALUE = 42U;
    buffer[0] = INITIAL_VALUE;
    buffer[1] = INITIAL_VALUE + 1;
    buffer[2] = INITIAL_VALUE + 2;

    uint64_t count{0};
    for (auto& e : buffer)
    {
        EXPECT_THAT(e, Eq(INITIAL_VALUE + count));
        ++count;
    }
    EXPECT_THAT(count, Eq(CAPACITY));
}

TEST(UninitializedArrayTest, ConstIteratorIteratesThroughNonEmptyUninitializedArray)
{
    ::testing::Test::RecordProperty("TEST_ID", "e8d7ac7f-9ec7-4264-8b27-d0469b167375");
    constexpr uint64_t CAPACITY = 3;
    // NOLINTNEXTLINE(hicpp-member-init, cppcoreguidelines-pro-type-member-init) false positive
    UninitializedArray<uint32_t, CAPACITY> buffer;
    constexpr uint64_t INITIAL_VALUE = 13U;
    buffer[0] = INITIAL_VALUE;
    buffer[1] = INITIAL_VALUE + 1;
    buffer[2] = INITIAL_VALUE + 2;

    uint64_t count{0};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) reuse of buffer to test const methods
    for (const auto& e : *const_cast<const decltype(buffer)*>(&buffer))
    {
        EXPECT_THAT(e, Eq(INITIAL_VALUE + count));
        ++count;
    }
    EXPECT_THAT(count, Eq(CAPACITY));
}
} // namespace
