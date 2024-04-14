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

#include <array>
#include <vector>

namespace
{
using namespace ::testing;
using namespace iox;

TEST(span_test, NewEmptySpanCreatedFromIteratorContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "59980664-c94f-4bb5-bc9d-adeac630746e");
    constexpr int32_t* kNull = nullptr;

    constexpr span<int32_t> empty_sut(kNull, 0);

    ASSERT_TRUE(empty_sut.empty());
    EXPECT_EQ(nullptr, empty_sut.data());
}

TEST(span_test, NewDynSpanCreatedFromIteratorAndSizeContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "05db30c2-e13d-4116-ba05-668b30ba4a23");
    const std::vector<int32_t> expected_data = {1, 1, 2, 3, 5, 8};
    std::vector<int32_t> vector = expected_data;

    span<int32_t> dyn_sut(vector.begin(), vector.size());

    EXPECT_EQ(vector.data(), dyn_sut.data());
    EXPECT_EQ(vector.size(), dyn_sut.size());
    for (size_t i = 0; i < dyn_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], dyn_sut[i]);
    }
}

TEST(span_test, NewStaticSpanCreatedFromIteratorAndSizeContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "fdc6a3fe-3971-4326-b6b2-1967afbc9726");
    std::vector<int32_t> vector = {8, 2, 2, 4, 5, 8};

    span<int32_t, 6> static_sut(vector.begin(), vector.size());

    EXPECT_EQ(vector.data(), static_sut.data());
    EXPECT_EQ(vector.size(), static_sut.size());

    for (size_t i = 0; i < static_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], static_sut[i]);
    }
}

TEST(span_test, NewDynSpanCreatedFromIteratorsContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f7224c9-b500-43f1-acb7-b64c5d407fce");
    const std::vector<int32_t> expected_data = {1, 1, 2, 3, 5, 8};
    std::vector<int32_t> vector = expected_data;

    span<int32_t> dyn_sut(vector.begin(), vector.end());

    EXPECT_EQ(vector.data(), dyn_sut.data());
    EXPECT_EQ(vector.size(), dyn_sut.size());
    for (size_t i = 0; i < dyn_sut.size(); ++i)
    {
        EXPECT_EQ(vector[i], dyn_sut[i]);
    }
}

TEST(span_test, NewStaticSpanCreatedFromIteratorsContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff3f731e-9fa2-4584-a6a4-090ba5aad5f4");
    std::vector<int32_t> vector = {8, 2, 2, 4, 5, 8};

    span<int32_t, 6> static_sut(vector.begin(), vector.end());

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
    std::vector<int32_t> vector = {6, 7, 2, 8, 9, 2};

    span<const int32_t> const_sut(vector);

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
    std::vector<int32_t> vector = {1, 1, 2, 3, 5, 8};

    span<int32_t> dyn_sut(vector);

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
    std::vector<int32_t> vector = {1, 1, 13, 3, 5, 8};

    span<int32_t, 6> static_sut(vector.data(), vector.size());

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
    // NOLINTJUSTIFICATION this is explicitely a test for a C-array
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,-warnings-as-errors)
    int32_t array[] = {5, 41, 3, 2, 1};

    span<const int32_t> const_sut(array);

    ASSERT_EQ(array, const_sut.data());
    ASSERT_EQ(iox::size(array), const_sut.size());

    size_t i = 0;
    for (const auto item : array)
    {
        EXPECT_EQ(item, const_sut[i]);
        ++i;
    }
}
TEST(span_test, NewDynSpanCreatedFromArrayContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bd35b66-2cf7-42bc-b7b8-5344ac92d8fa");
    // NOLINTJUSTIFICATION this is explicitely a test for a C-array
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,-warnings-as-errors)
    int32_t array[] = {5, 24, 3, 22, 1};

    span<int32_t> dyn_sut(array);

    ASSERT_EQ(array, dyn_sut.data());
    ASSERT_EQ(iox::size(array), dyn_sut.size());

    size_t i = 0;
    for (const auto item : array)
    {
        EXPECT_EQ(item, dyn_sut[i]);
        ++i;
    }
}
TEST(span_test, NewStaticSpanCreatedFromArrayContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "3dfae2a7-d6eb-4961-a600-0e5d6738c283");
    // NOLINTJUSTIFICATION this is explicitely a test for a C-array
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,-warnings-as-errors)
    int32_t array[] = {5, 4, 3, 32, 1};

    span<int32_t, iox::size(array)> static_sut(array);

    ASSERT_EQ(array, static_sut.data());
    ASSERT_EQ(iox::size(array), static_sut.size());

    size_t i = 0;
    for (const auto item : array)
    {
        EXPECT_EQ(item, static_sut[i]);
        ++i;
    }
}

TEST(span_test, NewDynSpanCreatedFromConstexprArrayContainsSameData)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ec9c31c-b97f-43a3-9669-3bdff3a82b9e");
    // NOLINTJUSTIFICATION this is explicitely a test for a C-array
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,-warnings-as-errors)
    static constexpr int32_t arr[] = {5, 4, 3, 2, 1};

    constexpr span<const int32_t> dyn_sut(arr);

    static_assert(&arr[0] == dyn_sut.data(), "Data needs to point to array");
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
    // NOLINTJUSTIFICATION this is explicitely a test for a C-array
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,-warnings-as-errors)
    static constexpr int32_t arr[] = {55, 44, 33, 22, 11};

    constexpr span<const int32_t, iox::size(arr)> static_sut(arr);

    static_assert(&arr[0] == static_sut.data(), "Data needs to point to array");
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
    const std::vector<int32_t> vector = {1, 1, 2, 3, 5, 8};

    span<const int32_t> const_sut(vector);

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
    const std::vector<int32_t> vector = {1, 1, 2, 3, 5, 8};

    span<const int32_t, 6> static_sut(vector.data(), vector.size());

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
    constexpr int32_t DEFAULT_VALUE{1};
    iox::vector<int32_t, CAPACITY> vector(CAPACITY, DEFAULT_VALUE);
    vector[2] = 2;
    vector[3] = 3;
    vector[4] = 5;
    vector[5] = 7;

    span<const int32_t> const_sut(vector);

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
    constexpr int32_t DEFAULT_VALUE{1};
    iox::vector<int32_t, CAPACITY> vector(CAPACITY, DEFAULT_VALUE);
    vector[2] = 22;
    vector[3] = 33;
    vector[4] = 55;
    vector[5] = 77;

    span<const int32_t, 6> static_sut(vector.data(), vector.size());

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
    iox::UninitializedArray<int32_t, CAPACITY> uninitializedArray;
    uninitializedArray[0] = 60;
    uninitializedArray[1] = 41;
    uninitializedArray[2] = 21;
    uninitializedArray[3] = 32;
    uninitializedArray[4] = 53;
    uninitializedArray[5] = 74;

    span<const int32_t> const_sut(uninitializedArray);

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
    iox::UninitializedArray<int32_t, CAPACITY> uninitializedArray;
    uninitializedArray[0] = 66;
    uninitializedArray[1] = 44;
    uninitializedArray[2] = 22;
    uninitializedArray[3] = 33;
    uninitializedArray[4] = 55;
    uninitializedArray[5] = 77;

    span<const int32_t, 6> static_sut(uninitializedArray.begin(), uninitializedArray.capacity());

    ASSERT_EQ(uninitializedArray.begin(), static_sut.data());
    ASSERT_EQ(uninitializedArray.capacity(), static_sut.size());
    for (size_t i = 0; i < static_sut.size(); ++i)
    {
        EXPECT_EQ(uninitializedArray[i], static_sut[i]);
    }
}

TEST(span_test, NewStaticSpanCopyConstructed)
{
    ::testing::Test::RecordProperty("TEST_ID", "88da307d-ed51-42a0-a587-784f29be7905");
    static constexpr std::array<int32_t, 5> arr = {1, 6, 2, 8, 0};
    const span<const int32_t, 5> original_span(arr.begin(), arr.size());
    span<const int32_t, 5> new_static_span(original_span);
    EXPECT_EQ(arr.data(), new_static_span.data());
    EXPECT_EQ(arr.size(), new_static_span.size());
}

TEST(span_test, NewDynamicSpanCopyConstructed)
{
    ::testing::Test::RecordProperty("TEST_ID", "9c9e897b-2755-44f9-9075-dc224d0e72ac");
    static constexpr std::array<int32_t, 5> arr = {1, 6, 2, 8, 0};
    const span<const int32_t> original_span(arr);
    span<const int32_t> new_static_span(original_span);
    EXPECT_EQ(arr.data(), new_static_span.data());
    EXPECT_EQ(arr.size(), new_static_span.size());
}

TEST(span_test, NewStaticSpanMoveConstructed)
{
    ::testing::Test::RecordProperty("TEST_ID", "9e8c6cea-e005-41c2-9bc3-ebfb968b3674");
    static constexpr std::array<int32_t, 5> arr = {1, 6, 2, 8, 0};
    span<const int32_t, 5> original_span(arr.begin(), arr.size());
    span<const int32_t, 5> new_static_span(std::move(original_span));
    EXPECT_EQ(arr.data(), new_static_span.data());
    EXPECT_EQ(arr.size(), new_static_span.size());
}

TEST(span_test, NewDynamicSpanMoveConstructed)
{
    ::testing::Test::RecordProperty("TEST_ID", "e58c41f5-4ea8-40e9-8131-0f8e7a93644c");
    static constexpr std::array<int32_t, 5> arr = {1, 6, 2, 8, 0};
    span<const int32_t> original_span(arr);
    span<const int32_t> new_static_span(std::move(original_span));
    EXPECT_EQ(arr.data(), new_static_span.data());
    EXPECT_EQ(arr.size(), new_static_span.size());
}

TEST(span_test, CheckFrontOfSpanIfItReturnsTheElementAtIndex0)
{
    ::testing::Test::RecordProperty("TEST_ID", "57b2f67f-79c1-4c1e-a305-f4665283c474");
    static constexpr std::array<int32_t, 5> arr = {1, 6, 2, 8, 0};
    constexpr span<const int32_t> span(arr);
    static_assert(arr.data() == &span.front(), "span.front() does not refer to the same element as arr[0]");
    // Also check at runtime to show in coverage reports
    EXPECT_EQ(arr[0], span.front());
}

TEST(span_test, CheckBackOfSpanIfItReturnsTheElementAtLastIndex)
{
    ::testing::Test::RecordProperty("TEST_ID", "2b9fa3d2-e57b-4c17-b8ef-541de8b3f9f9");
    static constexpr std::array<int32_t, 5> arr = {1, 6, 2, 8, 0};
    constexpr span<const int32_t> span(arr);
    static_assert(arr.data() + 4 == &span.back(), "span.back() does not refer to the same element as arr[N-1]");
    // Also check at runtime to show in coverage reports
    EXPECT_EQ(arr[4], span.back());
}

TEST(span_test, CheckIterOfSpan)
{
    ::testing::Test::RecordProperty("TEST_ID", "4760addf-87f1-46c2-901a-63cf4de3a6ea");
    static constexpr std::array<int32_t, 5> arr = {1, 6, 2, 8, 0};
    [[maybe_unused]] constexpr span<const int32_t> span(arr);

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
    static constexpr std::array<int32_t, 5> arr = {1, 6, 2, 8, 0};
    [[maybe_unused]] constexpr span<const int32_t> span(arr);

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
    std::vector<int32_t> vec = {1, 41, 2, 3, 5, 85};

    span<int32_t> mutable_sut(vec);
    span<uint8_t> writable_bytes_sut = as_writable_bytes(mutable_sut);

    EXPECT_EQ(static_cast<void*>(vec.data()), static_cast<void*>(writable_bytes_sut.data()));
    EXPECT_EQ(sizeof(int32_t) * vec.size(), writable_bytes_sut.size());
    EXPECT_EQ(writable_bytes_sut.size(), writable_bytes_sut.size_bytes());
    // Set the first entry of vec to zero while writing through the span.
    std::fill(writable_bytes_sut.begin(), writable_bytes_sut.begin() + sizeof(int32_t), 0);
    EXPECT_EQ(0, vec[0]);
}

TEST(span_test, IterateOverSpan)
{
    ::testing::Test::RecordProperty("TEST_ID", "87924274-b774-467e-8ffc-a66a46596cbe");
    std::vector<int32_t> vector = {1, 1, 13, 3, 5, 8};
    span<int32_t, 6> static_sut(vector.data(), vector.size());

    // Sum the values in the span as a simple test
    int32_t sum{0U};
    for (const auto& val : static_sut)
    {
        sum += val;
    }
    EXPECT_EQ(sum, 31);
}

TEST(span_test, IterateOverSpanInReverse)
{
    ::testing::Test::RecordProperty("TEST_ID", "2994f138-41ec-4a51-8266-c4c461454411");
    std::vector<int32_t> vector = {1, 1, 13, 3, 5, 8};
    span<int32_t, 6> static_sut(vector.data(), vector.size());

    // Sum the values in the span as a simple test
    int32_t sum{0U};
    for (auto it = static_sut.rbegin(); it != static_sut.rend(); ++it)
    {
        sum += *it;
    }
    EXPECT_EQ(sum, 31);
}

TEST(span_test, CreateStaticSubspan)
{
    ::testing::Test::RecordProperty("TEST_ID", "bd1983a4-3e73-4f1f-8bca-7613fa2a0b43");
    constexpr std::array<int32_t, 6> array = {1, 1, 13, 3, 5, 8};
    const span<const int32_t, 6> static_sut(array.begin(), array.end());

    // Create subspan
    const span<const int32_t, 3> subspan = static_sut.subspan<2U, 3U>();

    EXPECT_EQ(subspan.size(), 3);
    EXPECT_EQ(subspan.data(), &array[2]);
    EXPECT_EQ(subspan[0], 13);
    EXPECT_EQ(subspan[1], 3);
    EXPECT_EQ(subspan[2], 5);
}

TEST(span_test, CreateDynSubspan)
{
    ::testing::Test::RecordProperty("TEST_ID", "45595686-ed6e-47e1-9523-7312052187ec");
    constexpr std::array<int32_t, 6> array = {1, 1, 13, 3, 5, 8};
    const span<const int32_t, 6> static_sut(array.begin(), array.end());

    // Create subspan
    const span<const int32_t> subspan = static_sut.subspan(1U, 2U);

    EXPECT_EQ(subspan.size(), 2);
    EXPECT_EQ(subspan.data(), &array[1]);
    EXPECT_EQ(subspan[0], 1);
    EXPECT_EQ(subspan[1], 13);
}

TEST(span_test, CreateStaticSubspanFirstN)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a0d421c-f308-4ef8-a3b1-dd795e8920b0");
    constexpr std::array<int32_t, 6> array = {1, 1, 13, 3, 5, 8};
    const span<const int32_t, 6> static_sut(array.begin(), array.end());

    // Create subspan
    const span<const int32_t, 3> subspan = static_sut.first<3U>();

    EXPECT_EQ(subspan.size(), 3);
    EXPECT_EQ(subspan.data(), array.data());
    EXPECT_EQ(subspan[0], 1);
    EXPECT_EQ(subspan[1], 1);
    EXPECT_EQ(subspan[2], 13);
}

TEST(span_test, CreateDynSubspanFirstN)
{
    ::testing::Test::RecordProperty("TEST_ID", "bc1df89d-e727-42a2-9d1f-20055883e605");
    constexpr std::array<int32_t, 6> array = {1, 1, 13, 3, 5, 8};
    const span<const int32_t, 6> static_sut(array.begin(), array.end());

    // Create subspan
    const span<const int32_t> subspan = static_sut.first(3U);

    EXPECT_EQ(subspan.size(), 3);
    EXPECT_EQ(subspan.data(), array.data());
    EXPECT_EQ(subspan[0], 1);
    EXPECT_EQ(subspan[1], 1);
    EXPECT_EQ(subspan[2], 13);
}

TEST(span_test, CreateStaticSubspanLastN)
{
    ::testing::Test::RecordProperty("TEST_ID", "e1ae58ef-e4c5-4ea9-88b0-af0701f5cebe");
    constexpr std::array<int32_t, 6> array = {1, 1, 13, 3, 5, 8};
    const span<const int32_t, 6> static_sut(array.begin(), array.end());

    // Create subspan
    const span<const int32_t, 4> subspan = static_sut.last<4U>();

    EXPECT_EQ(subspan.size(), 4);
    EXPECT_EQ(subspan.data(), &array[2]);
    EXPECT_EQ(subspan[0], 13);
    EXPECT_EQ(subspan[1], 3);
    EXPECT_EQ(subspan[2], 5);
    EXPECT_EQ(subspan[3], 8);
}

TEST(span_test, CreateDynSubspanLastN)
{
    ::testing::Test::RecordProperty("TEST_ID", "4948e802-3134-45f7-89fa-3d51bfe0e3eb");
    constexpr std::array<int32_t, 6> array = {1, 1, 13, 3, 5, 8};
    const span<const int32_t, 6> static_sut(array.begin(), array.end());

    // Create subspan
    const span<const int32_t> subspan = static_sut.last(4U);

    EXPECT_EQ(subspan.size(), 4);
    EXPECT_EQ(subspan.data(), &array[2]);
    EXPECT_EQ(subspan[0], 13);
    EXPECT_EQ(subspan[1], 3);
    EXPECT_EQ(subspan[2], 5);
    EXPECT_EQ(subspan[3], 8);
}

} // namespace
