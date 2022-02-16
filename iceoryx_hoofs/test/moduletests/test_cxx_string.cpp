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

#include "iceoryx_hoofs/cxx/string.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::cxx;

template <typename T>
class stringTyped_test : public Test
{
  protected:
    T testSubject;

    using stringType = T;
};

using Implementations = Types<string<1>, string<15>, string<100>, string<1000>>;

TYPED_TEST_SUITE(stringTyped_test, Implementations);


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
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP> fuu(this->testSubject);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
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
    char testChar[STRINGCAP];
    for (uint64_t i = 0U; i < STRINGCAP - 1U; i++)
    {
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1U] = '\0';
    string<STRINGCAP> testSubject(testChar);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
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
    char testChar[STRINGCAP + 1];
    for (uint64_t i = 0U; i < STRINGCAP - 1U; i++)
    {
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1U] = '\0';
    string<STRINGCAP> testSubject(TruncateToCapacity, testChar);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithSizeGreaterCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "5e0a2023-ea15-43d5-aae8-980a75be6122");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    char testChar[STRINGCAP + 1U];
    for (uint64_t i = 0U; i < STRINGCAP; i++)
    {
        testChar[i] = 'M';
    }
    testChar[STRINGCAP] = '\0';
    string<STRINGCAP> testSubject(TruncateToCapacity, testChar);
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

/// @note string(TruncateToCapacity_t, const std::string& other) noexcept
TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSize0ResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "83e1b7b2-8487-4c71-ac86-f4d5d98c1918");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString;
    string<STRINGCAP> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSizeSmallerCapaResultsInSizeSmallerCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "1bd6cd60-0487-4ba2-9e51-3a9297078454");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP - 1U, 'M');
    string<STRINGCAP> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
}

TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "afa37f19-fde0-40ab-b1bd-10862f623ae7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    string<STRINGCAP> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
}

TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSizeGreaterCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "67cba3f0-30ed-415d-8232-8e8b5898fe04");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP + 1U, 'M');
    string<STRINGCAP> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString.substr(0U, STRINGCAP)));
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
    std::string testString{"ice\0ryx", 7U};
    string<100U> testSubject(TruncateToCapacity, testString.c_str(), 7U);
    EXPECT_THAT(testSubject.capacity(), Eq(100U));
    EXPECT_THAT(testSubject.size(), Eq(7U));
    EXPECT_THAT(testSubject.c_str(), StrEq("ice\0ryx"));
}

TEST(CharArrayAssignment, AssignCharArrayWithStringSizeLessThanArraySize)
{
    ::testing::Test::RecordProperty("TEST_ID", "886f580d-e57c-4685-90bf-2399737779be");
    char testString[20] = "iceoryx";
    string<20U> testSubject(testString);
    EXPECT_THAT(testSubject.size(), Eq(7U));
    EXPECT_THAT(testSubject.c_str(), StrEq("iceoryx"));
}

TEST(CharArrayAssignment, AssignZeroTerminatedCharArrayWithSizeForFullCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "884e724a-f5d3-41d1-89fa-96f55ce99a96");
    char testString[8] = "iceoryx";
    string<7U> testSubject(testString);
    EXPECT_THAT(testSubject.size(), Eq(7U));
    EXPECT_THAT(testSubject.c_str(), StrEq("iceoryx"));
}

TEST(CharArrayAssignment, AssignNonZeroTerminatedCharArrayOfSizeForFullCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "2a43553f-4358-4c41-a885-1495de0d7f4f");
    char testString[8] = "iceoryx";
    testString[7] = 'x'; // overwrite the 0 termination
    string<7U> testSubject(testString);
    EXPECT_THAT(testSubject.size(), Eq(7U));
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
    char testChar[STRINGCAP];
    for (uint64_t i = 0U; i < STRINGCAP - 1U; i++)
    {
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1U] = '\0';
    string<STRINGCAP> testSubject;
    testSubject = testChar;
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
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
    string<STRINGCAP> fuu(TruncateToCapacity, testString);
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString), Eq(true));
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
    char testChar[STRINGCAP];
    for (uint64_t i = 0U; i < STRINGCAP - 1U; i++)
    {
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1U] = '\0';
    string<STRINGCAP> testSubject;
    testSubject.assign(testChar);
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
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

/// @note bool unsafe_assign(const std::string& str) noexcept
TYPED_TEST(stringTyped_test, UnsafeAssignOfSTDStringOfSize0ResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "709e4c86-963b-495e-a81c-c09a84cb2320");
    std::string testString;
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfSTDStringOfSize1ResultsInSize1)
{
    ::testing::Test::RecordProperty("TEST_ID", "84dc3655-a4c8-46bc-afd2-059501da1c86");
    std::string testString = "M";
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, UnsafeAssignSTDStringOfSizeCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "27011e92-813a-410a-ba14-fec7dfefe942");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
}

TYPED_TEST(stringTyped_test, UnsafeAssignSTDStringOfSizeGreaterCapaResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "72dbeffb-5787-41cf-9998-b745d8683bb4");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP + 1U, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(0U));
}

TYPED_TEST(stringTyped_test, AssignOfInvalidSTDStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "c16cec0c-40b5-43cb-adab-f95e11b96b8e");
    this->testSubject = "L";

    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP + 1U, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("L"));
}

/// @note template <uint64_t N>
/// int64_t compare(const string<N>& other) const noexcept
TYPED_TEST(stringTyped_test, CompareEqStringsResultsInZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "8271fa31-3301-4d2e-a290-a4b61a70dfb5");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP> fuu;
    EXPECT_THAT(fuu.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.compare(this->testSubject), Eq(0));
    EXPECT_THAT(this->testSubject.compare(fuu), Eq(0));
}

TYPED_TEST(stringTyped_test, CompareResultNegative)
{
    ::testing::Test::RecordProperty("TEST_ID", "b17fc495-a82b-4ee8-af17-28afaabd3f0e");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    string<STRINGCAP> fuu;
    std::string testString2(STRINGCAP, 'L');
    EXPECT_THAT(fuu.unsafe_assign(testString2), Eq(true));
    EXPECT_THAT(fuu.compare(this->testSubject), Lt(0));
}

TYPED_TEST(stringTyped_test, CompareResultPositive)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff95b244-937a-4519-90d8-8c82acf01b6b");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    string<STRINGCAP> fuu;
    std::string testString2(STRINGCAP, 'L');
    EXPECT_THAT(fuu.unsafe_assign(testString2), Eq(true));
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
    std::string testString1{"ice\0ryx", 7};
    std::string testString2{"ice\0rYx", 7};
    string<100U> testSubject1(TruncateToCapacity, testString1.c_str(), 7U);
    string<100U> testSubject2(TruncateToCapacity, testString2.c_str(), 7U);
    EXPECT_THAT(testSubject1.compare(testSubject2), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareEqStringsWithDifferentCapaResultsInZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "ddf78ea1-b5cd-44fd-9320-6801893c30e7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP + 1U> fuu;
    EXPECT_THAT(fuu.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.compare(this->testSubject), Eq(0));
    EXPECT_THAT(this->testSubject.compare(fuu), Eq(0));
}

TYPED_TEST(stringTyped_test, CompareResultNegativeWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0499ebc-249c-4b31-a1b0-0e34035e77f2");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    string<STRINGCAP + 1U> fuu;
    std::string testString2(STRINGCAP + 1U, 'M');
    EXPECT_THAT(fuu.unsafe_assign(testString2), Eq(true));
    EXPECT_THAT(this->testSubject.compare(fuu), Lt(0));
}

TYPED_TEST(stringTyped_test, CompareResultPositiveWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "97fed2cb-4f25-4732-9bbe-4d710b9a35f7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    string<STRINGCAP + 1U> fuu;
    std::string testString2(STRINGCAP + 1U, 'M');
    EXPECT_THAT(fuu.unsafe_assign(testString2), Eq(true));
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
    std::string testString1{"ice\0ryx", 7};
    std::string testString2{"ice\0rYx", 7};
    string<200U> testSubject1(TruncateToCapacity, testString1.c_str(), 7U);
    string<100U> testSubject2(TruncateToCapacity, testString2.c_str(), 7U);
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
    EXPECT_THAT(testString3.unsafe_assign(testStdString), Eq(true));
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
    EXPECT_THAT(testString3.unsafe_assign(testStdString), Eq(true));
    EXPECT_THAT(testString1 != testString2, Eq(true));
    EXPECT_THAT(testString3 != testString2, Eq(true));
}

/// @note template <uint64_t N>
/// bool operator<(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorLesserResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb8495ba-f2f9-4e7c-a017-f8295fcff518");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(bar < fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "6bf80f49-f637-4207-900e-2cfec89eb556");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu < bar, Eq(false));
    EXPECT_THAT(fuu < fuu, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserResultTrueWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "bbe252cb-6fba-45d1-ad31-cac42fb73caa");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testString1("M");
    string<STRINGCAP + 1U> testString2("L");
    EXPECT_THAT(testString2 < testString1, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserResultFalseWithDifferentCapa)
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
TYPED_TEST(stringTyped_test, CompareOperatorLesserEqResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "3a8ba399-4e2c-483d-835a-6d4bcbfa9a29");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(this->testSubject <= fuu, Eq(true));
    EXPECT_THAT(bar <= fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserEqResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "59928731-2d4b-4122-8924-7809001fa631");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu <= bar, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserEqResultTrueWithDifferentCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "60c20357-c67a-4d4f-aca7-1ce3ab84677c");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP + 1U> bar("L");
    EXPECT_THAT(this->testSubject <= fuu, Eq(true));
    EXPECT_THAT(bar <= fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserEqResultFalseWithDifferentCapa)
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

/// @note explicit operator std::string() const noexcept
TYPED_TEST(stringTyped_test, EmptyStringToSTDStringConvResultsInZeroSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "753888b8-12e2-4534-a2fd-32b29b457803");
    std::string testString = std::string(this->testSubject);
    EXPECT_THAT(testString.size(), Eq(0U));
    EXPECT_THAT(testString.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, StringOfSizeCapaToSTDStringConvResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "50e727f3-c855-4613-9e38-a56429fa5748");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    std::string testString2 = std::string(this->testSubject);
    EXPECT_THAT(testString2.size(), Eq(STRINGCAP));
    EXPECT_THAT(testString2.c_str(), StrEq(testString1.substr(0, STRINGCAP)));
}

/// @note template <uint64_t Capacity>
/// inline bool operator==(const std::string& lhs, const string<Capacity>& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorSTDStringEqualFixedStringResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c048e63-2e9f-443a-8d22-380f5fc41057");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testSTDString == testFixedString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringEqualFixedStringWithSameSizeResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "7500e443-43d0-4483-909e-0d4dc4c6b6bd");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testSTDString == testFixedString, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringEqualFixedStringResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "19e827d2-eece-47d6-a28d-efd3ad2938b4");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "ML";
    EXPECT_THAT(testSTDString == testFixedString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline bool operator==(const string<Capacity>& lhs, const std::string& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorFixedStringEqualSTDStringResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "c7a12cc3-6062-4f96-b34a-be1cd8976824");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testFixedString == testSTDString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringEqualSTDStringWithSameSizeResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "558d111c-d5ba-497c-bb90-9ffee2d155d8");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testFixedString == testSTDString, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringEqualSTDStringResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "cd651629-e6e7-4f7f-82da-aab5517aebec");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "ML";
    EXPECT_THAT(testFixedString == testSTDString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline bool operator!=(const std::string& lhs, const string<Capacity>& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorSTDStringNotEqualFixedStringResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "0c2523db-6848-49d9-99c9-43bb25edbe5d");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "Ferdinand Spitzschnüffler";
    EXPECT_THAT(testSTDString != testFixedString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringNotEqualFixedStringWithSameSizeResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "9e1292c3-44ac-4889-9a2f-233233cb3fe5");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testSTDString != testFixedString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringNotEqualFixedStringResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "34c13d2b-9e1b-4293-9d30-c5a5969c58e8");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testSTDString != testFixedString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline bool operator!=(const string<Capacity>& lhs, const std::string& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorFixedStringNotEqualSTDStringResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3ceaf9a-6f77-4688-be70-9a56a7b2cb78");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "Müslimädchen";
    EXPECT_THAT(testFixedString != testSTDString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringNotEqualSTDStringWithSameSizeResultTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "99a40704-9d0b-49f9-bd5c-4c87b9f2ba00");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testFixedString != testSTDString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringNotEqualSTDStringResultFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "ad5b2551-2a6c-4da4-a421-ee02e3b39f88");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testFixedString != testSTDString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline std::ostream& operator<<(std::ostream& stream, const string<Capacity>& str)
TYPED_TEST(stringTyped_test, EmptyStreamInputWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "88c68194-9a9c-4f2f-a0e0-90bd72f9b102");
    std::ostringstream testStream;
    testStream << "";
    EXPECT_THAT(testStream.str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, StreamInputOfSizeCapacityWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c9b3dff3-008d-4189-818f-3534767e7ee4");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    string<STRINGCAP> testFixedString(TruncateToCapacity, testString);
    std::ostringstream testStream;
    testStream << testFixedString;
    EXPECT_THAT(testStream.str(), Eq(testFixedString.c_str()));
}

/// @ note constexpr bool empty() const noexcept
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
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
    auto testString2 = iox::cxx::concatenate(this->testSubject, testString1);

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
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString), Eq(true));
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString0), Eq(true));
    std::string testStdString1(STRINGCAP + 3U, 'L');
    string<STRINGCAP + 3U> testString1(TruncateToCapacity, testStdString1);
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

    std::string cmpString = std::string(testString2) + std::string(this->testSubject) + std::string(testString1);
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
    auto testString = iox::cxx::concatenate("Ferdi", "nandSpitzschnu", "ef", "fler");
    EXPECT_THAT(testString.capacity(), Eq(25U));
    EXPECT_THAT(testString.size(), Eq(25U));
    EXPECT_THAT(testString.c_str(), StrEq("FerdinandSpitzschnueffler"));
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString), Eq(true));
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
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString0), Eq(true));
    std::string testStdString1(STRINGCAP + 3U, 'L');
    string<STRINGCAP + 3U> testString1(TruncateToCapacity, testStdString1);
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
    EXPECT_THAT(testString.unsafe_assign(testStdString), Eq(true));

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
    string<10U> testString("R2-D");
    EXPECT_THAT(testString.unsafe_append("2"), Eq(true));
    EXPECT_THAT(testString.capacity(), Eq(10U));
    EXPECT_THAT(testString.size(), Eq(5U));
    EXPECT_THAT(testString.c_str(), StrEq("R2-D2"));
}

TEST(String10, UnsafeAppendTooLargeStringLiteralFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "f9305077-c3a8-4edb-be5f-ea8b1af19645");
    string<10U> testString("Kern");
    EXPECT_THAT(testString.unsafe_append("fusionsbaby"), Eq(false));
    EXPECT_THAT(testString.capacity(), Eq(10U));
    EXPECT_THAT(testString.size(), Eq(4U));
    EXPECT_THAT(testString.c_str(), StrEq("Kern"));
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
    EXPECT_THAT(testString.unsafe_assign(testStdString), Eq(true));
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
    string<10U> testString("Picar");
    testString.append(TruncateToCapacity, "d");
    EXPECT_THAT(testString.capacity(), Eq(10U));
    EXPECT_THAT(testString.size(), Eq(6U));
    EXPECT_THAT(testString.c_str(), StrEq("Picard"));
}

TEST(String10, AppendTooLargeStringLiteralResultsInTruncatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "524a544a-0275-4549-a6dd-7c40e31af91c");
    string<10U> testString("Live long");
    testString.append(TruncateToCapacity, " and prosper");
    EXPECT_THAT(testString.capacity(), Eq(10U));
    EXPECT_THAT(testString.size(), Eq(10U));
    EXPECT_THAT(testString.c_str(), StrEq("Live long "));
}

/// @note iox::cxx::optional<string<Capacity>> substr(uint64_t pos = 0) const noexcept;
TYPED_TEST(stringTyped_test, SubstrWithDefaultPosAndSizeResultsInWholeString)
{
    ::testing::Test::RecordProperty("TEST_ID", "da66bb36-2a1c-435b-8a47-874eb12315ef");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString), Eq(true));
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
    std::string testStdString = "Mueslimaedchen";
    std::string testStdSubstring = testStdString.substr(8);
    string<100U> testCxxString(TruncateToCapacity, testStdString);
    auto res = testCxxString.substr(8U);
    ASSERT_THAT(res.has_value(), Eq(true));

    auto testSubstring = res.value();
    EXPECT_THAT(testSubstring.capacity(), Eq(100U));
    EXPECT_THAT(testSubstring.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring.c_str(), StrEq(testStdSubstring));
}

/// @note iox::cxx::optional<string<Capacity>> substr(uint64_t pos, uint64_t count) const noexcept
TEST(String100, SubstrWithValidPosAndSizeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3cd90af2-97f4-4767-854d-d3ca726bd348");
    std::string testStdString = "Ferdinand Spitzschnueffler";
    string<100U> testCxxString(TruncateToCapacity, testStdString);

    std::string testStdSubstring = testStdString.substr(0, 19);
    auto res1 = testCxxString.substr(0U, 19U);
    ASSERT_THAT(res1.has_value(), Eq(true));
    auto testSubstring1 = res1.value();
    EXPECT_THAT(testSubstring1.capacity(), Eq(100U));
    EXPECT_THAT(testSubstring1.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring1.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(20, 5);
    auto res2 = testCxxString.substr(20U, 5U);
    EXPECT_THAT(res2.has_value(), Eq(true));
    auto testSubstring2 = res2.value();
    EXPECT_THAT(testSubstring2.capacity(), Eq(100U));
    EXPECT_THAT(testSubstring2.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring2.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(0, 26);
    auto res3 = testCxxString.substr(0U, 26U);
    ASSERT_THAT(res3.has_value(), Eq(true));
    auto testSubstring3 = res3.value();
    EXPECT_THAT(testSubstring3.capacity(), Eq(100U));
    EXPECT_THAT(testSubstring3.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring3.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(11, 8);
    auto res4 = testCxxString.substr(11U, 8U);
    ASSERT_THAT(res4.has_value(), Eq(true));
    auto testSubstring4 = res4.value();
    EXPECT_THAT(testSubstring4.capacity(), Eq(100U));
    EXPECT_THAT(testSubstring4.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring4.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(13, 98);
    auto res5 = testCxxString.substr(13U, 98U);
    ASSERT_THAT(res5.has_value(), Eq(true));
    auto testSubstring5 = res5.value();
    EXPECT_THAT(testSubstring5.capacity(), Eq(100U));
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

    std::string testStdString;
    res = this->testSubject.find(testStdString);
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

    std::string testStdString = "a";
    res = this->testSubject.find(testStdString);
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
    string<100U> testString("Kernfusionsbaby");
    string<100U> substring("abc");
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
    string<100U> testString1("Mueslimaedchen");
    auto res = testString1.find("lima");
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    res = testString1.find("lima", 2U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    res = testString1.find("e", 10U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(12U));

    std::string testStdString{"ice\0ryx", 7};
    string<100U> testString2(TruncateToCapacity, testStdString.c_str(), 7U);
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

TEST(String100, FindSTDStringInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "794f62c2-deca-4511-9529-3353ff9ee552");
    string<100U> testString("R2-D2");
    std::string testStdString = "2";
    auto res = testString.find(testStdString);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find(testStdString, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find(testStdString, 2U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));
}

TEST(String100, FindNotIncludedSTDStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "8b2116c9-5f7d-48b4-8c26-cb3b71cf0ea2");
    string<100U> testString("Kernfusionsbaby");
    std::string testStdString = "abc";
    auto res = testString.find(testStdString);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find(testStdString, 0U);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find(testStdString, 50U);
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

    std::string testStdString;
    res = this->testSubject.find_first_of(testStdString);
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

    std::string testStdString = "a";
    res = this->testSubject.find_first_of(testStdString);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindFirstOfForStringInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b24dbd99-3595-4b04-9e6e-177d2a6e4ad0");
    string<10U> testString("R2-D2");
    string<100U> substring1("2");
    auto res = testString.find_first_of(substring1);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find_first_of(substring1, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find_first_of(substring1, 2U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    string<100U> substring2("D3R");
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
    string<100U> testString("Kernfusionsbaby");
    string<100U> substring("cdG");
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
    string<100U> testString1("Mueslimaedchen");
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

    std::string testStdString{"ice\0ryx", 7};
    string<100U> testString2(TruncateToCapacity, testStdString.c_str(), 7U);
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

TEST(String100, FindFirstOfForSTDStringInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1f44acab-aa37-4f45-a782-06ad02bd926b");
    string<100U> testString("R2-D2");
    std::string testStdString1 = "2";
    auto res = testString.find_first_of(testStdString1);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find_first_of(testStdString1, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find_first_of(testStdString1, 2U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    std::string testStdString2 = "D3R";
    res = testString.find_first_of(testStdString2);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0U));

    res = testString.find_first_of(testStdString2, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(3U));
}

TEST(String100, FindFirstOfForNotIncludedSTDStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "700d9fd9-4039-490e-9dd4-3833fb9f5e08");
    string<100U> testString("Kernfusionsbaby");
    std::string testStdString = "cd";
    auto res = testString.find_first_of(testStdString);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of(testStdString, 0U);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of(testStdString, 50U);
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

    std::string testStdString;
    res = this->testSubject.find_last_of(testStdString);
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

    std::string testStdString = "a";
    res = this->testSubject.find_last_of(testStdString);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindLastOfForStringInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "384b1457-ccaa-4801-a7be-625ef9b7a27a");
    string<10U> testString("R2-D2");
    string<100U> substring1("2");
    auto res = testString.find_last_of(substring1);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    res = testString.find_last_of(substring1, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find_last_of(substring1, 5U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    string<100U> substring2("D3R");
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
    string<100U> testString("Kernfusionsbaby");
    string<100U> substring("cdG");
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

TEST(String100, FindLastOfForSTDStringInNotEmptyStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f068fa78-1e97-4148-bbba-da9cc2cf022e");
    string<100U> testString("R2-D2");
    std::string testStdString1 = "2";
    auto res = testString.find_last_of(testStdString1);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    res = testString.find_last_of(testStdString1, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1U));

    res = testString.find_last_of(testStdString1, 5U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4U));

    std::string testStdString2 = "D3R";
    res = testString.find_last_of(testStdString2);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(3U));

    res = testString.find_last_of(testStdString2, 1U);
    ASSERT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0U));
}

TEST(String100, FindLastOfForNotIncludedSTDStringFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "91df370f-38ee-41e8-8063-5f4c3010374f");
    string<100U> testString("Kernfusionsbaby");
    std::string testStdString = "cd";
    auto res = testString.find_last_of(testStdString);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_last_of(testStdString, 0U);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_last_of(testStdString, 50U);
    EXPECT_THAT(res.has_value(), Eq(false));
}
} // namespace
