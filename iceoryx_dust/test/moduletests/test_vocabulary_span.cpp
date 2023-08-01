// Copyright (c) 2022 - 2023 by Apex.AI Inc. All rights reserved.
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

#include "iox/span.hpp"
#include "iox/vector.hpp"
#include "test.hpp"

#include <vector>

namespace
{
using namespace ::testing;
using namespace iox;

TEST(span_test, NewEmptySpanCreatedFromIteratorContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "59980664-c94f-4bb5-bc9d-adeac630746e");
    constexpr int* kNull = nullptr;

    constexpr span<int> empty_sut(kNull, 0);

    ASSERT_TRUE(empty_sut.empty());
    EXPECT_EQ(nullptr, empty_sut.data());
}

TEST(span_test, NewDynSpanCreatedFromIteratorContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "05db30c2-e13d-4116-ba05-668b30ba4a23");
    const std::vector<int> expected_data = {1, 1, 2, 3, 5, 8};
    std::vector<int> vector = expected_data;

    span<int> dyn_sut(vector.begin(), vector.size());

    EXPECT_EQ(vector.data(), dyn_sut.data());
    EXPECT_EQ(vector.size(), dyn_sut.size());
    for (size_t i = 0; i < dyn_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], dyn_sut[i]);
    }
}

TEST(span_test, NewStaticSpanCreatedFromIteratorContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "fdc6a3fe-3971-4326-b6b2-1967afbc9726");
    std::vector<int> vector = {8, 2, 2, 4, 5, 8};

    span<int, 6> static_sut(vector.begin(), vector.size());

    EXPECT_EQ(vector.data(), static_sut.data());
    EXPECT_EQ(vector.size(), static_sut.size());

    for (size_t i = 0; i < static_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], static_sut[i]);
    }
}

TEST(span_test, NewConstSpanCreatedFromContainerContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "593aa3b6-9937-469d-991d-9e682110727e");
    std::vector<int> vector = {6, 7, 2, 8, 9, 2};

    span<const int> const_sut(vector);

    EXPECT_EQ(vector.data(), const_sut.data());
    EXPECT_EQ(vector.size(), const_sut.size());
    for (size_t i = 0; i < const_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], const_sut[i]);
    }
}

TEST(span_test, NewDynSpanCreatedFromContainerContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "4b85bc77-2d3b-4a89-b86a-d5c75a4f3c49");
    std::vector<int> vector = {1, 1, 2, 3, 5, 8};

    span<int> dyn_sut(vector);

    ASSERT_EQ(vector.data(), dyn_sut.data());
    ASSERT_EQ(vector.size(), dyn_sut.size());
    for (size_t i = 0; i < dyn_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], dyn_sut[i]);
    }
}

TEST(span_test, NewStaticSpanCreatedFromContainerContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "3a5f3675-2365-4966-ae78-2035bac45db0");
    std::vector<int> vector = {1, 1, 13, 3, 5, 8};

    span<int, 6> static_sut(vector.data(), vector.size());

    ASSERT_EQ(vector.data(), static_sut.data());
    ASSERT_EQ(vector.size(), static_sut.size());
    for (size_t i = 0; i < static_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], static_sut[i]);
    }
}

TEST(span_test, NewConstSpanCreatedFromArrayContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "bbbd4ce2-30ea-4b32-86e3-aa7d0a1184d8");
    int array[] = {5, 41, 3, 2, 1};

    span<const int> const_sut(array);

    ASSERT_EQ(array, const_sut.data());
    ASSERT_EQ(iox::size(array), const_sut.size());
    for (size_t i = 0; i < const_sut.size(); ++i)
    {
        EXPECT_EQ(array[i], const_sut[i]);
    }
}
TEST(span_test, NewDynSpanCreatedFromArrayContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bd35b66-2cf7-42bc-b7b8-5344ac92d8fa");
    int array[] = {5, 24, 3, 22, 1};

    span<int> dyn_sut(array);

    ASSERT_EQ(array, dyn_sut.data());
    ASSERT_EQ(iox::size(array), dyn_sut.size());
    for (size_t i = 0; i < dyn_sut.size(); ++i)
    {
        EXPECT_EQ(array[i], dyn_sut[i]);
    }
}
TEST(span_test, NewStaticSpanCreatedFromArrayContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "3dfae2a7-d6eb-4961-a600-0e5d6738c283");
    int array[] = {5, 4, 3, 32, 1};

    span<int, iox::size(array)> static_sut(array);

    ASSERT_EQ(array, static_sut.data());
    ASSERT_EQ(iox::size(array), static_sut.size());
    for (size_t i = 0; i < static_sut.size(); ++i)
    {
        EXPECT_EQ(array[i], static_sut[i]);
    }
}

TEST(span_test, NewDynSpanCreatedFromConstexprArrayContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ec9c31c-b97f-43a3-9669-3bdff3a82b9e");
    static constexpr int arr[] = {5, 4, 3, 2, 1};

    constexpr span<const int> dyn_sut(arr);

    static_assert(arr == dyn_sut.data(), "Data needs to point to array");
    static_assert(iox::size(arr) == dyn_sut.size(), "Size needs to be the same as array size");
    static_assert(arr[0] == dyn_sut[0], "Values need to be the same");
    static_assert(arr[1] == dyn_sut[1], "Values need to be the same");
    static_assert(arr[2] == dyn_sut[2], "Values need to be the same");
    static_assert(arr[3] == dyn_sut[3], "Values need to be the same");
    static_assert(arr[4] == dyn_sut[4], "Values need to be the same");
}

TEST(span_test, NewStaticSpanCreatedFromConstexprArrayContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "e9aa537e-4d6b-48d1-bb04-b621a2d14df6");
    static constexpr int arr[] = {55, 44, 33, 22, 11};

    constexpr span<const int, iox::size(arr)> static_sut(arr);

    static_assert(arr == static_sut.data(), "Data needs to point to array");
    static_assert(iox::size(arr) == static_sut.size(), "Size needs to be the same as array size");
    static_assert(arr[0] == static_sut[0], "Values need to be the same");
    static_assert(arr[1] == static_sut[1], "Values need to be the same");
    static_assert(arr[2] == static_sut[2], "Values need to be the same");
    static_assert(arr[3] == static_sut[3], "Values need to be the same");
    static_assert(arr[4] == static_sut[4], "Values need to be the same");
}

TEST(span_test, NewConstSpanFromConstContainerContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "4358e397-c82b-45f7-a75f-8d0b1cf03667");
    const std::vector<int> vector = {1, 1, 2, 3, 5, 8};

    span<const int> const_sut(vector);

    ASSERT_EQ(vector.data(), const_sut.data());
    ASSERT_EQ(vector.size(), const_sut.size());
    for (size_t i = 0; i < const_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], const_sut[i]);
    }
}

TEST(span_test, NewStaticSpanFromConstContainerContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "28f85385-3bdb-4bd1-ad40-2bebe399ac08");
    const std::vector<int> vector = {1, 1, 2, 3, 5, 8};

    span<const int, 6> static_sut(vector.data(), vector.size());

    ASSERT_EQ(vector.data(), static_sut.data());
    ASSERT_EQ(vector.size(), static_sut.size());
    for (size_t i = 0; i < static_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], static_sut[i]);
    }
}

TEST(span_test, NewConstSpanFromIoxVectorContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "a7b1177b-0db5-44b8-bacd-b75d25c3a448");
    constexpr uint64_t CAPACITY{6U};
    constexpr int DEFAULT_VALUE{1};
    iox::vector<int, CAPACITY> vector(CAPACITY, DEFAULT_VALUE);
    vector[2] = 2;
    vector[3] = 3;
    vector[4] = 5;
    vector[5] = 7;

    span<const int> const_sut(vector);

    ASSERT_EQ(vector.data(), const_sut.data());
    ASSERT_EQ(vector.size(), const_sut.size());
    for (size_t i = 0; i < const_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], const_sut[i]);
    }
}

TEST(span_test, NewStaticSpanFromConstIoxVectorContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "af1bdb48-4cae-4c7d-a830-a098d84fb1aa");
    constexpr uint64_t CAPACITY{6U};
    constexpr int DEFAULT_VALUE{1};
    iox::vector<int, CAPACITY> vector(CAPACITY, DEFAULT_VALUE);
    vector[2] = 22;
    vector[3] = 33;
    vector[4] = 55;
    vector[5] = 77;

    span<const int, 6> static_sut(vector.data(), vector.size());

    ASSERT_EQ(vector.data(), static_sut.data());
    ASSERT_EQ(vector.size(), static_sut.size());
    for (size_t i = 0; i < static_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], static_sut[i]);
    }
}

TEST(span_test, NewConstSpanFromConstIoxUninitializedArrayContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "86ee3995-4267-4650-b1c4-4db8f5cf154b");
    constexpr uint64_t CAPACITY{6U};
    iox::UninitializedArray<int, CAPACITY> uninitializedArray;
    uninitializedArray[0] = 60;
    uninitializedArray[1] = 41;
    uninitializedArray[2] = 21;
    uninitializedArray[3] = 32;
    uninitializedArray[4] = 53;
    uninitializedArray[5] = 74;

    span<const int> const_sut(uninitializedArray);

    ASSERT_EQ(uninitializedArray.begin(), const_sut.data());
    ASSERT_EQ(uninitializedArray.capacity(), const_sut.size());
    for (size_t i = 0; i < const_sut.size(); ++i)
    {
        EXPECT_EQ(uninitializedArray[i], const_sut[i]);
    }
}

TEST(span_test, NewStaticSpanFromConstIoxUninitializedArrayContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "c6a3f7d2-dbab-4c9f-a405-6ee2cc3f4457");
    constexpr uint64_t CAPACITY{6U};
    iox::UninitializedArray<int, CAPACITY> uninitializedArray;
    uninitializedArray[0] = 66;
    uninitializedArray[1] = 44;
    uninitializedArray[2] = 22;
    uninitializedArray[3] = 33;
    uninitializedArray[4] = 55;
    uninitializedArray[5] = 77;

    span<const int, 6> static_sut(uninitializedArray.begin(), uninitializedArray.capacity());

    ASSERT_EQ(uninitializedArray.begin(), static_sut.data());
    ASSERT_EQ(uninitializedArray.capacity(), static_sut.size());
    for (size_t i = 0; i < static_sut.size(); ++i)
    {
        EXPECT_EQ(uninitializedArray[i], static_sut[i]);
    }
}

TEST(span_test, CheckFrontOfSpanIfItReturnsTheElementAtIndex0)
{
    ::testing::Test::RecordProperty("TEST_ID", "57b2f67f-79c1-4c1e-a305-f4665283c474");
    static constexpr int arr[] = {1, 6, 1, 8, 0};
    constexpr span<const int> span(arr);
    static_assert(&arr[0] == &span.front(), "span.front() does not refer to the same element as arr[0]");
}

TEST(span_test, CheckIterOfSpan)
{
    ::testing::Test::RecordProperty("TEST_ID", "4760addf-87f1-46c2-901a-63cf4de3a6ea");
    static constexpr int arr[] = {1, 6, 1, 8, 0};
    IOX_MAYBE_UNUSED constexpr span<const int> span(arr);

    EXPECT_TRUE(1 == span.begin()[0]);      // First element needs to be '1'
    EXPECT_TRUE(1 == *(span.begin() += 0)); // First element needs to be '1'
    EXPECT_TRUE(6 == *(span.begin() += 1)); // Second element needs to be '6'

    EXPECT_TRUE(1 == *((span.begin() + 1) -= 1)); // First element needs to be '1'
    EXPECT_TRUE(6 == *((span.begin() + 1) -= 0)); // Second element needs to be '6'
}

TEST(span_test, CheckConstexprIterOfSpan)
{
    ::testing::Test::RecordProperty("TEST_ID", "8764fcfb-27df-4f39-b8cd-56bf881db382");
#ifdef __GNUC__
    GTEST_SKIP() << "Some GCC versions (especially with address sanitizer) break the 'constexpr' therefore this test "
                    "can only be run with clang!";
#else
    static constexpr int arr[] = {1, 6, 1, 8, 0};
    IOX_MAYBE_UNUSED constexpr span<const int> span(arr);

    // Explicitly not use EXPECT_TRUE here to be able to execute the test case during compile-time
    static_assert(1 == span.begin()[0], "First element needs to be '1'");
    static_assert(1 == *(span.begin() += 0), "First element needs to be '1'");
    static_assert(6 == *(span.begin() += 1), "Second element needs to be '6'");

    static_assert(1 == *((span.begin() + 1) -= 1), "First element needs to be '1'");
    static_assert(6 == *((span.begin() + 1) -= 0), "Second element needs to be '6'");
#endif
}

TEST(span_test, GetSpanDataAsWritableBytes)
{
    ::testing::Test::RecordProperty("TEST_ID", "73ed24f9-c2ea-467a-b64e-e53e97247e8d");
    std::vector<int> vec = {1, 41, 2, 3, 5, 85};

    span<int> mutable_sut(vec);
    span<uint8_t> writable_bytes_sut = as_writable_bytes(mutable_sut);

    EXPECT_EQ(reinterpret_cast<uint8_t*>(vec.data()), writable_bytes_sut.data());
    EXPECT_EQ(sizeof(int) * vec.size(), writable_bytes_sut.size());
    EXPECT_EQ(writable_bytes_sut.size(), writable_bytes_sut.size_bytes());
    // Set the first entry of vec to zero while writing through the span.
    std::fill(writable_bytes_sut.data(), writable_bytes_sut.data() + sizeof(int), 0);
    EXPECT_EQ(0, vec[0]);
}

} // namespace
