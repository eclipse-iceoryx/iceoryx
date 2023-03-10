// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/error_handling/error_handling.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iox/string.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::testing;

template <typename T>
class stringTyped_test : public Test
{
  protected:
    T testSubject;

    using stringType = T;
};

using Implementations = Types<string<1>, string<15>, string<100>, string<1000>>;

TYPED_TEST_SUITE(stringTyped_test, Implementations, );

TEST(string_test, CapacityReturnsSpecifiedCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "6eed7f1a-c8d9-4902-ac22-405a3bd62d28");
    constexpr uint16_t CAPACITY_ONE{1};
    constexpr uint16_t CAPACITY_FOURTYTWO{42};

    EXPECT_THAT(string<CAPACITY_ONE>::capacity(), Eq(CAPACITY_ONE));
    EXPECT_THAT(string<CAPACITY_FOURTYTWO>::capacity(), Eq(CAPACITY_FOURTYTWO));
}

/// @note string() noexcept
TYPED_TEST(stringTyped_test, EmptyInitializationResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a1a3850-b151-4f11-8208-b286e403847d");
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
}

TYPED_TEST(stringTyped_test, EmptyInitializationResultsInEmptyString)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4833ccc-814c-408e-a469-9950550703b9");
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

/// @note string(const string& other) noexcept
TYPED_TEST(stringTyped_test, CopyConstructEmptyStringResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a88b069-4c7c-425a-b515-35deeaacaae5");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu(this->testSubject);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, CopyConstructStringOfSizeCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d6bc778-8a1e-4eb5-8f87-0ebf45c5116e");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP> fuu(this->testSubject);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), StrEq(testString));
}

/// @note string(string&& other) noexcept
TYPED_TEST(stringTyped_test, MoveConstructionWithStringOfSize0Works)
{
    ::testing::Test::RecordProperty("TEST_ID", "1857ab80-bb27-4e4d-8f11-6ddf94b0cfaf");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testString(std::move(this->testSubject));
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveConstructionWithStringOfSizeSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb79ff56-d10f-4bb3-85a5-08c1ccc6947a");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP - 1U, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP> fuu(std::move(this->testSubject));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveConstructionWithStringOfSizeCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f313dd8a-7dc4-40e3-8203-e099e2a05305");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP> fuu(std::move(this->testSubject));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

/// @note string& operator=(const string& rhs) noexcept
TYPED_TEST(stringTyped_test, SelfCopyAssignmentExcluded)
{
    ::testing::Test::RecordProperty("TEST_ID", "51b70520-fc11-4948-8fdf-643c638a81b9");
    this->testSubject = "M";
    this->testSubject = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, CopyAssignmentWithStringOfSize0Works)
{
    ::testing::Test::RecordProperty("TEST_ID", "fdfca1c5-8293-4e14-beef-519729e71420");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu;
    fuu = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, CopyAssignmentWithStringOfSizeSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b9b030db-e760-42ab-adbd-311cb5293a2e");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP - 1U, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP> fuu;
    fuu = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testString.substr(0U, STRINGCAP - 1U)));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(fuu.c_str(), StrEq(testString.substr(0U, STRINGCAP - 1U)));
}

TYPED_TEST(stringTyped_test, CopyAssignmentWithStringOfSizeCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "10fa4f3d-089c-4d10-83e9-5a37f7d5f0ec");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP> fuu;
    fuu = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.c_str(), Eq(testString));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
}

/// @note string& operator=(string&& rhs) noexcept
TYPED_TEST(stringTyped_test, SelfMoveAssignmentExcluded)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ad45975-b68b-465a-b8c5-83dd8d8290d5");
    this->testSubject = "M";
    this->testSubject = std::move(this->testSubject);
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfStringWithSize0ResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "7170b9d6-ff73-4e3f-a6d1-b66f6de21dcd");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfStringWithSmallerSizeResultsInSmallerSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca455516-5ca6-4882-933b-082769eb975a");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP - 1U, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfStringWithSizeCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "375944b8-6817-44d4-8a05-de9e0235c503");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

/// @note template <uint64_t N>
/// string(const char (&other)[N]) noexcept
TYPED_TEST(stringTyped_test, CharToStringConvConstrWithSize0ResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "29e9f7b8-a1c7-4462-81d0-52ef34a8b883");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("");
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, CharToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "de81475a-527e-43e0-97b8-faf7a9300204");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testChar[STRINGCAP];
    for (uint64_t i = 0U; i < STRINGCAP - 1U; i++)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1U] = '\0';
    string<STRINGCAP> testSubject(testChar);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(testSubject.c_str(), StrEq(&testChar[0]));
}

/// @note string(TruncateToCapacity_t, const char* const other) noexcept
TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithSize0ResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ed6360e-c4a0-474b-8ed7-e6b4e129a6c0");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu(TruncateToCapacity, "");
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "5c417b37-ee9d-42f9-bb25-c59c6929d4ca");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    // increase capacity by one to circumvent gcc -Werror=array-bounds
    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testChar[STRINGCAP + 1];
    for (uint64_t i = 0U; i < STRINGCAP - 1U; i++)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1U] = '\0';
    string<STRINGCAP> testSubject(TruncateToCapacity, &testChar[0]);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(testSubject.c_str(), StrEq(&testChar[0]));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithSizeGreaterCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "5e0a2023-ea15-43d5-aae8-980a75be6122");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testChar[STRINGCAP + 1U];
    for (uint64_t i = 0U; i < STRINGCAP; i++)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testChar[i] = 'M';
    }
    testChar[STRINGCAP] = '\0';
    string<STRINGCAP> testSubject(TruncateToCapacity, &testChar[0]);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithNullPtrResultsEmptyString)
{
    ::testing::Test::RecordProperty("TEST_ID", "c6bbcbc6-e049-4c2c-bf84-8d89dcf42ce8");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu(TruncateToCapacity, nullptr);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}


/// @note string(TruncateToCapacity_t, const char* const other, const uint64_t count) noexcept
TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithCount0ResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "6cbd9ee8-f015-41ce-8a2c-b8c023b0722a");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu(TruncateToCapacity, "Yoda", 0U);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithCountEqCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "0e62c9fe-78e7-4b6d-8c31-27270dca7581");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    string<STRINGCAP> testSubject(TruncateToCapacity, testString.c_str(), STRINGCAP);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.c_str(), StrEq(testString));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithCountGreaterCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2d321ed-8ee3-4337-a7c9-4785b5ae9a67");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP + 1U, 'M');
    string<STRINGCAP> testSubject(TruncateToCapacity, testString.c_str(), STRINGCAP + 1U);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.c_str(), StrEq(testString.substr(0U, STRINGCAP)));
}

TEST(String100, UnsafeCharToStringConstrIncludingNullCharWithCountResultsInSizeCount)
{
    ::testing::Test::RecordProperty("TEST_ID", "548eb26e-39b0-4c35-ad80-7665cde80361");
    constexpr uint64_t STRING_CAPACITY = 100U;
    constexpr uint64_t STRING_SIZE = 7U;
    std::string testString{"ice\0ryx", STRING_SIZE};
    string<STRING_CAPACITY> testSubject(TruncateToCapacity, testString.c_str(), STRING_SIZE);
    EXPECT_THAT(testSubject.capacity(), Eq(STRING_CAPACITY));
    EXPECT_THAT(testSubject.size(), Eq(STRING_SIZE));
    // NOLINTNEXTLINE(bugprone-string-literal-with-embedded-nul) this shall be explicitly tested
    EXPECT_THAT(testSubject.c_str(), StrEq("ice\0ryx"));
}

TEST(CharArrayAssignment, AssignCharArrayWithStringSizeLessThanArraySize)
{
    ::testing::Test::RecordProperty("TEST_ID", "886f580d-e57c-4685-90bf-2399737779be");
    constexpr uint64_t STRING_CAPACITY = 20U;
    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testString[STRING_CAPACITY] = "iceoryx";
    string<STRING_CAPACITY> testSubject(testString);
    EXPECT_THAT(testSubject.size(), Eq(7U));
    EXPECT_THAT(testSubject.c_str(), StrEq("iceoryx"));
}

TEST(CharArrayAssignment, AssignZeroTerminatedCharArrayWithSizeForFullCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "884e724a-f5d3-41d1-89fa-96f55ce99a96");
    constexpr uint64_t STRING_CAPACITY = 7U;
    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testString[STRING_CAPACITY + 1U] = "iceoryx";
    string<STRING_CAPACITY> testSubject(testString);
    EXPECT_THAT(testSubject.size(), Eq(STRING_CAPACITY));
    EXPECT_THAT(testSubject.c_str(), StrEq("iceoryx"));
}

TEST(CharArrayAssignment, AssignNonZeroTerminatedCharArrayOfSizeForFullCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "2a43553f-4358-4c41-a885-1495de0d7f4f");
    constexpr uint64_t STRING_CAPACITY = 7U;
    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testString[STRING_CAPACITY + 1U] = "iceoryx";
    testString[STRING_CAPACITY] = 'x'; // overwrite the 0 termination
    string<STRING_CAPACITY> testSubject(testString);
    EXPECT_THAT(testSubject.size(), Eq(STRING_CAPACITY));
    EXPECT_THAT(testSubject.c_str(), StrEq("iceoryx"));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithNullPtrResultsEmptyString)
{
    ::testing::Test::RecordProperty("TEST_ID", "cd772d2a-b6db-4b9d-b90d-f2a0aca4aaf6");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu(TruncateToCapacity, nullptr, STRINGCAP);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

/// @note template <uint64_t N>
/// string& operator=(const char (&rhs)[N]) noexcept
TYPED_TEST(stringTyped_test, AssignCStringOfSize0WithOperatorResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "49d08010-7a31-472d-bd99-6c6e7c49aad5");
    this->testSubject = "";
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, AssignCStringOfSizeCapaWithOperatorResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "19b0a4af-acfa-4d9b-b432-145ab1e7f59d");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testChar[STRINGCAP];
    for (uint64_t i = 0U; i < STRINGCAP - 1U; i++)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1U] = '\0';
    string<STRINGCAP> testSubject;
    testSubject = testChar;
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(testSubject.c_str(), StrEq(&testChar[0]));
}

/// @note template <uint64_t N>
/// string& assign(const string& str) noexcept
TYPED_TEST(stringTyped_test, SelfAssignmentIsExcluded)
{
    ::testing::Test::RecordProperty("TEST_ID", "32c07349-3924-4b86-9ce8-21247c8148b9");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testSubject;
    testSubject.assign(testSubject);
    EXPECT_THAT(testSubject.size(), Eq(0U));
}

TYPED_TEST(stringTyped_test, AssignStringOfSize0ResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "ee981dcb-80ab-439e-a3bf-a52a591b7dbb");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu;
    this->testSubject.assign(fuu);
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, AssignStringOfSizeCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "75803631-93e8-4555-84d1-cdb44d88adc4");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    string<STRINGCAP> fuu(TruncateToCapacity, testString.c_str(), testString.size());
    this->testSubject.assign(fuu);
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testString));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), StrEq(testString));
}

TYPED_TEST(stringTyped_test, AssignStringOfSize0AndSmallerCapaResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "d66ee5d5-03ac-4485-ac73-b8b6f044d38a");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> testString;
    testString.assign(this->testSubject);
    EXPECT_THAT(testString.size(), Eq(0U));
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, AssignStringWithSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d7bdad39-fd7f-4b32-9e3f-60e6cd246bdc");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString.c_str()), Eq(true));
    string<STRINGCAP + 1U> testString;
    testString.assign(this->testSubject);
    EXPECT_THAT(testString.size(), Eq(STRINGCAP));
    EXPECT_THAT(testString.c_str(), StrEq(testStdString));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testStdString));
}

/// @note template <uint64_t N>
/// string& assign(const char (&str)[N]) noexcept
TYPED_TEST(stringTyped_test, FreshlyAssignNothingResultsInZeroSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0df23ee-86f8-4821-b577-38bad21c2e77");
    this->testSubject.assign("");
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
}

TYPED_TEST(stringTyped_test, ReassignNothingResultsInZeroSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "854089ee-bf0f-432c-96ce-20e83e09181c");
    this->testSubject.assign("M");
    this->testSubject.assign("");
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
}

TYPED_TEST(stringTyped_test, AssignCStringOfSizeCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "25f4f306-2303-4cc8-a42b-d0cbb600d833");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testChar[STRINGCAP];
    for (uint64_t i = 0U; i < STRINGCAP - 1U; i++)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1U] = '\0';
    string<STRINGCAP> testSubject;
    testSubject.assign(testChar);
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(testSubject.c_str(), StrEq(&testChar[0]));
}

/// @note bool unsafe_assign(const char* const str) noexcept
TYPED_TEST(stringTyped_test, UnsafeAssignOfCStringOfSize0ResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "861aa3a2-4fa3-401a-b6af-2ddbb3139f69");
    EXPECT_THAT(this->testSubject.unsafe_assign(""), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfCStringOfSize1ResultsInSize1)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b09a56c-1706-43b9-85d0-6c8bdf79f1b2");
    EXPECT_THAT(this->testSubject.unsafe_assign("M"), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, UnsafeAssignCStringOfSizeCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "422a0400-0862-441f-a7d0-20272856800f");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::vector<char> testCharstring(STRINGCAP, 'M');
    testCharstring.emplace_back('\0');
    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
}

TYPED_TEST(stringTyped_test, UnsafeAssignCStringOfSizeGreaterCapaResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "6012209b-87f8-4796-b9ec-336881cf7e24");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::vector<char> testCharstring(STRINGCAP + 1U, 'M');
    testCharstring.emplace_back('\0');
    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfInvalidCStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "863d38e5-5c5f-4a78-a2ca-155205bb9ecc");
    this->testSubject = "L";

    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::vector<char> testCharstring(STRINGCAP + 1U, 'M');
    testCharstring.emplace_back('\0');

    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("L"));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfCharPointerPointingToSameAddress)
{
    ::testing::Test::RecordProperty("TEST_ID", "a129add2-4d8e-4953-ba28-b78ece5a2f02");
    this->testSubject = "M";
    const char* fuu = this->testSubject.c_str();
    EXPECT_THAT(this->testSubject.unsafe_assign(fuu), Eq(false));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "140e5402-c6b5-4a07-a0f7-2a10f6d307fb");
    EXPECT_THAT(this->testSubject.unsafe_assign(nullptr), Eq(false));
}

/// @note template <uint64_t N>
/// int64_t compare(const string<N>& other) const noexcept
TYPED_TEST(stringTyped_test, CompareEqStringsResultsInZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "8271fa31-3301-4d2e-a290-a4b61a70dfb5");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP> fuu;
    EXPECT_THAT(fuu.unsafe_assign(testString.c_str()), Eq(true));
    EXPECT_THAT(this->testSubject.compare(this->testSubject), Eq(0));
    EXPECT_THAT(this->testSubject.compare(fuu), Eq(0));
}

TYPED_TEST(stringTyped_test, CompareResultNegative)
{
    ::testing::Test::RecordProperty("TEST_ID", "b17fc495-a82b-4ee8-af17-28afaabd3f0e");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1.c_str()), Eq(true));
    string<STRINGCAP> fuu;
    std::string testString2(STRINGCAP, 'L');
    EXPECT_THAT(fuu.unsafe_assign(testString2.c_str()), Eq(true));
    EXPECT_THAT(fuu.compare(this->testSubject), Lt(0));
}

TYPED_TEST(stringTyped_test, CompareResultPositive)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff95b244-937a-4519-90d8-8c82acf01b6b");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1.c_str()), Eq(true));
    string<STRINGCAP> fuu;
    std::string testString2(STRINGCAP, 'L');
    EXPECT_THAT(fuu.unsafe_assign(testString2.c_str()), Eq(true));
    EXPECT_THAT(this->testSubject.compare(fuu), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareWithEmptyStringResultsInPositive)
{
    ::testing::Test::RecordProperty("TEST_ID", "0495349d-2e63-442f-8214-b44d249f057f");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    EXPECT_THAT(fuu.compare(this->testSubject), Gt(0));
}

TEST(String100, CompareStringsInclNullCharacterWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a6c1b983-a88c-46bb-a38f-33947e52f209");
    constexpr uint64_t STRING_CAPACITY = 100U;
    constexpr uint64_t STRING_SIZE = 7U;
    std::string testString1{"ice\0ryx", STRING_SIZE};
    std::string testString2{"ice\0rYx", STRING_SIZE};
    string<STRING_CAPACITY> testSubject1(TruncateToCapacity, testString1.c_str(), STRING_SIZE);
    string<STRING_CAPACITY> testSubject2(TruncateToCapacity, testString2.c_str(), STRING_SIZE);
    EXPECT_THAT(testSubject1.compare(testSubject2), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareEqStringsWithDifferentCapaResultsInZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "ddf78ea1-b5cd-44fd-9320-6801893c30e7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP + 1U> fuu;
    EXPECT_THAT(fuu.unsafe_assign(testString.c_str()), Eq(true));
    EXPECT_THAT(this->testSubject.compare(this->testSubject), Eq(0));
    EXPECT_THAT(this->testSubject.compare(fuu), Eq(0));
}

TYPED_TEST(stringTyped_test, CompareResultNegativeWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0499ebc-249c-4b31-a1b0-0e34035e77f2");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1.c_str()), Eq(true));
    string<STRINGCAP + 1U> fuu;
    std::string testString2(STRINGCAP + 1U, 'M');
    EXPECT_THAT(fuu.unsafe_assign(testString2.c_str()), Eq(true));
    EXPECT_THAT(this->testSubject.compare(fuu), Lt(0));
}

TYPED_TEST(stringTyped_test, CompareResultPositiveWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "97fed2cb-4f25-4732-9bbe-4d710b9a35f7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1.c_str()), Eq(true));
    string<STRINGCAP + 1U> fuu;
    std::string testString2(STRINGCAP + 1U, 'M');
    EXPECT_THAT(fuu.unsafe_assign(testString2.c_str()), Eq(true));
    EXPECT_THAT(fuu.compare(this->testSubject), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareWithEmptyStringOfDifferentCapaResultsInPositive)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d54c681-fc19-444b-8e1b-059ff6237a8f");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> fuu("M");
    EXPECT_THAT(fuu.compare(this->testSubject), Gt(0));
}

TEST(String100, CompareStringsWithDifferentCapaInclNullCharacterWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe260cb6-5d77-42b1-89b8-073c9ea9593d");
    constexpr uint64_t STRING_SIZE = 7U;
    std::string testString1{"ice\0ryx", STRING_SIZE};
    std::string testString2{"ice\0rYx", STRING_SIZE};
    string<200> testSubject1(TruncateToCapacity, testString1.c_str(), STRING_SIZE);
    string<100> testSubject2(TruncateToCapacity, testString2.c_str(), STRING_SIZE);
    EXPECT_THAT(testSubject1.compare(testSubject2), Gt(0));
}

/// @note template <uint64_t N>
/// bool operator==(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorEqualResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "149069dd-a2f2-441a-9d16-30aa038a7c5e");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    EXPECT_THAT(fuu == fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorEqualResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "30c9bb50-03ea-437e-99c8-c663ed62340b");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu == bar, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorEqualResultTrueWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "10ce496d-635e-4aeb-9969-95881d1efc87");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testString1("M");
    string<STRINGCAP + 1U> testString2("M");
    EXPECT_THAT(testString1 == testString2, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorEqualResultFalseWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "15444c15-7319-4224-8519-091d2b47da22");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> testString1("M");
    string<STRINGCAP> testString2("L");
    string<STRINGCAP + 1U> testString3;
    std::string testStdString(STRINGCAP + 1U, 'L');
    EXPECT_THAT(testString3.unsafe_assign(testStdString.c_str()), Eq(true));
    EXPECT_THAT(testString1 == testString2, Eq(false));
    EXPECT_THAT(testString3 == testString2, Eq(false));
}

/// @note template <uint64_t N>
/// bool operator!=(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorNotEqualResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "b31770c0-2695-4fd6-b5b6-cae479852417");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    EXPECT_THAT(fuu != fuu, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorNotEqualResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "57db46d4-df73-49da-aa43-0dfc8ff04f44");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu != bar, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorNotEqualResultFalseWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "f5b53871-4bb0-4f2a-adea-e5335c6c4611");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testString1("M");
    string<STRINGCAP + 1U> testString2("M");
    EXPECT_THAT(testString1 != testString2, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorNotEqualResultTrueWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "394d3dd9-2304-4608-9f29-12cfaacaeef7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> testString1("M");
    string<STRINGCAP> testString2("L");
    string<STRINGCAP + 1U> testString3;
    std::string testStdString(STRINGCAP + 1U, 'L');
    EXPECT_THAT(testString3.unsafe_assign(testStdString.c_str()), Eq(true));
    EXPECT_THAT(testString1 != testString2, Eq(true));
    EXPECT_THAT(testString3 != testString2, Eq(true));
}

/// @note template <uint64_t N>
/// bool operator<(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorLessResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb8495ba-f2f9-4e7c-a017-f8295fcff518");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(bar < fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLessResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "6bf80f49-f637-4207-900e-2cfec89eb556");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu < bar, Eq(false));
    EXPECT_THAT(fuu < fuu, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorLessResultTrueWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "bbe252cb-6fba-45d1-ad31-cac42fb73caa");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testString1("M");
    string<STRINGCAP + 1U> testString2("L");
    EXPECT_THAT(testString2 < testString1, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLessResultFalseWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "884217ee-085b-479d-8ab1-a0476f14105c");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> testString1("M");
    string<STRINGCAP> testString2("L");
    EXPECT_THAT(testString1 < testString2, Eq(false));
    EXPECT_THAT(testString1 < testString1, Eq(false));
}

/// @note template <uint64_t N>
/// bool operator<=(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorLessEqResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "3a8ba399-4e2c-483d-835a-6d4bcbfa9a29");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(this->testSubject <= fuu, Eq(true));
    EXPECT_THAT(bar <= fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLessEqResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "59928731-2d4b-4122-8924-7809001fa631");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu <= bar, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorLessEqResultTrueWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "60c20357-c67a-4d4f-aca7-1ce3ab84677c");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP + 1U> bar("L");
    EXPECT_THAT(this->testSubject <= fuu, Eq(true));
    EXPECT_THAT(bar <= fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLessEqResultFalseWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "9126ec2b-eb0c-4567-aa9e-8009e9910571");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu <= bar, Eq(false));
}

/// @note template <uint64_t N>
/// bool operator>(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorGreaterResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "92c81295-3169-49ac-a12f-700a5dd2179b");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu > bar, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b3d33af-c905-44de-9ece-c66a96e59fb9");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(bar > fuu, Eq(false));
    EXPECT_THAT(bar > bar, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterResultTrueWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb2b2abd-4670-44e9-a1ea-e1ae2d437ce2");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu > bar, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterResultFalseWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "5bacb492-2d28-4260-96d2-516858462054");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP + 1U> bar("L");
    EXPECT_THAT(bar > fuu, Eq(false));
    EXPECT_THAT(bar > bar, Eq(false));
}

/// @note template <uint64_t N>
/// bool operator>=(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorGreaterEqResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "97184a06-2153-4b8e-93b7-0c620b90efb2");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    this->testSubject = "M";
    EXPECT_THAT(fuu >= bar, Eq(true));
    EXPECT_THAT(fuu >= this->testSubject, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterEqResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "72dca46d-86aa-4a01-b282-97b46f7f1504");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(bar >= fuu, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterEqResultTrueWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "edb0b737-630e-4305-9fd6-0f9c8be524b4");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> fuu("M");
    string<STRINGCAP> bar("L");
    this->testSubject = "M";
    EXPECT_THAT(fuu >= bar, Eq(true));
    EXPECT_THAT(fuu >= this->testSubject, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterEqResultFalseWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "aa2e606e-28a2-424b-ac98-b1d194a11738");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP + 1U> bar("L");
    this->testSubject = "L";
    EXPECT_THAT(bar >= fuu, Eq(false));
}

/// @note int64_t compare(const T& other) const noexcept
/// with T = {char array}
TYPED_TEST(stringTyped_test, CompareEqCharArrayResultsInZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "13a0f1a3-b006-4686-a5a0-3c6a2c7113e0");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testStdString(STRINGCAP, 'M');
    ASSERT_THAT(this->testSubject.unsafe_assign(testStdString.c_str()), Eq(true));

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 1U];
    for (uint64_t i = 0U; i < STRINGCAP; ++i)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testCharArray[i] = 'M';
    }
    testCharArray[STRINGCAP] = '\0';
    EXPECT_THAT(this->testSubject.compare(testCharArray), Eq(0));
}

TYPED_TEST(stringTyped_test, CompareWithCharArrayResultNegative)
{
    ::testing::Test::RecordProperty("TEST_ID", "df4a32d0-72b1-4c65-86f3-15b007ab003c");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string temp(STRINGCAP, 'L');
    ASSERT_THAT(this->testSubject.unsafe_assign(temp.c_str()), Eq(true));

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 1U];
    for (uint64_t i = 0U; i < STRINGCAP; ++i)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testCharArray[i] = 'M';
    }
    testCharArray[STRINGCAP] = '\0';
    EXPECT_THAT(this->testSubject.compare(testCharArray), Lt(0));
}

TYPED_TEST(stringTyped_test, CompareWithCharArrayResultPositive)
{
    ::testing::Test::RecordProperty("TEST_ID", "d315afbc-558d-474a-8ae5-f53451526c73");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string temp(STRINGCAP, 'M');
    ASSERT_THAT(this->testSubject.unsafe_assign(temp.c_str()), Eq(true));

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 1U];
    for (uint64_t i = 0U; i < STRINGCAP; ++i)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testCharArray[i] = 'L';
    }
    testCharArray[STRINGCAP] = '\0';
    EXPECT_THAT(this->testSubject.compare(testCharArray), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareWithEmptyCharArrayResultsInPositive)
{
    ::testing::Test::RecordProperty("TEST_ID", "a871dfe3-4acd-437c-b315-de3c43ece19b");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 1U] = {'\0'};
    EXPECT_THAT(this->testSubject.compare(testCharArray), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareEqStringAndCharArrayWithDifferentCapaResultsInZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "d0e86a0c-f68d-4d88-8e8b-f65f82a1e7aa");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string temp(STRINGCAP, 'M');
    ASSERT_THAT(this->testSubject.unsafe_assign(temp.c_str()), Eq(true));

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 2U];
    for (uint64_t i = 0U; i < STRINGCAP; ++i)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testCharArray[i] = 'M';
    }
    testCharArray[STRINGCAP] = '\0';
    EXPECT_THAT(this->testSubject.compare(testCharArray), Eq(0));
}

TYPED_TEST(stringTyped_test, CompareWithCharArrayResultNegativeWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "3efb771d-88e5-4775-8ce8-ba14a4158930");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string temp(STRINGCAP, 'M');
    ASSERT_THAT(this->testSubject.unsafe_assign(temp.c_str()), Eq(true));

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 2U];
    for (uint64_t i = 0U; i < STRINGCAP + 1U; ++i)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testCharArray[i] = 'M';
    }
    testCharArray[STRINGCAP + 1U] = '\0';
    EXPECT_THAT(this->testSubject.compare(testCharArray), Lt(0));
}

TYPED_TEST(stringTyped_test, CompareWithCharArrayResultPositiveWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "c2c627d5-c633-4368-b7dc-f674315553f7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string temp(STRINGCAP + 1U, 'M');
    string<STRINGCAP + 1U> sut;
    ASSERT_THAT(sut.unsafe_assign(temp.c_str()), Eq(true));

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP];
    for (uint64_t i = 0U; i < STRINGCAP - 1U; ++i)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testCharArray[i] = 'M';
    }
    testCharArray[STRINGCAP - 1U] = '\0';
    EXPECT_THAT(sut.compare(testCharArray), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareWithEmptyCharArrayOfDifferentCapaResultsInPositive)
{
    ::testing::Test::RecordProperty("TEST_ID", "7fe1a772-6004-470e-8355-e54156571fa0");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> sut("M");

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP] = {'\0'};
    EXPECT_THAT(sut.compare(testCharArray), Gt(0));
}

/// @note bool operator==(const T& rhs) const noexcept
/// bool operator!=(const T& rhs) const noexcept
/// bool operator==(const T& lhs, const string<Capacity>& rhs) noexcept
/// bool operator!=(const T& lhs, const string<Capacity>& rhs) noexcept
/// with T = {char array}
TYPED_TEST(stringTyped_test, CheckForEqualityWithEqualStringsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "38d8e0ca-97c5-4e3f-9cb7-589bb7de3b71");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    const char testCharArray[STRINGCAP + 1U] = {'M'};
    EXPECT_THAT(this->testSubject == testCharArray, Eq(true));
    EXPECT_THAT(testCharArray == this->testSubject, Eq(true));
    EXPECT_THAT(this->testSubject != testCharArray, Eq(false));
    EXPECT_THAT(testCharArray != this->testSubject, Eq(false));
}

TYPED_TEST(stringTyped_test, CheckForEqualityWithUnequalStringsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "129d4f55-71cd-4b6a-b07e-1f53069c117b");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 1U] = {'L'};
    EXPECT_THAT(this->testSubject == testCharArray, Eq(false));
    EXPECT_THAT(testCharArray == this->testSubject, Eq(false));
    EXPECT_THAT(this->testSubject != testCharArray, Eq(true));
    EXPECT_THAT(testCharArray != this->testSubject, Eq(true));
}

TYPED_TEST(stringTyped_test, CheckForEqualityWithEqualStringWithDifferentCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "a01cc09b-fe94-42db-bcbd-4f813f4acd62");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 5U] = {'M'};
    EXPECT_THAT(this->testSubject == testCharArray, Eq(true));
    EXPECT_THAT(testCharArray == this->testSubject, Eq(true));
    EXPECT_THAT(this->testSubject != testCharArray, Eq(false));
    EXPECT_THAT(testCharArray != this->testSubject, Eq(false));

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    const char test[] = {'M'};
    EXPECT_THAT(test == this->testSubject, Eq(true));
}

TYPED_TEST(stringTyped_test, CheckForEqualityWithUnequalStringWithDifferentSizeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d678927a-f629-433c-ad10-1d62338c816b");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 3U] = {'M', 'L'};
    EXPECT_THAT(this->testSubject == testCharArray, Eq(false));
    EXPECT_THAT(testCharArray == this->testSubject, Eq(false));
    EXPECT_THAT(this->testSubject != testCharArray, Eq(true));
    EXPECT_THAT(testCharArray != this->testSubject, Eq(true));
}

/// @note bool operator<(const T& rhs) const noexcept
/// bool operator<=(const T& rhs) const noexcept
/// bool operator>(const T& rhs) const noexcept
/// bool operator>=(const T& rhs) const noexcept
/// bool operator<(const T& lhs, const string<Capacity>& rhs) noexcept
/// bool operator<=(const T& lhs, const string<Capacity>& rhs) noexcept
/// bool operator>(const T& lhs, const string<Capacity>& rhs) noexcept
/// bool operator>=(const T& lhs, const string<Capacity>& rhs) noexcept
/// with T = {char array}
TYPED_TEST(stringTyped_test, CompareOperatorsWithDifferentStrings)
{
    ::testing::Test::RecordProperty("TEST_ID", "9dcd5cce-ce7d-4cf9-8c36-edca46d09ff7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 1U] = {'L'};

    // compare with greater string
    string<STRINGCAP> sutGreater("M");

    EXPECT_THAT(sutGreater < testCharArray, Eq(false));
    EXPECT_THAT(sutGreater <= testCharArray, Eq(false));
    EXPECT_THAT(sutGreater > testCharArray, Eq(true));
    EXPECT_THAT(sutGreater >= testCharArray, Eq(true));
    EXPECT_THAT(testCharArray < sutGreater, Eq(true));
    EXPECT_THAT(testCharArray <= sutGreater, Eq(true));
    EXPECT_THAT(testCharArray > sutGreater, Eq(false));
    EXPECT_THAT(testCharArray >= sutGreater, Eq(false));

    // compare with less string
    string<STRINGCAP> sutLess("F");

    EXPECT_THAT(sutLess < testCharArray, Eq(true));
    EXPECT_THAT(sutLess <= testCharArray, Eq(true));
    EXPECT_THAT(sutLess > testCharArray, Eq(false));
    EXPECT_THAT(sutLess >= testCharArray, Eq(false));
    EXPECT_THAT(testCharArray < sutLess, Eq(false));
    EXPECT_THAT(testCharArray <= sutLess, Eq(false));
    EXPECT_THAT(testCharArray > sutLess, Eq(true));
    EXPECT_THAT(testCharArray >= sutLess, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorsWithEqualStrings)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a07445e-8d0b-43f6-92b7-70a390ad6a27");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 1U] = {'M'};
    EXPECT_THAT(this->testSubject < testCharArray, Eq(false));
    EXPECT_THAT(this->testSubject <= testCharArray, Eq(true));
    EXPECT_THAT(this->testSubject > testCharArray, Eq(false));
    EXPECT_THAT(this->testSubject >= testCharArray, Eq(true));

    EXPECT_THAT(testCharArray < this->testSubject, Eq(false));
    EXPECT_THAT(testCharArray <= this->testSubject, Eq(true));
    EXPECT_THAT(testCharArray > this->testSubject, Eq(false));
    EXPECT_THAT(testCharArray >= this->testSubject, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorsWithDifferentStringWithDifferentSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "6ee0d514-12b8-48e5-b19b-997af585fc95");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[STRINGCAP + 1U];
    for (uint64_t i = 0U; i < STRINGCAP; ++i)
    {
        // NOLINTJUSTIFICATION no other way to populate testCharArray
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        testCharArray[i] = 'L';
    }
    testCharArray[STRINGCAP] = '\0';

    const std::string testStdString = &testCharArray[0];

    // compare with greater string
    std::string temp1(STRINGCAP + 5U, 'M');
    string<STRINGCAP + 5U> sutGreater;
    ASSERT_THAT(sutGreater.unsafe_assign(temp1.c_str()), Eq(true));

    EXPECT_THAT(sutGreater < testCharArray, Eq(false));
    EXPECT_THAT(sutGreater <= testCharArray, Eq(false));
    EXPECT_THAT(sutGreater > testCharArray, Eq(true));
    EXPECT_THAT(sutGreater >= testCharArray, Eq(true));
    EXPECT_THAT(testCharArray < sutGreater, Eq(true));
    EXPECT_THAT(testCharArray <= sutGreater, Eq(true));
    EXPECT_THAT(testCharArray > sutGreater, Eq(false));
    EXPECT_THAT(testCharArray >= sutGreater, Eq(false));

    // compare with less string
    std::string temp2(STRINGCAP + 5U, 'F');
    string<STRINGCAP + 5U> sutLess;
    ASSERT_THAT(sutLess.unsafe_assign(temp2.c_str()), Eq(true));

    EXPECT_THAT(sutLess < testCharArray, Eq(true));
    EXPECT_THAT(sutLess <= testCharArray, Eq(true));
    EXPECT_THAT(sutLess > testCharArray, Eq(false));
    EXPECT_THAT(sutLess >= testCharArray, Eq(false));
    EXPECT_THAT(testCharArray < sutLess, Eq(false));
    EXPECT_THAT(testCharArray <= sutLess, Eq(false));
    EXPECT_THAT(testCharArray > sutLess, Eq(true));
    EXPECT_THAT(testCharArray >= sutLess, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorsWithEqualStringWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ccedecd-215c-4bd7-89ff-d0bb25c206bb");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string temp(STRINGCAP, 'M');
    ASSERT_THAT(this->testSubject.unsafe_assign(temp.c_str()), Eq(true));

    constexpr uint64_t TEST_CHAR_ARRAY_CAPACITY = STRINGCAP + 6U;
    // NOLINTJUSTIFICATION required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testCharArray[TEST_CHAR_ARRAY_CAPACITY];
    for (auto& c : testCharArray)
    {
        c = 'M';
    }
    testCharArray[STRINGCAP] = '\0';
    EXPECT_THAT(this->testSubject < testCharArray, Eq(false));
    EXPECT_THAT(this->testSubject <= testCharArray, Eq(true));
    EXPECT_THAT(this->testSubject > testCharArray, Eq(false));
    EXPECT_THAT(this->testSubject >= testCharArray, Eq(true));

    EXPECT_THAT(testCharArray < this->testSubject, Eq(false));
    EXPECT_THAT(testCharArray <= this->testSubject, Eq(true));
    EXPECT_THAT(testCharArray > this->testSubject, Eq(false));
    EXPECT_THAT(testCharArray >= this->testSubject, Eq(true));
}

/// @note int64_t compare(char other) const noexcept
TYPED_TEST(stringTyped_test, CompareEqCharResultsInZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "94837615-8171-4da4-8157-19b4f8f170d1");
    this->testSubject = "M";
    const char testChar = 'M';
    EXPECT_THAT(this->testSubject.compare(testChar), Eq(0));
}

TYPED_TEST(stringTyped_test, CompareWithCharResultNegative)
{
    ::testing::Test::RecordProperty("TEST_ID", "b84785ae-3162-4ff1-a9b0-24405c4b381e");
    this->testSubject = "L";
    const char testChar = 'M';
    EXPECT_THAT(this->testSubject.compare(testChar), Lt(0));
}

TYPED_TEST(stringTyped_test, CompareWithCharResultPositive)
{
    ::testing::Test::RecordProperty("TEST_ID", "3339c92d-a45e-4f95-9825-d9699e2bc734");
    this->testSubject = "M";
    const char testChar = 'L';
    EXPECT_THAT(this->testSubject.compare(testChar), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareWithCharResultPositiveWithDifferentSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "93e57b3b-0c17-4314-bcfb-26f776519d44");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string temp(STRINGCAP + 3U, 'M');
    string<STRINGCAP + 3U> sut;
    ASSERT_THAT(sut.unsafe_assign(temp.c_str()), Eq(true));

    char testChar1 = 'L';
    EXPECT_THAT(sut.compare(testChar1), Gt(0));
    char testChar2 = 'M';
    EXPECT_THAT(sut.compare(testChar2), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareEmptyStringWithCharWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d0857725-4f4a-4052-a957-71fc906d535e");
    EXPECT_THAT(this->testSubject.compare('A'), Lt(0));
    EXPECT_THAT(this->testSubject.compare('\0'), Lt(0));
}

/// @note bool operator==(const char& rhs) const noexcept
/// bool operator!=(const char& rhs) const noexcept
TYPED_TEST(stringTyped_test, CheckForEqualityWithEqualCharWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d4e79679-939f-4aa9-9db1-4b3ba38b5dc7");
    this->testSubject = "M";

    const char testChar = 'M';
    EXPECT_THAT(this->testSubject == testChar, Eq(true));
    EXPECT_THAT(testChar == this->testSubject, Eq(true));
    EXPECT_THAT(this->testSubject != testChar, Eq(false));
    EXPECT_THAT(testChar != this->testSubject, Eq(false));
}

TYPED_TEST(stringTyped_test, CheckForEqualityWithUnequalCharWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1547d1dd-bf34-45ec-be4e-64fdc7164404");
    this->testSubject = "M";

    char testChar = 'L';
    EXPECT_THAT(this->testSubject == testChar, Eq(false));
    EXPECT_THAT(testChar == this->testSubject, Eq(false));
    EXPECT_THAT(this->testSubject != testChar, Eq(true));
    EXPECT_THAT(testChar != this->testSubject, Eq(true));
}

TYPED_TEST(stringTyped_test, CheckForEqualityWithCharWithDifferentSizeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c49cc0a3-b31c-49ec-90e2-707216df5eaa");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string temp(STRINGCAP + 4U, 'M');
    string<STRINGCAP + 4U> sut;
    ASSERT_THAT(sut.unsafe_assign(temp.c_str()), Eq(true));

    char testChar = 'M';
    EXPECT_THAT(sut == testChar, Eq(false));
    EXPECT_THAT(testChar == sut, Eq(false));
    EXPECT_THAT(sut != testChar, Eq(true));
    EXPECT_THAT(testChar != sut, Eq(true));
}

/// @note bool operator<(const char& rhs) const noexcept
/// bool operator<=(const char& rhs) const noexcept
/// bool operator>(const char& rhs) const noexcept
/// bool operator>=(const char& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorsWithDifferentChar)
{
    ::testing::Test::RecordProperty("TEST_ID", "c818b150-b926-4f3c-8405-6327303f12f6");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();

    char testChar = 'L';

    // compare testChar with greater string
    string<STRINGCAP> sutGreaterTestChar("M");

    EXPECT_THAT(sutGreaterTestChar < testChar, Eq(false));
    EXPECT_THAT(sutGreaterTestChar <= testChar, Eq(false));
    EXPECT_THAT(sutGreaterTestChar > testChar, Eq(true));
    EXPECT_THAT(sutGreaterTestChar >= testChar, Eq(true));
    EXPECT_THAT(testChar < sutGreaterTestChar, Eq(true));
    EXPECT_THAT(testChar <= sutGreaterTestChar, Eq(true));
    EXPECT_THAT(testChar > sutGreaterTestChar, Eq(false));
    EXPECT_THAT(testChar >= sutGreaterTestChar, Eq(false));


    // compare testChar with less string
    string<STRINGCAP> sutLessTestChar("F");

    EXPECT_THAT(sutLessTestChar < testChar, Eq(true));
    EXPECT_THAT(sutLessTestChar <= testChar, Eq(true));
    EXPECT_THAT(sutLessTestChar > testChar, Eq(false));
    EXPECT_THAT(sutLessTestChar >= testChar, Eq(false));
    EXPECT_THAT(testChar < sutLessTestChar, Eq(false));
    EXPECT_THAT(testChar <= sutLessTestChar, Eq(false));
    EXPECT_THAT(testChar > sutLessTestChar, Eq(true));
    EXPECT_THAT(testChar >= sutLessTestChar, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorsWithEqualChar)
{
    ::testing::Test::RecordProperty("TEST_ID", "6d6344a8-d8d6-426d-b288-b429eb9a6eca");
    this->testSubject = "M";

    char testChar = 'M';
    EXPECT_THAT(this->testSubject < testChar, Eq(false));
    EXPECT_THAT(this->testSubject <= testChar, Eq(true));
    EXPECT_THAT(this->testSubject > testChar, Eq(false));
    EXPECT_THAT(this->testSubject >= testChar, Eq(true));

    EXPECT_THAT(testChar < this->testSubject, Eq(false));
    EXPECT_THAT(testChar <= this->testSubject, Eq(true));
    EXPECT_THAT(testChar > this->testSubject, Eq(false));
    EXPECT_THAT(testChar >= this->testSubject, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorsWithDifferentCharWithDifferentSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "c9f2e472-3fbf-4e08-a0ee-6bb725e7c0d7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();

    char testChar = 'L';

    // compare testChar with greater string
    std::string temp1(STRINGCAP + 5U, 'M');
    string<STRINGCAP + 5U> sutGreaterTestChar;
    ASSERT_THAT(sutGreaterTestChar.unsafe_assign(temp1.c_str()), Eq(true));

    EXPECT_THAT(sutGreaterTestChar < testChar, Eq(false));
    EXPECT_THAT(sutGreaterTestChar <= testChar, Eq(false));
    EXPECT_THAT(sutGreaterTestChar > testChar, Eq(true));
    EXPECT_THAT(sutGreaterTestChar >= testChar, Eq(true));
    EXPECT_THAT(testChar < sutGreaterTestChar, Eq(true));
    EXPECT_THAT(testChar <= sutGreaterTestChar, Eq(true));
    EXPECT_THAT(testChar > sutGreaterTestChar, Eq(false));
    EXPECT_THAT(testChar >= sutGreaterTestChar, Eq(false));

    // compare testChar with less string
    std::string temp2(STRINGCAP + 5U, 'F');
    string<STRINGCAP + 5U> sutLessTestChar;
    ASSERT_THAT(sutLessTestChar.unsafe_assign(temp2.c_str()), Eq(true));

    EXPECT_THAT(sutLessTestChar < testChar, Eq(true));
    EXPECT_THAT(sutLessTestChar <= testChar, Eq(true));
    EXPECT_THAT(sutLessTestChar > testChar, Eq(false));
    EXPECT_THAT(sutLessTestChar >= testChar, Eq(false));
    EXPECT_THAT(testChar < sutLessTestChar, Eq(false));
    EXPECT_THAT(testChar <= sutLessTestChar, Eq(false));
    EXPECT_THAT(testChar > sutLessTestChar, Eq(true));
    EXPECT_THAT(testChar >= sutLessTestChar, Eq(true));

    // compare testChar with equal string
    std::string temp3(STRINGCAP + 5U, 'L');
    string<STRINGCAP + 5U> sutEqualTestChar;
    ASSERT_THAT(sutEqualTestChar.unsafe_assign(temp3.c_str()), Eq(true));

    EXPECT_THAT(sutEqualTestChar < testChar, Eq(false));
    EXPECT_THAT(sutEqualTestChar <= testChar, Eq(false));
    EXPECT_THAT(sutEqualTestChar > testChar, Eq(true));
    EXPECT_THAT(sutEqualTestChar >= testChar, Eq(true));
    EXPECT_THAT(testChar < sutEqualTestChar, Eq(true));
    EXPECT_THAT(testChar <= sutEqualTestChar, Eq(true));
    EXPECT_THAT(testChar > sutEqualTestChar, Eq(false));
    EXPECT_THAT(testChar >= sutEqualTestChar, Eq(false));
}

/// @note constexpr bool empty() const noexcept
TYPED_TEST(stringTyped_test, NewlyCreatedStringIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "b76d10c1-46e2-4bc3-ab79-af187b85d584");
    using MyString = typename TestFixture::stringType;
    MyString sut;
    EXPECT_THAT(sut.empty(), Eq(true));
}

TYPED_TEST(stringTyped_test, StringWithContentIsNotEmtpy)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb7fd1ff-82dc-4ae2-af70-9b080eb2265d");
    using MyString = typename TestFixture::stringType;
    MyString sut(TruncateToCapacity, "Dr.SchluepferStrikesAgain!");
    EXPECT_THAT(sut.empty(), Eq(false));
}

/// @note void clear() noexcept
TYPED_TEST(stringTyped_test, ClearEmptyStringDoesNotChangeString)
{
    ::testing::Test::RecordProperty("TEST_ID", "5109413a-6067-4f7f-ac35-8dd3a33a641f");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();

    this->testSubject.clear();
    EXPECT_THAT(this->testSubject.empty(), Eq(true));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
}

TYPED_TEST(stringTyped_test, ClearNotEmptyStringResultsInEmptyStringWithUnchangedCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "d25d2a74-6ea0-4892-b072-02177b31309e");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();
    this->testSubject = "M";
    ASSERT_THAT(this->testSubject.empty(), Eq(false));

    this->testSubject.clear();
    EXPECT_THAT(this->testSubject.empty(), Eq(true));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
}

TYPED_TEST(stringTyped_test, ChangeStringAfterClearWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "58444d00-ec85-464f-8269-7838823f04c2");
    this->testSubject.clear();
    this->testSubject = "M";
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

/// @note template <uint64_t N>
/// string(const string<N>& other) noexcept;
TYPED_TEST(stringTyped_test, ConstrWithEmptyStringWithSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "94bb789d-a98e-4eaa-8255-029a6d837d7c");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> testString(this->testSubject);
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(testString.size(), Eq(0U));
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
}

TYPED_TEST(stringTyped_test, ConstrWithStringWithSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "cebe491f-abaa-4473-8417-6c3264620f61");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    string<STRINGCAP + 1U> testString(this->testSubject);
    EXPECT_THAT(testString.c_str(), StrEq("M"));
    EXPECT_THAT(testString.size(), Eq(1U));
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
}

/// @note template <uint64_t N>
/// string(string<N>&& other) noexcept;
TYPED_TEST(stringTyped_test, MoveConstrWithEmptyStringWithSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "76527963-8483-4689-816e-01e6803e078a");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 30U> testString(std::move(this->testSubject));
    EXPECT_THAT(testString.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveConstrWithStringSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4bc0b719-080b-4345-9875-a74d0b5cf77d");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP + 11U> fuu(std::move(this->testSubject));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

/// @note template <uint64_t N>
/// string& operator=(const string<N>& rhs) noexcept;
TYPED_TEST(stringTyped_test, AssignmentOfStringWithSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "983c0fd2-6224-4712-9704-01367f1cd2c6");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> testString;
    testString = this->testSubject;
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(testString.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
}

TYPED_TEST(stringTyped_test, AssignmentOfEmptyStringWithSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c2361b47-cc22-458e-b9bc-e295324c3216");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> testString("M");
    testString = this->testSubject;
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(testString.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
}

TYPED_TEST(stringTyped_test, AssignmentOfNotEmptyStringWithSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "66eaa759-fe92-4b94-9c1c-371f2c4fc2bf");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    string<STRINGCAP + 30U> testString("Ferdinand Spitzschnueffler");
    testString = this->testSubject;
    EXPECT_THAT(testString.c_str(), StrEq("M"));
    EXPECT_THAT(testString.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
}

/// @note template <uint64_t N>
/// string& operator=(string<N>&& rhs) noexcept;
TYPED_TEST(stringTyped_test, MoveAssignmentOfEmptyStringWithSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0703614e-88a6-4f5f-80c7-f0d7e51736f6");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 63U> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfStringWithSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7c8713cb-d018-448b-9df7-880f1e765474");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    string<STRINGCAP + 36U> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfNotEmptyStringWithSmallerCapaWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "74811de5-eab2-44b7-9a80-6022e5d90ce0");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    string<STRINGCAP + 30U> testString("Jean-Luc Picard");
    testString = std::move(this->testSubject);
    EXPECT_THAT(testString.c_str(), StrEq("M"));
    EXPECT_THAT(testString.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
}

/// @note template <typename T1, typename T2, typename... Targs>
/// string<internal::SumCapa<T1, T2, Targs...>::value> concatenate(const T1& t1, const T2& t2, const Targs&... targs)
TYPED_TEST(stringTyped_test, ConcatenateTwoEmptyStringsReturnsEmptyStringWithTotalCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "ea0dea87-1bf7-44f9-9a4a-78ea2f0d834b");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> testString1;
    auto testString2 = concatenate(this->testSubject, testString1);

    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(testString1.capacity(), Eq(STRINGCAP + 1U));
    EXPECT_THAT(testString1.size(), Eq(0U));
    EXPECT_THAT(testString1.c_str(), StrEq(""));
    EXPECT_THAT(testString2.capacity(), Eq(2U * STRINGCAP + 1U));
    EXPECT_THAT(testString2.size(), Eq(0U));
    EXPECT_THAT(testString2.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, ConcatenateTwoStringsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "783fec4d-6ab8-455c-b8a4-5eacff5865ab");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString.c_str()), Eq(true));
    string<STRINGCAP + 2U> testString1;
    auto testString2 = concatenate(testString1, this->testSubject);

    EXPECT_THAT(testString2.capacity(), Eq(2U * STRINGCAP + 2U));
    EXPECT_THAT(testString2.size(), Eq(STRINGCAP));
    EXPECT_THAT(testString2.c_str(), StrEq(testStdString));
}

TYPED_TEST(stringTyped_test, ConcatenateTwoNotEmptyStringsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "edc2f7a4-758c-47c0-bf66-283df11f636f");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testStdString0(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString0.c_str()), Eq(true));
    std::string testStdString1(STRINGCAP + 3U, 'L');
    string<STRINGCAP + 3U> testString1(TruncateToCapacity, testStdString1.c_str(), testStdString1.size());
    auto testString2 = concatenate(this->testSubject, testString1);

    EXPECT_THAT(testString2.capacity(), Eq(2U * STRINGCAP + 3U));
    EXPECT_THAT(testString2.size(), Eq(this->testSubject.size() + testString1.size()));
    EXPECT_THAT(testString2.c_str(), StrEq(testStdString0 + testStdString1));
}

TYPED_TEST(stringTyped_test, ConcatenateThreeStringsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4f825a17-ae54-4478-8cad-e6a73f9d7801");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testString1("A");
    string<STRINGCAP + 2U> testString2("YOD");
    auto testString3 = concatenate(testString2, this->testSubject, testString1);

    std::string cmpString = std::string(testString2.c_str(), testString2.size())
                            + std::string(this->testSubject.c_str(), this->testSubject.size())
                            + std::string(testString1.c_str(), testString1.size());
    EXPECT_THAT(testString3.capacity(), Eq(3U * STRINGCAP + 2U));
    EXPECT_THAT(testString3.size(), Eq(cmpString.size()));
    EXPECT_THAT(testString3.c_str(), StrEq(cmpString));
}

TYPED_TEST(stringTyped_test, ConcatenateEmptyStringAndStringLiteralWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e3eb16d0-1aec-40dc-8221-025e844d6dc8");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    auto testString = concatenate(this->testSubject, "M");
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 1U));
    EXPECT_THAT(testString.size(), Eq(1U));
    EXPECT_THAT(testString.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, ConcatenateStringLiteralAndStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "727d85a2-73c4-4524-89d9-e4b8ebcdac0f");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "S";
    auto testString = concatenate("Ferdinand", this->testSubject);
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 9U));
    EXPECT_THAT(testString.size(), Eq(10));
    EXPECT_THAT(testString.c_str(), StrEq("FerdinandS"));
}

TEST(StringLiteralConcatenation, ConcatenateOnlyStringLiteralsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f591fb69-a8fa-4d8e-b42d-29573b4a20f7");
    auto testString = concatenate("Ferdi", "nandSpitzschnu", "ef", "fler");
    EXPECT_THAT(testString.capacity(), Eq(25U));
    EXPECT_THAT(testString.size(), Eq(25U));
    EXPECT_THAT(testString.c_str(), StrEq("FerdinandSpitzschnueffler"));
}

TYPED_TEST(stringTyped_test, ConcatenateEmptyStringAndNullCharReturnsStringWithSizeOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a4374fb-b21f-4465-95dc-22b898079c81");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();

    auto result1 = concatenate(this->testSubject, '\0');

    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.empty(), Eq(true));
    EXPECT_THAT(result1.capacity(), Eq(STRINGCAP + 1U));
    EXPECT_THAT(result1.size(), Eq(1U));
    EXPECT_THAT(result1.empty(), Eq(false));
    EXPECT_THAT(result1[0U], Eq('\0'));

    auto result2 = concatenate('\0', this->testSubject);

    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.empty(), Eq(true));
    EXPECT_THAT(result2.capacity(), Eq(STRINGCAP + 1U));
    EXPECT_THAT(result2.size(), Eq(1U));
    EXPECT_THAT(result2.empty(), Eq(false));
    EXPECT_THAT(result2[0U], Eq('\0'));
}

TYPED_TEST(stringTyped_test, ConcatenateEmptyStringAndCharWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f99022c0-1ea9-4c63-bb18-d0ec53020e66");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();

    auto result1 = concatenate('M', this->testSubject);

    EXPECT_THAT(result1.capacity(), Eq(STRINGCAP + 1U));
    EXPECT_THAT(result1.size(), Eq(1U));
    EXPECT_THAT(result1.c_str(), StrEq("M"));

    auto result2 = concatenate(this->testSubject, 'M');

    EXPECT_THAT(result2.capacity(), Eq(STRINGCAP + 1U));
    EXPECT_THAT(result2.size(), Eq(1U));
    EXPECT_THAT(result2.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, ConcatenateStringAndCharWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d8cf335a-4fa2-4dc5-a98d-8b34f3b47464");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    const string<STRINGCAP + 26U> expectedString("FerdinandSpitzschnueffler");
    string<STRINGCAP + 10U> testString1("Ferdinand");
    const char testChar = 'S';
    string<15U> testString2("pitzschnueffler");
    auto result = concatenate(testString1, testChar, testString2);

    EXPECT_THAT(result.capacity(), Eq(expectedString.capacity()));
    EXPECT_THAT(result.size(), Eq(expectedString.size()));
    EXPECT_THAT(result.c_str(), StrEq(expectedString.c_str()));
}

TEST(CharConcatenation, ConcatenateOnlyCharsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "32900bfd-69c9-4116-b702-9d2165bd54b0");
    auto testString = concatenate('W', 'o', 'o', 'h', 'o', 'o');
    EXPECT_THAT(testString.capacity(), Eq(6U));
    EXPECT_THAT(testString.size(), Eq(6U));
    EXPECT_THAT(testString.c_str(), StrEq("Woohoo"));
}

/// @note template <typename T1, typename T2>
/// string<internal::GetCapa<T1>::capa + internal::GetCapa<T2>::capa> operator+(const T1& t1, const T2& t2);
TYPED_TEST(stringTyped_test, ConcatenateEmptyStringsReturnsEmptyString)
{
    ::testing::Test::RecordProperty("TEST_ID", "39fedfb3-f3fd-477a-8499-f6f4c63c46dd");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    auto testString = this->testSubject + this->testSubject;
    EXPECT_THAT(testString.capacity(), Eq(2U * STRINGCAP));
    EXPECT_THAT(testString.size(), Eq(0U));
    EXPECT_THAT(testString.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, ConcatenateStringsWithOperatorPlusWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a792faf-f944-49f2-980f-dc58d3aedd40");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString.c_str()), Eq(true));
    string<STRINGCAP + 2U> testString1;
    string<2U * STRINGCAP + 2U> testString2;
    testString2 = testString1 + this->testSubject;
    EXPECT_THAT(testString2.capacity(), Eq(2U * STRINGCAP + 2U));
    EXPECT_THAT(testString2.size(), Eq(STRINGCAP));
    EXPECT_THAT(testString2.c_str(), StrEq(testStdString));
}

TYPED_TEST(stringTyped_test, ConcatenateNotEmptyStringsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "161902aa-98a0-450f-874e-f300a99a144d");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testStdString0(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString0.c_str()), Eq(true));
    std::string testStdString1(STRINGCAP + 3U, 'L');
    string<STRINGCAP + 3U> testString1(TruncateToCapacity, testStdString1.c_str(), testStdString1.size());
    string<6U * STRINGCAP> testString2 = this->testSubject + testString1 + this->testSubject;
    EXPECT_THAT(testString2.capacity(), Eq(6U * STRINGCAP));
    EXPECT_THAT(testString2.size(), Eq(2U * this->testSubject.size() + testString1.size()));
    EXPECT_THAT(testString2.c_str(), StrEq(testStdString0 + testStdString1 + testStdString0));
}

TYPED_TEST(stringTyped_test, ConcatenateEmptyStringAndStringLiteralWithOperatorPlusWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f87151cd-ef6c-4f51-a986-7f238d608678");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<2U * STRINGCAP> testString1 = this->testSubject + "M";
    EXPECT_THAT(testString1.capacity(), Eq(2U * STRINGCAP));
    EXPECT_THAT(testString1.size(), Eq(1U));
    EXPECT_THAT(testString1.c_str(), StrEq("M"));

    // required to verify string literal functionality of iox::string
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    char testChar[3] = "ab";
    testChar[2] = 'c';
    string<3U * STRINGCAP> testString2 = this->testSubject + testChar;
    EXPECT_THAT(testString2.capacity(), Eq(3U * STRINGCAP));
    EXPECT_THAT(testString2.size(), Eq(3U));
    EXPECT_THAT(testString2.c_str(), StrEq("abc"));
}

TYPED_TEST(stringTyped_test, ConcatenateStringLiteralAndStringWithOperatorPlusWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d6630140-592b-45c7-ab76-be8cd3fa3d98");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "e";
    string<STRINGCAP + 7U> testString = "AdmTass" + this->testSubject;
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 7U));
    EXPECT_THAT(testString.size(), Eq(8U));
    EXPECT_THAT(testString.c_str(), StrEq("AdmTasse"));
}

TYPED_TEST(stringTyped_test, ConcatenateEmptyStringAndCharWithOperatorPlusWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "09efee5c-985f-4852-aa3c-5c67fc61071b");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<2U * STRINGCAP> testString = this->testSubject + 'M';
    EXPECT_THAT(testString.capacity(), Eq(2U * STRINGCAP));
    EXPECT_THAT(testString.size(), Eq(1U));
    EXPECT_THAT(testString.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, ConcatenateCharAndStringWithOperatorPlusWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d25ab82-48f9-4000-9764-e14522fd4ecc");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "S";
    string<STRINGCAP + 7U> testString = 'F' + this->testSubject;
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 7U));
    EXPECT_THAT(testString.size(), Eq(2U));
    EXPECT_THAT(testString.c_str(), StrEq("FS"));
}

TEST(String10, ConcatenateSeveralCharsAndStringsWithOperatorPlusWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "68e69585-fe83-497c-9249-59644501abd1");
    const string<3U> testString1("Hyp");
    const char testChar1 = 'n';
    const string<2U> testString2("ot");
    const char testChar2 = 'o';
    const string<2U> testString3("ad");
    auto result = testString1 + testChar1 + testString2 + testChar2 + testString3;

    EXPECT_THAT(result.capacity(), Eq(9U));
    EXPECT_THAT(result.size(), Eq(9U));
    EXPECT_THAT(result.c_str(), StrEq("Hypnotoad"));
}

/// @note template <typename T>
/// bool unsafe_append(const T& t) noexcept;
TYPED_TEST(stringTyped_test, UnsafeAppendEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "965e3b39-2cda-4816-9d64-ccf0d4e6f761");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    string<2U * STRINGCAP> testString;
    EXPECT_THAT(this->testSubject.unsafe_append(testString), Eq(true));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, UnsafeAppendFittingStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b609c480-c1dd-49d5-b230-09ccda1b0e78");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "2";
    string<5U * STRINGCAP> testString("R2-D");
    EXPECT_THAT(testString.unsafe_append(this->testSubject), Eq(true));
    EXPECT_THAT(testString.capacity(), Eq(5U * STRINGCAP));
    EXPECT_THAT(testString.size(), Eq(5));
    EXPECT_THAT(testString.c_str(), StrEq("R2-D2"));
}

TYPED_TEST(stringTyped_test, UnsafeAppendTooLargeStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a572827-b778-4e7a-a79f-c62b48928a81");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    string<2U * STRINGCAP> testString;
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(testString.unsafe_assign(testStdString.c_str()), Eq(true));

    EXPECT_THAT(this->testSubject.unsafe_append(testString), Eq(false));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, UnsafeAppendEmptyStringLiteralWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5725ff86-b3db-4a43-9278-fc358628a1fc");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    EXPECT_THAT(this->testSubject.unsafe_append(""), Eq(true));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TEST(String10, UnsafeAppendFittingStringLiteralWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ed188a09-9550-4377-95f6-ca51f8cfbf7e");
    constexpr uint64_t STRINGCAP = 10U;
    string<STRINGCAP> testString("R2-D");
    EXPECT_THAT(testString.unsafe_append("2"), Eq(true));
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testString.size(), Eq(5U));
    EXPECT_THAT(testString.c_str(), StrEq("R2-D2"));
}

TEST(String10, UnsafeAppendTooLargeStringLiteralFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "f9305077-c3a8-4edb-be5f-ea8b1af19645");
    constexpr uint64_t STRINGCAP = 10U;
    string<STRINGCAP> testString("Kern");
    EXPECT_THAT(testString.unsafe_append("fusionsbaby"), Eq(false));
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testString.size(), Eq(4U));
    EXPECT_THAT(testString.c_str(), StrEq("Kern"));
}

TYPED_TEST(stringTyped_test, UnsafeAppendNullCharWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1a6743f-1146-4269-9c13-0baa1e14fbe4");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> testString("M");
    const char testChar = '\0';
    EXPECT_THAT(testString.unsafe_append(testChar), Eq(true));
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 1U));
    EXPECT_THAT(testString.size(), Eq(2U));
    EXPECT_THAT(testString.c_str(), StrEq("M"));
    EXPECT_THAT(testString[1U], Eq(testChar));
}

TYPED_TEST(stringTyped_test, UnsafeAppendCharWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4243b52a-fe02-45ec-b201-dde2a0dca5b8");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 5U> testString("R2-D");
    EXPECT_THAT(testString.unsafe_append('2'), Eq(true));
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 5U));
    EXPECT_THAT(testString.size(), Eq(5U));
    EXPECT_THAT(testString.c_str(), StrEq("R2-D2"));
}

TYPED_TEST(stringTyped_test, UnsafeAppendWithCharFailsWhenCapacityIsExceeded)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e92266b-d9dc-4751-a4e6-3be772a1855d");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string temp(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(temp.c_str()), Eq(true));

    EXPECT_THAT(this->testSubject.unsafe_append('L'), Eq(false));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(temp));

    EXPECT_THAT(this->testSubject.unsafe_append('\0'), Eq(false));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(temp));
}

TYPED_TEST(stringTyped_test, UnsafeAppendWithCharToEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d6965c7-b0f1-45d4-989a-c7349a53f8f7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    EXPECT_THAT(this->testSubject.unsafe_append('v'), Eq(true));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("v"));
}

/// @note template <typename T>
/// string& append(TruncateToCapacity_t, const T& t) noexcept
TYPED_TEST(stringTyped_test, AppendEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9f286751-a900-4fd4-be49-105c2de25265");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    string<STRINGCAP + 1U> testString;
    this->testSubject.append(TruncateToCapacity, testString);
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, AppendStringToEmptyStringResultsInConcatenatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb8651ac-efcf-430a-94e8-8f6e20dcd265");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 5U> testString("M");
    this->testSubject.append(TruncateToCapacity, testString);
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, AppendStringResultsInConcatenatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "9c7f7d12-4b5b-483b-acb4-0afbe6d59ec2");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "d";
    string<STRINGCAP + 5U> testString("Picar");
    testString.append(TruncateToCapacity, this->testSubject);
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 5U));
    EXPECT_THAT(testString.size(), Eq(6U));
    EXPECT_THAT(testString.c_str(), StrEq("Picard"));
}

TYPED_TEST(stringTyped_test, AppendTooLargeStringResultsInTruncatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "f4e3243d-744d-4f00-bdf5-118884fd42c0");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    string<STRINGCAP + 1U> testString;
    std::string testStdString(STRINGCAP + 1U, 'M');
    EXPECT_THAT(testString.unsafe_assign(testStdString.c_str()), Eq(true));
    this->testSubject.append(TruncateToCapacity, testString);
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testStdString.substr(0, STRINGCAP)));
}

TYPED_TEST(stringTyped_test, AppendEmptyStringLiteralWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f24afd76-6d2a-4d2f-8cf5-4883bbf5bb6b");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    this->testSubject.append(TruncateToCapacity, "");
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, AppendStringLiteralToEmptyStringResultsInConcatenatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "8ebaa145-0a91-4853-9320-2cab77a7f99f");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject.append(TruncateToCapacity, "M");
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TEST(String10, AppendStringLiteralResultsInConcatenatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "f620c504-8d0a-4088-9074-4a1be80ff077");
    constexpr uint64_t STRINGCAP = 10U;
    string<STRINGCAP> testString("Picar");
    testString.append(TruncateToCapacity, "d");
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testString.size(), Eq(6U));
    EXPECT_THAT(testString.c_str(), StrEq("Picard"));
}

TEST(String10, AppendTooLargeStringLiteralResultsInTruncatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "524a544a-0275-4549-a6dd-7c40e31af91c");
    constexpr uint64_t STRINGCAP = 10U;
    string<STRINGCAP> testString("Live long");
    testString.append(TruncateToCapacity, " and prosper");
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testString.size(), Eq(STRINGCAP));
    EXPECT_THAT(testString.c_str(), StrEq("Live long "));
}

TYPED_TEST(stringTyped_test, AppendStringContainingNullWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ed16b243-ddb3-46be-8bd8-2ed93fc7184d");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    constexpr auto RESULT_CAPACITY = STRINGCAP + 10U;
    const std::string expectedString{"ice\0ryx", 7};

    string<RESULT_CAPACITY> sut("i");
    const string<RESULT_CAPACITY> testCxxString(TruncateToCapacity, expectedString.substr(1).c_str(), 6U);
    const std::string testStdString = expectedString.substr(1);

    // append iox::string
    sut.append(TruncateToCapacity, testCxxString);
    EXPECT_THAT(sut.capacity(), Eq(RESULT_CAPACITY));
    EXPECT_THAT(sut.size(), Eq(7U));
    EXPECT_THAT(std::memcmp(sut.c_str(), expectedString.c_str(), sut.size()), Eq(0));
}

/// @note string& append(TruncateToCapacity_t, char c) noexcept
TYPED_TEST(stringTyped_test, AppendNullCharWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c20f1797-2797-4f4a-9acf-7bd1b9518c3d");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 1U> sut = "M";
    sut.append(TruncateToCapacity, '\0');
    EXPECT_THAT(sut.capacity(), Eq(STRINGCAP + 1U));
    EXPECT_THAT(sut.size(), Eq(2U));
    EXPECT_THAT(sut.c_str(), StrEq("M"));
    EXPECT_THAT(sut[1U], Eq('\0'));
}

TYPED_TEST(stringTyped_test, AppendCharToEmptyStringResultsInConcatenatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "7cf74a7a-a468-4efa-b54a-61d265de8808");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject.append(TruncateToCapacity, 'M');
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, AppendCharResultsInConcatenatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "853b6d2f-4e48-43f4-b167-53f076747ab2");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 5U> testString("Picar");
    testString.append(TruncateToCapacity, 'd');
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 5U));
    EXPECT_THAT(testString.size(), Eq(6U));
    EXPECT_THAT(testString.c_str(), StrEq("Picard"));
}

TYPED_TEST(stringTyped_test, AppendCharDoesNotChangeStringWhenCapacityIsExceeded)
{
    ::testing::Test::RecordProperty("TEST_ID", "3005b5af-28b5-452a-98dc-603f6973230f");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string temp(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(temp.c_str()), Eq(true));

    this->testSubject.append(TruncateToCapacity, 'L');
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(temp.substr(0, STRINGCAP)));
}

/// @note iox::cxx::optional<string<Capacity>> substr(uint64_t pos = 0) const noexcept;
TYPED_TEST(stringTyped_test, SubstrWithDefaultPosAndSizeResultsInWholeString)
{
    ::testing::Test::RecordProperty("TEST_ID", "da66bb36-2a1c-435b-8a47-874eb12315ef");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString.c_str()), Eq(true));
    auto res = this->testSubject.substr();
    ASSERT_THAT(res.has_value(), Eq(true));

    auto testSubstring = res.value();
    std::string testStdSubstring = testStdString.substr();
    EXPECT_THAT(testSubstring.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubstring.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring.c_str(), StrEq(testStdSubstring));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testStdString));
}

TEST(String100, SubstrWithDefaultSizeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "10b1244f-7245-449d-b6d0-5c94faa7d274");
    constexpr uint64_t STRINGCAP = 100U;
    constexpr uint64_t SUBSTR_POS = 8U;
    std::string testStdString = "Mueslimaedchen";
    std::string testStdSubstring = testStdString.substr(SUBSTR_POS);
    string<STRINGCAP> testCxxString(TruncateToCapacity, testStdString.c_str(), testStdString.size());
    auto res = testCxxString.substr(SUBSTR_POS);
    ASSERT_THAT(res.has_value(), Eq(true));

    auto testSubstring = res.value();
    EXPECT_THAT(testSubstring.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubstring.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring.c_str(), StrEq(testStdSubstring));
}

/// @note iox::cxx::optional<string<Capacity>> substr(uint64_t pos, uint64_t count) const noexcept
TEST(String100, SubstrWithValidPosAndSizeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3cd90af2-97f4-4767-854d-d3ca726bd348");
    constexpr uint64_t STRINGCAP = 100U;
    std::string testStdString = "Ferdinand Spitzschnueffler";
    string<STRINGCAP> testCxxString(TruncateToCapacity, testStdString.c_str(), testStdString.size());

    std::string testStdSubstring = testStdString.substr(0, 19);
    auto res1 = testCxxString.substr(0U, 19U);
    ASSERT_THAT(res1.has_value(), Eq(true));
    auto testSubstring1 = res1.value();
    EXPECT_THAT(testSubstring1.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubstring1.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring1.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(20, 5);
    auto res2 = testCxxString.substr(20U, 5U);
    EXPECT_THAT(res2.has_value(), Eq(true));
    auto testSubstring2 = res2.value();
    EXPECT_THAT(testSubstring2.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubstring2.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring2.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(0, 26);
    auto res3 = testCxxString.substr(0U, 26U);
    ASSERT_THAT(res3.has_value(), Eq(true));
    auto testSubstring3 = res3.value();
    EXPECT_THAT(testSubstring3.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubstring3.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring3.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(11, 8);
    auto res4 = testCxxString.substr(11U, 8U);
    ASSERT_THAT(res4.has_value(), Eq(true));
    auto testSubstring4 = res4.value();
    EXPECT_THAT(testSubstring4.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubstring4.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring4.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(13, 98);
    auto res5 = testCxxString.substr(13U, 98U);
    ASSERT_THAT(res5.has_value(), Eq(true));
    auto testSubstring5 = res5.value();
    EXPECT_THAT(testSubstring5.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubstring5.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring5.c_str(), StrEq(testStdSubstring));
}

TYPED_TEST(stringTyped_test, SubstrWithInvalidPosFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "2298f577-2974-4a01-82fb-a11d1f6faf6e");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    auto res = this->testSubject.substr(STRINGCAP + 1U, STRINGCAP + 2U);
    EXPECT_THAT(res.has_value(), Eq(false));
}

/// @note template <typename T>
/// iox::cxx::optional<uint64_t> find(const T& t, uint64_t pos = 0) const noexcept
TYPED_TEST(stringTyped_test, FindEmptyStringInEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ceb4ed6-1395-445e-afe4-94f6c9b2cee8");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 5U> testString;
    auto res = this->testSubject.find(testString);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0U));

    res = this->testSubject.find("");
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0U));
}

TYPED_TEST(stringTyped_test, FindStringInEmptyStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "feb48bb6-c6f0-4f8b-8ce5-bf881fd876cb");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 5U> testString("a");
    auto res = this->testSubject.find(testString);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = this->testSubject.find("a");
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindStringInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b487b6bc-1c40-46f1-8285-dd1660d2e882");
    string<10U> testString("R2-D2");
    string<100U> substring("2");
    auto res = testString.find(substring);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find(substring, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find(substring, 2U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));
}

TEST(String100, FindNotIncludedStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb0ca1b6-3dbd-491a-acac-e1f57ad2eb4f");
    constexpr uint64_t STRINGCAP = 100U;
    string<STRINGCAP> testString("Kernfusionsbaby");
    string<STRINGCAP> substring("abc");
    auto res = testString.find(substring);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find(substring, 0U);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find(substring, 50U);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindStringLiteralInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b9a3018f-10d5-4f57-bfeb-f536a6ea9642");
    constexpr uint64_t STRINGCAP = 100U;

    string<STRINGCAP> testString1("Mueslimaedchen");
    auto res = testString1.find("lima");
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    res = testString1.find("lima", 2U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    res = testString1.find("e", 10U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(12U));

    constexpr uint64_t STRING_COUNT = 7U;
    std::string testStdString{"ice\0ryx", STRING_COUNT};
    string<STRINGCAP> testString2(TruncateToCapacity, testStdString.c_str(), STRING_COUNT);
    res = testString2.find("e\0ry", 0U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(2U));
}

TEST(String100, FindNotIncludedStringLiteralFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "bd1ef66b-3698-4049-9d62-131b6de51b76");
    string<100U> testString("Kernfusionsbaby");
    auto res = testString.find("abc");
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find("abc", 50U);
    EXPECT_THAT(res.has_value(), Eq(false));
}

/// @note template <typename T>
/// iox::cxx::optional<uint64_t> find_first_of(const T& t, uint64_t pos = 0) const noexcept
TYPED_TEST(stringTyped_test, FindFirstOfFailsForEmptyStringInEmptyString)
{
    ::testing::Test::RecordProperty("TEST_ID", "21f90f13-6b15-4ce1-9258-c21154b6043c");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 5U> testString;
    auto res = this->testSubject.find_first_of(testString);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = this->testSubject.find_first_of("");
    EXPECT_THAT(res.has_value(), Eq(false));
}

TYPED_TEST(stringTyped_test, FindFirstOfForStringInEmptyStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a1b1272-fc5b-431e-980d-39357f66e882");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 5U> testString("a");
    auto res = this->testSubject.find_first_of(testString);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = this->testSubject.find_first_of("a");
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindFirstOfForStringInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b24dbd99-3595-4b04-9e6e-177d2a6e4ad0");
    constexpr uint64_t STRINGCAP = 10U;
    constexpr uint64_t SUB_STRINGCAP = 100U;
    string<STRINGCAP> testString("R2-D2");
    string<SUB_STRINGCAP> substring1("2");
    auto res = testString.find_first_of(substring1);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find_first_of(substring1, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find_first_of(substring1, 2U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    string<SUB_STRINGCAP> substring2("D3R");
    res = testString.find_first_of(substring2);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0U));

    res = testString.find_first_of(substring2, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(3U));
}

TEST(String100, FindFirstOfForNotIncludedStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "f42e026a-063b-43dd-8a6a-157330f3f426");
    constexpr uint64_t STRINGCAP = 100U;
    string<STRINGCAP> testString("Kernfusionsbaby");
    string<STRINGCAP> substring("cdG");
    auto res = testString.find_first_of(substring);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of(substring, 0U);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of(substring, 50U);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindFirstOfForStringLiteralInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "cfb3c539-c442-4a80-82b2-2da4ea37f1cd");
    constexpr uint64_t STRINGCAP = 100U;
    string<STRINGCAP> testString1("Mueslimaedchen");
    auto res = testString1.find_first_of("lima");
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    res = testString1.find_first_of("mali", 2U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    res = testString1.find_first_of("e", 10U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(12U));

    res = testString1.find_first_of("U3M");
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0U));

    constexpr uint64_t STRING_COUNT = 7U;
    std::string testStdString{"ice\0ryx", STRING_COUNT};
    string<STRINGCAP> testString2(TruncateToCapacity, testStdString.c_str(), STRING_COUNT);
    res = testString2.find_first_of("e\0ry", 0U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(2U));
}

TEST(String100, FindFirstOfForNotIncludedStringLiteralFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "bc574b5b-496b-4c81-acf9-a81fe3d2abb0");
    string<100U> testString("Kernfusionsbaby");
    auto res = testString.find_first_of("cd");
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of("cd", 0U);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of("cd", 50U);
    EXPECT_THAT(res.has_value(), Eq(false));
}

/// @note template <typename T>
/// iox::cxx::optional<uint64_t> find_last_of(const T& t, uint64_t pos = 0) const noexcept
TYPED_TEST(stringTyped_test, FindLastOfFailsForEmptyStringInEmptyString)
{
    ::testing::Test::RecordProperty("TEST_ID", "7e09947d-762f-4d7f-a1ab-65696877da06");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 5U> testString;
    auto res = this->testSubject.find_last_of(testString);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = this->testSubject.find_last_of("");
    EXPECT_THAT(res.has_value(), Eq(false));
}

TYPED_TEST(stringTyped_test, FindLastOfForStringInEmptyStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab1650d8-b3f1-445a-b53a-b339f9f37830");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 5U> testString("a");
    auto res = this->testSubject.find_last_of(testString);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = this->testSubject.find_last_of("a");
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindLastOfForStringInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "384b1457-ccaa-4801-a7be-625ef9b7a27a");
    constexpr uint64_t STRINGCAP = 10U;
    constexpr uint64_t SUB_STRINGCAP = 100U;
    string<STRINGCAP> testString("R2-D2");
    string<SUB_STRINGCAP> substring1("2");
    auto res = testString.find_last_of(substring1);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    res = testString.find_last_of(substring1, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find_last_of(substring1, 5U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    string<SUB_STRINGCAP> substring2("D3R");
    res = testString.find_last_of(substring2);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(3U));

    res = testString.find_last_of(substring2, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0U));
}

TEST(String100, FindLastOfForNotIncludedStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0aad222-061f-42ab-8d58-7c7fb5dc1296");
    constexpr uint64_t STRINGCAP = 100U;
    string<STRINGCAP> testString("Kernfusionsbaby");
    string<STRINGCAP> substring("cdG");
    auto res = testString.find_last_of(substring);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_last_of(substring, 0U);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_last_of(substring, 50U);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindLastOfForStringLiteralInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "cac2ed11-6f33-4e32-ac3f-fd725c068f42");
    string<100U> testString1("Mueslimaedchen");
    auto res = testString1.find_last_of("lima");
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(7U));

    res = testString1.find_last_of("lima", 5U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(5U));

    res = testString1.find_last_of("e", 7U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(2U));

    res = testString1.find_last_of("U3M");
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0U));
}

TEST(String100, FindLastOfForNotIncludedStringLiteralFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec2e64cf-91ef-4c9c-801d-4a27d26db240");
    string<100U> testString("Kernfusionsbaby");
    auto res = testString.find_last_of("cd");
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_last_of("cd", 0U);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_last_of("cd", 50U);
    EXPECT_THAT(res.has_value(), Eq(false));
}

/// @note constexpr char& at(const uint64_t pos) noexcept
TYPED_TEST(stringTyped_test, AccessPositionOfEmptyStringViaAtFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "89817818-f05a-4ceb-8663-9727d227048c");

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { this->testSubject.at(0U); },
                                              iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TYPED_TEST(stringTyped_test, AccessPositionOutOfBoundsViaAtFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "68035709-5f8d-4bcb-80ce-ad5619aba84a");

    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { this->testSubject.at(STRINGCAP); },
                                              iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TYPED_TEST(stringTyped_test, AccessFirstPositionOfNonEmptyStringViaAtReturnsCorrectCharacter)
{
    ::testing::Test::RecordProperty("TEST_ID", "8f096ca8-6951-43d4-86a8-3cf94de2977a");
    this->testSubject = "M";
    EXPECT_THAT(this->testSubject.at(0U), Eq('M'));
}

TYPED_TEST(stringTyped_test, AccessAndAssignToMaxPositionOfNotEmptyStringViaAtSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "9b9d106d-bfc5-40fe-876d-5beb1725e3fa");
    constexpr char START_CHARACTER = 'M';
    constexpr char NEW_CHARACTER = 'L';
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();

    std::string testSTDString(STRINGCAP, START_CHARACTER);
    ASSERT_THAT(this->testSubject.unsafe_assign(testSTDString.c_str()), Eq(true));

    this->testSubject.at(STRINGCAP - 1) = NEW_CHARACTER;
    testSTDString.at(STRINGCAP - 1) = NEW_CHARACTER;
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testSTDString));
}

/// @note constexpr const char& at(const uint64_t pos) const noexcept
TYPED_TEST(stringTyped_test, AccessPositionOfEmptyStringViaConstAtFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "5cf6d322-6ee9-41ce-bbf6-4e0d193fa938");

    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();
    const string<STRINGCAP> sut;

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut.at(0U); }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TYPED_TEST(stringTyped_test, AccessPositionOutOfBoundsViaConstAtFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "90a986f4-b29b-4ce7-ad55-79cc4b7b2b29");

    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();
    const string<STRINGCAP> sut;

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut.at(STRINGCAP); }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TYPED_TEST(stringTyped_test, AccessFirstPositionOfNotEmptyStringViaConstAtReturnsCorrectCharacter)
{
    ::testing::Test::RecordProperty("TEST_ID", "661fb674-3edf-4ac3-adb7-c7767153040d");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();
    const string<STRINGCAP> sut("M");
    EXPECT_THAT(sut.at(0U), Eq('M'));
}

TYPED_TEST(stringTyped_test, AccessMaxPositionOfNotEmptyStringViaConstAtSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "78f13867-8633-4d90-9f04-a9a47ab492b1");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();

    std::string testSTDString(STRINGCAP, 'M');
    const string<STRINGCAP> sut(TruncateToCapacity, testSTDString.c_str(), testSTDString.size());

    EXPECT_THAT(sut.at(STRINGCAP - 1), Eq('M'));
}

/// @note constexpr char& operator[](const uint64_t pos) noexcept
TYPED_TEST(stringTyped_test, AccessPositionOfEmptyStringViaSubscriptOperatorFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "95ced457-1aec-47e9-a496-0197ea3f4600");

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { this->testSubject[0U]; }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TYPED_TEST(stringTyped_test, AccessPositionOutOfBoundsViaSubscriptOperatorFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab52924e-1d6a-41e1-a8a9-8cfd9ab2120d");

    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { this->testSubject[STRINGCAP]; },
                                              iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TYPED_TEST(stringTyped_test, AccessFirstPositionOfNotEmptyStringViaSubscriptOperatorReturnsCorrectCharacter)
{
    ::testing::Test::RecordProperty("TEST_ID", "86a984b9-8345-4413-a25c-e9a1c74c5cef");
    this->testSubject = "L";
    EXPECT_THAT(this->testSubject[0U], Eq('L'));
}

TYPED_TEST(stringTyped_test, AccessAndAssignToMaxPositionOfNotEmptyStringViaSubscriptOperatorSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "9e2dd8ba-6053-4532-895b-f18a9ae0adf6");
    constexpr char START_CHARACTER = 'F';
    constexpr char NEW_CHARACTER = 'S';
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();

    std::string testSTDString(STRINGCAP, START_CHARACTER);
    ASSERT_THAT(this->testSubject.unsafe_assign(testSTDString.c_str()), Eq(true));

    this->testSubject[STRINGCAP - 1] = NEW_CHARACTER;
    testSTDString[STRINGCAP - 1] = NEW_CHARACTER;
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testSTDString));
}

/// @note constexpr const char& operator[](const uint64_t pos) const noexcept
TYPED_TEST(stringTyped_test, AccessPositionOfEmptyStringViaConstSubscriptOperatorFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ca75e53-8e26-4451-8712-a86bfe5bd32c");

    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();
    const string<STRINGCAP> sut;

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut[0U]; }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TYPED_TEST(stringTyped_test, AccessPositionOutOfBoundsViaConstSubscriptOperatorFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "5498e314-d321-464a-a667-400ee0c4d81f");

    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();
    const string<STRINGCAP> sut;

    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { sut[STRINGCAP]; }, iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

TYPED_TEST(stringTyped_test, AccessFirstPositionOfNotEmptyStringViaConstSubscriptOperatorReturnsCorrectCharacter)
{
    ::testing::Test::RecordProperty("TEST_ID", "7ea474cc-db27-43cd-a2f8-dfa2ba196404");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();
    const string<STRINGCAP> sut("L");
    EXPECT_THAT(sut[0U], Eq('L'));
}

TYPED_TEST(stringTyped_test, AccessMaxPositionOfNotEmptyStringViaConstSubscriptOperatorSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "43ca0e71-7ac6-4039-a0fb-ebe84f7c0bad");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString().capacity();

    std::string testSTDString(STRINGCAP, 'L');
    const string<STRINGCAP> sut(TruncateToCapacity, testSTDString.c_str(), testSTDString.size());

    EXPECT_THAT(sut[STRINGCAP - 1], Eq('L'));
}

/// @note template <typename T>
/// constexpr bool insert(const uint64_t pos, const T& str, const uint64_t count) noexcept
TEST(String10, InsertStringLiteralAtTheBeginningOfTheStringSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "8f3c5bf8-2367-46c0-b803-570a2a640c93");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("Hypnotoad");
    string<STRINGCAP> sut("toad");
    ASSERT_TRUE(sut.insert(0, "Hypno", 5));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TEST(String10, InsertStringLiteralInTheMiddleOfTheStringSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "861eec68-3e78-42cf-ac75-30c23a3e60d2");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("Hypnotoad");
    string<STRINGCAP> sut("Hypoad");
    ASSERT_TRUE(sut.insert(3, "not", 3));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TEST(String10, InsertStringLiteralAtTheEndOfTheStringSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "91f733e0-5ded-4f85-9727-241b649bd7dc");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("Hypnotoad");
    string<STRINGCAP> sut("Hypno");
    ASSERT_TRUE(sut.insert(sut.size(), "toad", 4));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TYPED_TEST(stringTyped_test, InsertStringLiteralToEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "eba320fc-9909-4043-a944-7a5c9f2a4272");
    const string<1> expectedString("M");
    ASSERT_TRUE(this->testSubject.insert(0, "M", 1));
    EXPECT_THAT(this->testSubject.size(), Eq(expectedString.size()));
    EXPECT_THAT(this->testSubject, Eq(expectedString));
}

TYPED_TEST(stringTyped_test, InsertEmptyStringLiteralDoesNotChangeTheString)
{
    ::testing::Test::RecordProperty("TEST_ID", "37d723b7-5b35-4f1d-ac33-23dd483121a1");
    this->testSubject = "M";
    ASSERT_TRUE(this->testSubject.insert(0, "", 0));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, InsertStringLiteralWithCountGreaterThanSizeOfStringLiteralFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "3d2951f8-d868-4998-bb33-58424aba7a65");
    ASSERT_FALSE(this->testSubject.insert(0, "M", 2));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TEST(String10, InsertTooLargeStringLiteralFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "0f11f387-f437-4a39-8207-259df4807c70");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("Ferdinand");
    string<STRINGCAP> sut(expectedString);
    ASSERT_FALSE(sut.insert(sut.size(), "Spitzschnueffler", 16));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TEST(String10, InsertTooLargeStringLiteralWithSmallCountSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "2763b3d8-3023-4841-80d1-798acd3b71d7");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("FerdinandS");
    string<STRINGCAP> sut("Ferdinand");
    ASSERT_TRUE(sut.insert(sut.size(), "Spitzschnueffler", 1));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TYPED_TEST(stringTyped_test, InsertStringLiteralWithCount0DoesntChangeTheString)
{
    ::testing::Test::RecordProperty("TEST_ID", "90e80302-1c14-48bb-b9e9-0f8fca017fea");
    ASSERT_TRUE(this->testSubject.insert(0, "Ferdinand", 0));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TEST(String10, InsertStringLiteralAtPositionGreaterStringSizeFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "a3a30ea5-f228-48f4-9497-08502e4f2c9a");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("Muesli");
    string<STRINGCAP> sut(expectedString);
    ASSERT_FALSE(sut.insert(sut.size() + 1U, "s", 1));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TEST(String10, InsertCxxStringAtTheBeginningOfTheStringSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "df62df30-0838-4282-91f7-b39b1c861861");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("Hypnotoad");
    string<STRINGCAP> sut("toad");
    string<STRINGCAP> string_to_insert("Hypno");
    ASSERT_TRUE(sut.insert(0, string_to_insert, string_to_insert.size()));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TEST(String10, InsertCxxStringInTheMiddleOfTheStringSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5a94b5a-c734-4654-a6ba-08819b4b7eac");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("Hypnotoad");
    string<STRINGCAP> sut("Hypoad");
    string<STRINGCAP> string_to_insert("not");
    ASSERT_TRUE(sut.insert(3, string_to_insert, string_to_insert.size()));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TEST(String10, InsertCxxStringAtTheEndOfTheStringSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "f77dfb73-77ce-471f-b90b-8fcf0ab1a5de");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("Hypnotoad");
    string<STRINGCAP> sut("Hypno");
    string<STRINGCAP> string_to_insert("toad");
    ASSERT_TRUE(sut.insert(sut.size(), string_to_insert, string_to_insert.size()));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TYPED_TEST(stringTyped_test, InsertCxxStringToEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "95ce194c-62a7-420c-9f37-9a2d7c5b17b2");
    string<1> string_to_insert("M");
    ASSERT_TRUE(this->testSubject.insert(0, string_to_insert, string_to_insert.size()));
    EXPECT_THAT(this->testSubject.size(), Eq(string_to_insert.size()));
    EXPECT_THAT(this->testSubject, Eq(string_to_insert));
}

TYPED_TEST(stringTyped_test, InsertEmptyCxxStringDoesNotChangeTheString)
{
    ::testing::Test::RecordProperty("TEST_ID", "b566efaf-4a32-41a1-970f-4d51675e594f");
    this->testSubject = "M";
    string<1> string_to_insert("");
    ASSERT_TRUE(this->testSubject.insert(0, string_to_insert, string_to_insert.size()));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, InsertCxxStringWithCountGreaterThanSizeOfStringLiteralFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "9bd29af2-12c8-4dc1-8ee8-82610f1bb9a6");
    string<1> string_to_insert("M");
    ASSERT_FALSE(this->testSubject.insert(0, string_to_insert, string_to_insert.size() + 1));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TEST(String10, InsertTooLargeCxxStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "e9d88dd2-9da5-4f5f-a798-10b3931e6516");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("Ferdinand");
    string<STRINGCAP> sut(expectedString);
    string<STRINGCAP + 6U> string_to_insert("Spitzschnueffler");
    ASSERT_FALSE(sut.insert(sut.size(), string_to_insert, string_to_insert.size()));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TEST(String10, InsertTooLargeCxxStringWithSmallCountSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "e99348ac-87a4-4678-b33a-2ddac3e21cb4");
    constexpr uint64_t STRINGCAP = 10U;
    constexpr uint64_t INSERT_STRINGCAP = STRINGCAP + 6U;
    const string<STRINGCAP> expectedString("FerdinandS");
    string<STRINGCAP> sut("Ferdinand");
    string<INSERT_STRINGCAP> string_to_insert("Spitzschnueffler");
    ASSERT_TRUE(sut.insert(sut.size(), string_to_insert, 1));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TYPED_TEST(stringTyped_test, InsertCxxStringWithCount0DoesntChangeTheString)
{
    ::testing::Test::RecordProperty("TEST_ID", "340c595e-b0b1-43a8-a08e-ebb62dc0d306");
    const string<10> string_to_insert("Ferdinand");
    ASSERT_TRUE(this->testSubject.insert(0, string_to_insert, 0));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TEST(String10, InsertCxxStringAtPositionGreaterStringSizeFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "a2517ea0-2842-4f56-a89a-b7fd01c6c6ba");
    constexpr uint64_t STRINGCAP = 10U;
    const string<STRINGCAP> expectedString("Muesli");
    string<STRINGCAP> sut(expectedString);
    string<1> string_to_insert("s");
    ASSERT_FALSE(sut.insert(sut.size() + 1U, string_to_insert, 1));
    EXPECT_THAT(sut.size(), Eq(expectedString.size()));
    EXPECT_THAT(sut, Eq(expectedString));
}

TEST(stringTyped_test, NonCxxStringsAreIdentifiedCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "898fdeb7-2b35-4d33-8db4-ed3b9447a1da");

    EXPECT_FALSE(is_iox_string<int>::value);
    /// @NOLINTJUSTIFICATION we want test explicitly the c arrays case
    /// @NOLINTBEGIN(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    EXPECT_FALSE(is_iox_string<int[10]>::value);
    EXPECT_FALSE(is_iox_string<char[11]>::value);
    /// @NOLINTEND(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    EXPECT_FALSE(is_iox_string<char>::value);
}

TEST(stringTyped_test, CxxStringsAreIdentifiedCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "778995dc-9be4-47f1-9490-cd111930d3d3");

    EXPECT_TRUE(is_iox_string<iox::string<1>>::value);
    EXPECT_TRUE(is_iox_string<iox::string<10>>::value);
}
} // namespace
