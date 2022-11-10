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

#include "iceoryx_hoofs/cxx/span.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "test.hpp"

#include <vector>

namespace
{
using namespace ::testing;
using namespace iox::cxx;

class span_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(span_test, NewSpanCreatedWithTheDefaultConstructor)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a0589e2-b2fb-4f81-8082-6d3ea411e659");
    span<int> dyn_span;
    EXPECT_EQ(nullptr, dyn_span.data());
    EXPECT_EQ(0u, dyn_span.size());

    constexpr span<int, 0> static_span;
    static_assert(nullptr == static_span.data(), "");
    static_assert(0u == static_span.size(), "");
}

TEST_F(span_test, NewSpanFromIterator)
{
    ::testing::Test::RecordProperty("TEST_ID", "92de7ff5-d03e-41ae-bfdd-893892381b24");
    constexpr int* kNull = nullptr;
    constexpr span<int> empty_span(kNull, 0);
    EXPECT_TRUE(empty_span.empty());
    EXPECT_EQ(nullptr, empty_span.data());

    std::vector<int> vector = {1, 1, 2, 3, 5, 8};

    span<int> dyn_span(vector.begin(), vector.size());
    EXPECT_EQ(vector.data(), dyn_span.data());
    EXPECT_EQ(vector.size(), dyn_span.size());

    for (size_t i = 0; i < dyn_span.size(); ++i)
        EXPECT_EQ(vector[i], dyn_span[i]);

    span<int, 6> static_span(vector.begin(), vector.size());
    EXPECT_EQ(vector.data(), static_span.data());
    EXPECT_EQ(vector.size(), static_span.size());

    for (size_t i = 0; i < static_span.size(); ++i)
        EXPECT_EQ(vector[i], static_span[i]);
}

TEST_F(span_test, NewSpanCreatedFromContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "593aa3b6-9937-469d-991d-9e682110727e");
    std::vector<int> vector = {1, 1, 2, 3, 5, 8};

    span<const int> const_span(vector);
    EXPECT_EQ(vector.data(), const_span.data());
    EXPECT_EQ(vector.size(), const_span.size());

    for (size_t i = 0; i < const_span.size(); ++i)
        EXPECT_EQ(vector[i], const_span[i]);

    span<int> dyn_span(vector);
    EXPECT_EQ(vector.data(), dyn_span.data());
    EXPECT_EQ(vector.size(), dyn_span.size());

    for (size_t i = 0; i < dyn_span.size(); ++i)
        EXPECT_EQ(vector[i], dyn_span[i]);

    span<int, 6> static_span(vector.data(), vector.size());
    EXPECT_EQ(vector.data(), static_span.data());
    EXPECT_EQ(vector.size(), static_span.size());

    for (size_t i = 0; i < static_span.size(); ++i)
        EXPECT_EQ(vector[i], static_span[i]);
}

TEST_F(span_test, NewSpanCreatedFromArray)
{
    ::testing::Test::RecordProperty("TEST_ID", "bbbd4ce2-30ea-4b32-86e3-aa7d0a1184d8");
    int array[] = {5, 4, 3, 2, 1};

    span<const int> const_span(array);
    EXPECT_EQ(array, const_span.data());
    EXPECT_EQ(iox::cxx::size(array), const_span.size());
    for (size_t i = 0; i < const_span.size(); ++i)
        EXPECT_EQ(array[i], const_span[i]);

    span<int> dyn_span(array);
    EXPECT_EQ(array, dyn_span.data());
    EXPECT_EQ(iox::cxx::size(array), dyn_span.size());
    for (size_t i = 0; i < dyn_span.size(); ++i)
        EXPECT_EQ(array[i], dyn_span[i]);

    span<int, iox::cxx::size(array)> static_span(array);
    EXPECT_EQ(array, static_span.data());
    EXPECT_EQ(iox::cxx::size(array), static_span.size());
    for (size_t i = 0; i < static_span.size(); ++i)
        EXPECT_EQ(array[i], static_span[i]);
}

TEST_F(span_test, NewSpanCreatedFromConstexprArray)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ec9c31c-b97f-43a3-9669-3bdff3a82b9e");
    static constexpr int arr[] = {5, 4, 3, 2, 1};

    constexpr span<const int> dyn_span(arr);
    static_assert(arr == dyn_span.data(), "");
    static_assert(iox::cxx::size(arr) == dyn_span.size(), "");

    static_assert(arr[0] == dyn_span[0], "");
    static_assert(arr[1] == dyn_span[1], "");
    static_assert(arr[2] == dyn_span[2], "");
    static_assert(arr[3] == dyn_span[3], "");
    static_assert(arr[4] == dyn_span[4], "");

    constexpr span<const int, iox::cxx::size(arr)> static_span(arr);
    static_assert(arr == static_span.data(), "");
    static_assert(iox::cxx::size(arr) == static_span.size(), "");

    static_assert(arr[0] == static_span[0], "");
    static_assert(arr[1] == static_span[1], "");
    static_assert(arr[2] == static_span[2], "");
    static_assert(arr[3] == static_span[3], "");
    static_assert(arr[4] == static_span[4], "");
}

TEST_F(span_test, NewSpanFromConstContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "4358e397-c82b-45f7-a75f-8d0b1cf03667");
    const std::vector<int> vector = {1, 1, 2, 3, 5, 8};

    span<const int> const_span(vector);
    EXPECT_EQ(vector.data(), const_span.data());
    EXPECT_EQ(vector.size(), const_span.size());

    for (size_t i = 0; i < const_span.size(); ++i)
        EXPECT_EQ(vector[i], const_span[i]);

    span<const int, 6> static_span(vector.data(), vector.size());
    EXPECT_EQ(vector.data(), static_span.data());
    EXPECT_EQ(vector.size(), static_span.size());

    for (size_t i = 0; i < static_span.size(); ++i)
        EXPECT_EQ(vector[i], static_span[i]);
}

TEST_F(span_test, NewSpanFromConstIoxCxxVector)
{
    ::testing::Test::RecordProperty("TEST_ID", "a7b1177b-0db5-44b8-bacd-b75d25c3a448");
    constexpr uint64_t CAPACITY{6U};
    constexpr int DEFAULT_VALUE{1};
    iox::cxx::vector<int, CAPACITY> vector(CAPACITY, DEFAULT_VALUE);
    vector[2] = 2;
    vector[3] = 3;
    vector[4] = 5;
    vector[5] = 8;

    span<const int> const_span(vector);
    EXPECT_EQ(vector.data(), const_span.data());
    EXPECT_EQ(vector.size(), const_span.size());

    for (size_t i = 0; i < const_span.size(); ++i)
        EXPECT_EQ(vector[i], const_span[i]);

    span<const int, 6> static_span(vector.data(), vector.size());
    EXPECT_EQ(vector.data(), static_span.data());
    EXPECT_EQ(vector.size(), static_span.size());

    for (size_t i = 0; i < static_span.size(); ++i)
        EXPECT_EQ(vector[i], static_span[i]);
}

TEST_F(span_test, CheckFrontOfSpanIfItReturnsTheElementAtIndex0)
{
    ::testing::Test::RecordProperty("TEST_ID", "57b2f67f-79c1-4c1e-a305-f4665283c474");
    static constexpr int arr[] = {1, 6, 1, 8, 0};
    constexpr span<const int> span(arr);
    static_assert(&arr[0] == &span.front(), "span.front() does not refer to the same element as arr[0]");
}

TEST_F(span_test, CheckConstexprIterOfSpan)
{
    ::testing::Test::RecordProperty("TEST_ID", "8764fcfb-27df-4f39-b8cd-56bf881db382");
    static constexpr int arr[] = {1, 6, 1, 8, 0};
    constexpr span<const int> span(arr);

    static_assert(1 == span.begin()[0], "");
    static_assert(1 == *(span.begin() += 0), "");
    static_assert(6 == *(span.begin() += 1), "");

    static_assert(1 == *((span.begin() + 1) -= 1), "");
    static_assert(6 == *((span.begin() + 1) -= 0), "");
}

TEST_F(span_test, GetSpanDataAsWritableBytes)
{
    ::testing::Test::RecordProperty("TEST_ID", "73ed24f9-c2ea-467a-b64e-e53e97247e8d");
    std::vector<int> vec = {1, 1, 2, 3, 5, 8};
    span<int> mutable_span(vec);
    span<uint8_t> writable_bytes_span = as_writable_bytes(mutable_span);
    EXPECT_EQ(reinterpret_cast<uint8_t*>(vec.data()), writable_bytes_span.data());
    EXPECT_EQ(sizeof(int) * vec.size(), writable_bytes_span.size());
    EXPECT_EQ(writable_bytes_span.size(), writable_bytes_span.size_bytes());

    // Set the first entry of vec to zero while writing through the span.
    std::fill(writable_bytes_span.data(), writable_bytes_span.data() + sizeof(int), 0);
    EXPECT_EQ(0, vec[0]);
}

} // namespace