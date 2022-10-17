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

typedef ::testing::Types<UninitializedArray<int, 10>, UninitializedArray<Integer, 10>> TestArrays;

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

} // namespace
