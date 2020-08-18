// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/cxx/string.hpp"
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
/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(stringTyped_test, Implementations);
#pragma GCC diagnostic pop

/// @note string() noexcept
TYPED_TEST(stringTyped_test, EmptyInitializationResultsInSize0)
{
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, EmptyInitializationResultsInEmptyString)
{
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

/// @note string(const string& other) noexcept
TYPED_TEST(stringTyped_test, CopyConstructEmptyStringResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu(this->testSubject);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, CopyConstructStringOfSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
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
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testString(std::move(this->testSubject));
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveConstructionWithStringOfSizeSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP - 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP> fuu(std::move(this->testSubject));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP - 1));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveConstructionWithStringOfSizeCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP> fuu(std::move(this->testSubject));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

/// @note string& operator=(const string& rhs) noexcept
TYPED_TEST(stringTyped_test, SelfCopyAssignmentExcluded)
{
    this->testSubject = "M";
    this->testSubject = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, CopyAssignmentWithStringOfSize0Works)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu;
    fuu = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, CopyAssignmentWithStringOfSizeSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP - 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP> fuu;
    fuu = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP - 1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testString.substr(0, STRINGCAP - 1)));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP - 1));
    EXPECT_THAT(fuu.c_str(), StrEq(testString.substr(0, STRINGCAP - 1)));
}

TYPED_TEST(stringTyped_test, CopyAssignmentWithStringOfSizeCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
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
    this->testSubject = "M";
    this->testSubject = std::move(this->testSubject);
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfStringWithSize0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfStringWithSmallerSizeResultsInSmallerSize)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP - 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP - 1));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfStringWithSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

/// @note template <uint64_t N>
/// string(const char (&other)[N]) noexcept
TYPED_TEST(stringTyped_test, CharToStringConvConstrWithSize0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("");
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, CharToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    char testChar[STRINGCAP];
    for (uint64_t i = 0; i < STRINGCAP - 1; i++)
    {
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1] = '\0';
    string<STRINGCAP> testSubject(testChar);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
}

/// @note string(TruncateToCapacity_t, const char* const other) noexcept
TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithSize0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu(TruncateToCapacity, "");
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    char testChar[STRINGCAP];
    for (uint64_t i = 0; i < STRINGCAP - 1; i++)
    {
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1] = '\0';
    string<STRINGCAP> testSubject(TruncateToCapacity, testChar);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithSizeGreaterCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    char testChar[STRINGCAP + 1];
    for (uint64_t i = 0; i < STRINGCAP; i++)
    {
        testChar[i] = 'M';
    }
    testChar[STRINGCAP] = '\0';
    string<STRINGCAP> testSubject(TruncateToCapacity, testChar);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP));
}

/// @note string(TruncateToCapacity_t, const std::string& other) noexcept
TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSize0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString;
    string<STRINGCAP> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSizeSmallerCapaResultsInSizeSmallerCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP - 1, 'M');
    string<STRINGCAP> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP - 1));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
}

TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP, 'M');
    string<STRINGCAP> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
}

TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSizeGreaterCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP + 1, 'M');
    string<STRINGCAP> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString.substr(0, STRINGCAP)));
}

/// @note string(TruncateToCapacity_t, const char* const other, const uint64_t count) noexcept
TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithCount0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu(TruncateToCapacity, "Yoda", 0);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithCountEqCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP, 'M');
    string<STRINGCAP> testSubject(TruncateToCapacity, testString.c_str(), STRINGCAP);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.c_str(), StrEq(testString));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithCountGreaterCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP + 1, 'M');
    string<STRINGCAP> testSubject(TruncateToCapacity, testString.c_str(), STRINGCAP + 1);
    EXPECT_THAT(testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(testSubject.c_str(), StrEq(testString.substr(0, STRINGCAP)));
}

TEST(String100, UnsafeCharToStringConstrIncludingNullCharWithCountResultsInSizeCount)
{
    std::string testString{"ice\0ryx", 7};
    string<100> testSubject(TruncateToCapacity, testString.c_str(), 7);
    EXPECT_THAT(testSubject.capacity(), Eq(100));
    EXPECT_THAT(testSubject.size(), Eq(7));
    EXPECT_THAT(testSubject.c_str(), StrEq("ice\0ryx"));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithNullPtrResultsEmptyString)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu(TruncateToCapacity, nullptr, STRINGCAP);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

/// @note template <uint64_t N>
/// string& operator=(const char (&rhs)[N]) noexcept
TYPED_TEST(stringTyped_test, AssignCStringOfSize0WithOperatorResultsInSize0)
{
    this->testSubject = "";
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, AssignCStringOfSizeCapaWithOperatorResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    char testChar[STRINGCAP];
    for (uint64_t i = 0; i < STRINGCAP - 1; i++)
    {
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1] = '\0';
    string<STRINGCAP> testSubject;
    testSubject = testChar;
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
}

/// @note template <uint64_t N>
/// string& assign(const string& str) noexcept
TYPED_TEST(stringTyped_test, SelfAssignmentIsExcluded)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testSubject;
    testSubject.assign(testSubject);
    EXPECT_THAT(testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, AssignStringOfSize0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu;
    this->testSubject.assign(fuu);
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, AssignStringOfSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
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
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> testString;
    testString.assign(this->testSubject);
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, AssignStringWithSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString), Eq(true));
    string<STRINGCAP + 1> testString;
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
    this->testSubject.assign("");
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, ReassignNothingResultsInZeroSize)
{
    this->testSubject.assign("M");
    this->testSubject.assign("");
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, AssignCStringOfSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    char testChar[STRINGCAP];
    for (uint64_t i = 0; i < STRINGCAP - 1; i++)
    {
        testChar[i] = 'M';
    }
    testChar[STRINGCAP - 1] = '\0';
    string<STRINGCAP> testSubject;
    testSubject.assign(testChar);
    EXPECT_THAT(testSubject.size(), Eq(STRINGCAP - 1));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
}

/// @note bool unsafe_assign(const char* const str) noexcept
TYPED_TEST(stringTyped_test, UnsafeAssignOfCStringOfSize0ResultsInSize0)
{
    EXPECT_THAT(this->testSubject.unsafe_assign(""), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfCStringOfSize1ResultsInSize1)
{
    EXPECT_THAT(this->testSubject.unsafe_assign("M"), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, UnsafeAssignCStringOfSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::vector<char> testCharstring(STRINGCAP, 'M');
    testCharstring.emplace_back('\0');
    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
}

TYPED_TEST(stringTyped_test, UnsafeAssignCStringOfSizeGreaterCapaResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::vector<char> testCharstring(STRINGCAP + 1, 'M');
    testCharstring.emplace_back('\0');
    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfInvalidCStringFails)
{
    this->testSubject = "L";

    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::vector<char> testCharstring(STRINGCAP + 1, 'M');
    testCharstring.emplace_back('\0');

    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("L"));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfCharPointerPointingToSameAddress)
{
    this->testSubject = "M";
    const char* fuu = this->testSubject.c_str();
    EXPECT_THAT(this->testSubject.unsafe_assign(fuu), Eq(false));
}

/// @note bool unsafe_assign(const std::string& str) noexcept
TYPED_TEST(stringTyped_test, UnsafeAssignOfSTDStringOfSize0ResultsInSize0)
{
    std::string testString;
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfSTDStringOfSize1ResultsInSize1)
{
    std::string testString = "M";
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, UnsafeAssignSTDStringOfSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
}

TYPED_TEST(stringTyped_test, UnsafeAssignSTDStringOfSizeGreaterCapaResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP + 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, AssignOfInvalidSTDStringFails)
{
    this->testSubject = "L";

    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP + 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("L"));
}

/// @note template <uint64_t N>
/// int64_t compare(const string<N>& other) const noexcept
TYPED_TEST(stringTyped_test, CompareEqStringsResultsInZero)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP> fuu;
    EXPECT_THAT(fuu.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.compare(this->testSubject), Eq(0));
    EXPECT_THAT(this->testSubject.compare(fuu), Eq(0));
}

TYPED_TEST(stringTyped_test, CompareResultNegative)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    string<STRINGCAP> fuu;
    std::string testString2(STRINGCAP, 'L');
    EXPECT_THAT(fuu.unsafe_assign(testString2), Eq(true));
    EXPECT_THAT(fuu.compare(this->testSubject), Lt(0));
}

TYPED_TEST(stringTyped_test, CompareResultPositive)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    string<STRINGCAP> fuu;
    std::string testString2(STRINGCAP, 'L');
    EXPECT_THAT(fuu.unsafe_assign(testString2), Eq(true));
    EXPECT_THAT(this->testSubject.compare(fuu), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareWithEmptyStringResultsInPositive)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    EXPECT_THAT(fuu.compare(this->testSubject), Gt(0));
}

TEST(String100, CompareStringsInclNullCharacterWorks)
{
    std::string testString1{"ice\0ryx", 7};
    std::string testString2{"ice\0rYx", 7};
    string<100> testSubject1(TruncateToCapacity, testString1.c_str(), 7);
    string<100> testSubject2(TruncateToCapacity, testString2.c_str(), 7);
    EXPECT_THAT(testSubject1.compare(testSubject2), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareEqStringsWithDifferentCapaResultsInZero)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP + 1> fuu;
    EXPECT_THAT(fuu.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.compare(this->testSubject), Eq(0));
    EXPECT_THAT(this->testSubject.compare(fuu), Eq(0));
}

TYPED_TEST(stringTyped_test, CompareResultNegativeWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    string<STRINGCAP + 1> fuu;
    std::string testString2(STRINGCAP + 1, 'M');
    EXPECT_THAT(fuu.unsafe_assign(testString2), Eq(true));
    EXPECT_THAT(this->testSubject.compare(fuu), Lt(0));
}

TYPED_TEST(stringTyped_test, CompareResultPositiveWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    string<STRINGCAP + 1> fuu;
    std::string testString2(STRINGCAP + 1, 'M');
    EXPECT_THAT(fuu.unsafe_assign(testString2), Eq(true));
    EXPECT_THAT(fuu.compare(this->testSubject), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareWithEmptyStringOfDifferentCapaResultsInPositive)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> fuu("M");
    EXPECT_THAT(fuu.compare(this->testSubject), Gt(0));
}

TEST(String100, CompareStringsWithDifferentCapaInclNullCharacterWorks)
{
    std::string testString1{"ice\0ryx", 7};
    std::string testString2{"ice\0rYx", 7};
    string<200> testSubject1(TruncateToCapacity, testString1.c_str(), 7);
    string<100> testSubject2(TruncateToCapacity, testString2.c_str(), 7);
    EXPECT_THAT(testSubject1.compare(testSubject2), Gt(0));
}

/// @note template <uint64_t N>
/// bool operator==(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorEqualResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    EXPECT_THAT(fuu == fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorEqualResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu == bar, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorEqualResultTrueWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testString1("M");
    string<STRINGCAP + 1> testString2("M");
    EXPECT_THAT(testString1 == testString2, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorEqualResultFalseWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> testString1("M");
    string<STRINGCAP> testString2("L");
    string<STRINGCAP + 1> testString3;
    std::string testStdString(STRINGCAP + 1, 'L');
    EXPECT_THAT(testString3.unsafe_assign(testStdString), Eq(true));
    EXPECT_THAT(testString1 == testString2, Eq(false));
    EXPECT_THAT(testString3 == testString2, Eq(false));
}

/// @note template <uint64_t N>
/// bool operator!=(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorNotEqualResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    EXPECT_THAT(fuu != fuu, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorNotEqualResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu != bar, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorNotEqualResultFalseWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testString1("M");
    string<STRINGCAP + 1> testString2("M");
    EXPECT_THAT(testString1 != testString2, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorNotEqualResultTrueWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> testString1("M");
    string<STRINGCAP> testString2("L");
    string<STRINGCAP + 1> testString3;
    std::string testStdString(STRINGCAP + 1, 'L');
    EXPECT_THAT(testString3.unsafe_assign(testStdString), Eq(true));
    EXPECT_THAT(testString1 != testString2, Eq(true));
    EXPECT_THAT(testString3 != testString2, Eq(true));
}

/// @note template <uint64_t N>
/// bool operator<(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorLesserResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(bar < fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu < bar, Eq(false));
    EXPECT_THAT(fuu < fuu, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserResultTrueWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testString1("M");
    string<STRINGCAP + 1> testString2("L");
    EXPECT_THAT(testString2 < testString1, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserResultFalseWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> testString1("M");
    string<STRINGCAP> testString2("L");
    EXPECT_THAT(testString1 < testString2, Eq(false));
    EXPECT_THAT(testString1 < testString1, Eq(false));
}

/// @note template <uint64_t N>
/// bool operator<=(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorLesserEqResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(this->testSubject <= fuu, Eq(true));
    EXPECT_THAT(bar <= fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserEqResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu <= bar, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserEqResultTrueWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP + 1> bar("L");
    EXPECT_THAT(this->testSubject <= fuu, Eq(true));
    EXPECT_THAT(bar <= fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserEqResultFalseWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu <= bar, Eq(false));
}

/// @note template <uint64_t N>
/// bool operator>(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorGreaterResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu > bar, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(bar > fuu, Eq(false));
    EXPECT_THAT(bar > bar, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterResultTrueWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(fuu > bar, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterResultFalseWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP + 1> bar("L");
    EXPECT_THAT(bar > fuu, Eq(false));
    EXPECT_THAT(bar > bar, Eq(false));
}

/// @note template <uint64_t N>
/// bool operator>=(const string<N>& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorGreaterEqResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    this->testSubject = "M";
    EXPECT_THAT(fuu >= bar, Eq(true));
    EXPECT_THAT(fuu >= this->testSubject, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterEqResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP> bar("L");
    EXPECT_THAT(bar >= fuu, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterEqResultTrueWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> fuu("M");
    string<STRINGCAP> bar("L");
    this->testSubject = "M";
    EXPECT_THAT(fuu >= bar, Eq(true));
    EXPECT_THAT(fuu >= this->testSubject, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterEqResultFalseWithDifferentCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> fuu("M");
    string<STRINGCAP + 1> bar("L");
    this->testSubject = "L";
    EXPECT_THAT(bar >= fuu, Eq(false));
}

/// @note explicit operator std::string() const noexcept
TYPED_TEST(stringTyped_test, EmptyStringToSTDStringConvResultsInZeroSize)
{
    std::string testString = std::string(this->testSubject);
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, StringOfSizeCapaToSTDStringConvResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
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
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testSTDString == testFixedString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringEqualFixedStringWithSameSizeResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testSTDString == testFixedString, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringEqualFixedStringResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "ML";
    EXPECT_THAT(testSTDString == testFixedString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline bool operator==(const string<Capacity>& lhs, const std::string& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorFixedStringEqualSTDStringResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testFixedString == testSTDString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringEqualSTDStringWithSameSizeResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testFixedString == testSTDString, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringEqualSTDStringResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "ML";
    EXPECT_THAT(testFixedString == testSTDString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline bool operator!=(const std::string& lhs, const string<Capacity>& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorSTDStringNotEqualFixedStringResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "Ferdinand Spitzschnüffler";
    EXPECT_THAT(testSTDString != testFixedString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringNotEqualFixedStringWithSameSizeResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testSTDString != testFixedString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringNotEqualFixedStringResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testSTDString != testFixedString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline bool operator!=(const string<Capacity>& lhs, const std::string& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorFixedStringNotEqualSTDStringResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "Müslimädchen";
    EXPECT_THAT(testFixedString != testSTDString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringNotEqualSTDStringWithSameSizeResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testFixedString != testSTDString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringNotEqualSTDStringResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testFixedString != testSTDString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline std::ostream& operator<<(std::ostream& stream, const string<Capacity>& str)
TYPED_TEST(stringTyped_test, EmptyStreamInputWorks)
{
    std::ostringstream testStream;
    testStream << "";
    EXPECT_THAT(testStream.str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, StreamInputOfSizeCapacityWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP, 'M');
    string<STRINGCAP> testFixedString(TruncateToCapacity, testString);
    std::ostringstream testStream;
    testStream << testFixedString;
    EXPECT_THAT(testStream.str(), Eq(testFixedString.c_str()));
}

/// @ note constexpr bool empty() const noexcept
TYPED_TEST(stringTyped_test, NewlyCreatedStringIsEmpty)
{
    using myString = typename TestFixture::stringType;
    myString sut;
    EXPECT_THAT(sut.empty(), Eq(true));
}

TYPED_TEST(stringTyped_test, StringWithContentIsNotEmtpy)
{
    using myString = typename TestFixture::stringType;
    myString sut(TruncateToCapacity, "Dr.SchluepferStrikesAgain!");
    EXPECT_THAT(sut.empty(), Eq(false));
}

/// @note template <uint64_t N>
/// string(const string<N>& other) noexcept;
TYPED_TEST(stringTyped_test, ConstrWithEmptyStringWithSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> testString(this->testSubject);
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
}

TYPED_TEST(stringTyped_test, ConstrWithStringWithSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "M";
    string<STRINGCAP + 1> testString(this->testSubject);
    EXPECT_THAT(testString.c_str(), StrEq("M"));
    EXPECT_THAT(testString.size(), Eq(1));
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
}

/// @note template <uint64_t N>
/// string(string<N>&& other) noexcept;
TYPED_TEST(stringTyped_test, MoveConstrWithEmptyStringWithSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 30> testString(std::move(this->testSubject));
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveConstrWithStringSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP + 11> fuu(std::move(this->testSubject));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

/// @note template <uint64_t N>
/// string& operator=(const string<N>& rhs) noexcept;
TYPED_TEST(stringTyped_test, AssignmentOfStringWithSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> testString;
    testString = this->testSubject;
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, AssignmentOfEmptyStringWithSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> testString("M");
    testString = this->testSubject;
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, AssignmentOfNotEmptyStringWithSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "M";
    string<STRINGCAP + 30> testString("Ferdinand Spitzschnueffler");
    testString = this->testSubject;
    EXPECT_THAT(testString.c_str(), StrEq("M"));
    EXPECT_THAT(testString.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
}

/// @note template <uint64_t N>
/// string& operator=(string<N>&& rhs) noexcept;
TYPED_TEST(stringTyped_test, MoveAssignmentOfEmptyStringWithSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 63> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfStringWithSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<STRINGCAP + 36> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfNotEmptyStringWithSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "M";
    string<STRINGCAP + 30> testString("Jean-Luc Picard");
    testString = std::move(this->testSubject);
    EXPECT_THAT(testString.c_str(), StrEq("M"));
    EXPECT_THAT(testString.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

/// @note template <typename T1, typename T2, typename... Targs>
/// string<internal::SumCapa<T1, T2, Targs...>::value> concatenate(const T1& t1, const T2& t2, const Targs&... targs)
TYPED_TEST(stringTyped_test, ConcatenateTwoEmptyStringsReturnsEmptyStringWithTotalCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 1> testString1;
    auto testString2 = iox::cxx::concatenate(this->testSubject, testString1);

    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(testString1.capacity(), Eq(STRINGCAP + 1));
    EXPECT_THAT(testString1.size(), Eq(0));
    EXPECT_THAT(testString1.c_str(), StrEq(""));
    EXPECT_THAT(testString2.capacity(), Eq(2 * STRINGCAP + 1));
    EXPECT_THAT(testString2.size(), Eq(0));
    EXPECT_THAT(testString2.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, ConcatenateTwoStringsWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString), Eq(true));
    string<STRINGCAP + 2> testString1;
    auto testString2 = concatenate(testString1, this->testSubject);

    EXPECT_THAT(testString2.capacity(), Eq(2 * STRINGCAP + 2));
    EXPECT_THAT(testString2.size(), Eq(STRINGCAP));
    EXPECT_THAT(testString2.c_str(), StrEq(testStdString));
}

TYPED_TEST(stringTyped_test, ConcatenateTwoNotEmptyStringsWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testStdString0(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString0), Eq(true));
    std::string testStdString1(STRINGCAP + 3, 'L');
    string<STRINGCAP + 3> testString1(TruncateToCapacity, testStdString1);
    auto testString2 = concatenate(this->testSubject, testString1);

    EXPECT_THAT(testString2.capacity(), Eq(2 * STRINGCAP + 3));
    EXPECT_THAT(testString2.size(), Eq(this->testSubject.size() + testString1.size()));
    EXPECT_THAT(testString2.c_str(), StrEq(testStdString0 + testStdString1));
}

TYPED_TEST(stringTyped_test, ConcatenateThreeStringsWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP> testString1("A");
    string<STRINGCAP + 2> testString2("YOD");
    auto testString3 = concatenate(testString2, this->testSubject, testString1);

    std::string cmpString = std::string(testString2) + std::string(this->testSubject) + std::string(testString1);
    EXPECT_THAT(testString3.capacity(), Eq(3 * STRINGCAP + 2));
    EXPECT_THAT(testString3.size(), Eq(cmpString.size()));
    EXPECT_THAT(testString3.c_str(), StrEq(cmpString));
}

TYPED_TEST(stringTyped_test, ConcatenateEmptyStringAndStringLiteralWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    auto testString = concatenate(this->testSubject, "M");
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 1));
    EXPECT_THAT(testString.size(), Eq(1));
    EXPECT_THAT(testString.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, ConcatenateStringLiteralAndStringWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "S";
    auto testString = concatenate("Ferdinand", this->testSubject);
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 9));
    EXPECT_THAT(testString.size(), Eq(10));
    EXPECT_THAT(testString.c_str(), StrEq("FerdinandS"));
}

TYPED_TEST(stringTyped_test, ConcatenateOnlyStringLiteralsWorks)
{
    auto testString = iox::cxx::concatenate("Ferdi", "nandSpitzschnu", "ef", "fler");
    EXPECT_THAT(testString.capacity(), Eq(25));
    EXPECT_THAT(testString.size(), Eq(25));
    EXPECT_THAT(testString.c_str(), StrEq("FerdinandSpitzschnueffler"));
}

/// @note template <typename T1, typename T2>
/// string<internal::GetCapa<T1>::capa + internal::GetCapa<T2>::capa> operator+(const T1& t1, const T2& t2);
TYPED_TEST(stringTyped_test, ConcatenateEmptyStringsReturnsEmptyString)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<2 * STRINGCAP> testString = this->testSubject + this->testSubject;
    EXPECT_THAT(testString.capacity(), Eq(2 * STRINGCAP));
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, ConcatenateStringsWithOperatorPlusWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString), Eq(true));
    string<STRINGCAP + 2> testString1;
    string<2 * STRINGCAP + 2> testString2;
    testString2 = testString1 + this->testSubject;
    EXPECT_THAT(testString2.capacity(), Eq(2 * STRINGCAP + 2));
    EXPECT_THAT(testString2.size(), Eq(STRINGCAP));
    EXPECT_THAT(testString2.c_str(), StrEq(testStdString));
}

TYPED_TEST(stringTyped_test, ConcatenateNotEmptyStringsWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testStdString0(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString0), Eq(true));
    std::string testStdString1(STRINGCAP + 3, 'L');
    string<STRINGCAP + 3> testString1(TruncateToCapacity, testStdString1);
    string<6 * STRINGCAP> testString2 = this->testSubject + testString1 + this->testSubject;
    EXPECT_THAT(testString2.capacity(), Eq(6 * STRINGCAP));
    EXPECT_THAT(testString2.size(), Eq(2 * this->testSubject.size() + testString1.size()));
    EXPECT_THAT(testString2.c_str(), StrEq(testStdString0 + testStdString1 + testStdString0));
}

TYPED_TEST(stringTyped_test, ConcatenateEmptyStringAndStringLiteralWithOperatorPlusWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<2 * STRINGCAP> testString = this->testSubject + "M";
    EXPECT_THAT(testString.capacity(), Eq(2 * STRINGCAP));
    EXPECT_THAT(testString.size(), Eq(1));
    EXPECT_THAT(testString.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, ConcatenateStringLiteralAndStringWithOperatorPlusWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "e";
    string<STRINGCAP + 7> testString = "AdmTass" + this->testSubject;
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 7));
    EXPECT_THAT(testString.size(), Eq(8));
    EXPECT_THAT(testString.c_str(), StrEq("AdmTasse"));
}

/// @note template <typename T>
/// bool unsafe_append(const T& t) noexcept;
TYPED_TEST(stringTyped_test, UnsafeAppendEmptyStringWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "M";
    string<2 * STRINGCAP> testString;
    EXPECT_THAT(this->testSubject.unsafe_append(testString), Eq(true));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, UnsafeAppendFittingStringWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "2";
    string<5 * STRINGCAP> testString("R2-D");
    EXPECT_THAT(testString.unsafe_append(this->testSubject), Eq(true));
    EXPECT_THAT(testString.capacity(), Eq(5 * STRINGCAP));
    EXPECT_THAT(testString.size(), Eq(5));
    EXPECT_THAT(testString.c_str(), StrEq("R2-D2"));
}

TYPED_TEST(stringTyped_test, UnsafeAppendTooLargeStringFails)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "M";
    string<2 * STRINGCAP> testString;
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(testString.unsafe_assign(testStdString), Eq(true));

    EXPECT_THAT(this->testSubject.unsafe_append(testString), Eq(false));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, UnsafeAppendEmptyStringLiteralWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "M";
    EXPECT_THAT(this->testSubject.unsafe_append(""), Eq(true));
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TEST(String10, UnsafeAppendFittingStringLiteralWorks)
{
    string<10> testString("R2-D");
    EXPECT_THAT(testString.unsafe_append("2"), Eq(true));
    EXPECT_THAT(testString.capacity(), Eq(10));
    EXPECT_THAT(testString.size(), Eq(5));
    EXPECT_THAT(testString.c_str(), StrEq("R2-D2"));
}

TEST(String10, UnsafeAppendTooLargeStringLiteralFails)
{
    string<10> testString("Kern");
    EXPECT_THAT(testString.unsafe_append("fusionsbaby"), Eq(false));
    EXPECT_THAT(testString.capacity(), Eq(10));
    EXPECT_THAT(testString.size(), Eq(4));
    EXPECT_THAT(testString.c_str(), StrEq("Kern"));
}

/// @note template <typename T>
/// string& append(TruncateToCapacity_t, const T& t) noexcept
TYPED_TEST(stringTyped_test, AppendEmptyStringWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "M";
    string<STRINGCAP + 1> testString;
    this->testSubject.append(TruncateToCapacity, testString);
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, AppendStringToEmptyStringResultsInConcatenatedString)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 5> testString("M");
    this->testSubject.append(TruncateToCapacity, testString);
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, AppendStringResultsInConcatenatedString)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "d";
    string<STRINGCAP + 5> testString("Picar");
    testString.append(TruncateToCapacity, this->testSubject);
    EXPECT_THAT(testString.capacity(), Eq(STRINGCAP + 5));
    EXPECT_THAT(testString.size(), Eq(6));
    EXPECT_THAT(testString.c_str(), StrEq("Picard"));
}

TYPED_TEST(stringTyped_test, AppendTooLargeStringResultsInTruncatedString)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "M";
    string<STRINGCAP + 1> testString;
    std::string testStdString(STRINGCAP + 1, 'M');
    EXPECT_THAT(testString.unsafe_assign(testStdString), Eq(true));
    this->testSubject.append(TruncateToCapacity, testString);
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testStdString.substr(0, STRINGCAP)));
}

TYPED_TEST(stringTyped_test, AppendEmptyStringLiteralWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject = "M";
    this->testSubject.append(TruncateToCapacity, "");
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(stringTyped_test, AppendStringLiteralToEmptyStringResultsInConcatenatedString)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    this->testSubject.append(TruncateToCapacity, "M");
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TEST(String10, AppendStringLiteralResultsInConcatenatedString)
{
    string<10> testString("Picar");
    testString.append(TruncateToCapacity, "d");
    EXPECT_THAT(testString.capacity(), Eq(10));
    EXPECT_THAT(testString.size(), Eq(6));
    EXPECT_THAT(testString.c_str(), StrEq("Picard"));
}

TEST(String10, AppendTooLargeStringLiteralResultsInTruncatedString)
{
    string<10> testString("Live long");
    testString.append(TruncateToCapacity, " and prosper");
    EXPECT_THAT(testString.capacity(), Eq(10));
    EXPECT_THAT(testString.size(), Eq(10));
    EXPECT_THAT(testString.c_str(), StrEq("Live long "));
}

/// @note iox::cxx::optional<string<Capacity>> substr(uint64_t pos = 0) const noexcept;
TYPED_TEST(stringTyped_test, SubstrWithDefaultPosAndSizeResultsInWholeString)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    std::string testStdString(STRINGCAP, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testStdString), Eq(true));
    auto substr = this->testSubject.substr();
    EXPECT_THAT(substr.has_value(), Eq(true));

    auto testSubstring = substr.value();
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
    std::string testStdString = "Mueslimaedchen";
    std::string testStdSubstring = testStdString.substr(8);
    string<100> testCxxString(TruncateToCapacity, testStdString);
    auto substr = testCxxString.substr(8);
    EXPECT_THAT(substr.has_value(), Eq(true));

    auto testSubstring = substr.value();
    EXPECT_THAT(testSubstring.capacity(), Eq(100));
    EXPECT_THAT(testSubstring.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring.c_str(), StrEq(testStdSubstring));
}

/// @note iox::cxx::optional<string<Capacity>> substr(uint64_t pos, uint64_t count) const noexcept
TEST(String100, SubstrWithValidPosAndSizeWorks)
{
    std::string testStdString = "Ferdinand Spitzschnueffler";
    string<100> testCxxString(TruncateToCapacity, testStdString);

    std::string testStdSubstring = testStdString.substr(0, 19);
    auto substr1 = testCxxString.substr(0, 19);
    EXPECT_THAT(substr1.has_value(), Eq(true));
    auto testSubstring1 = substr1.value();
    EXPECT_THAT(testSubstring1.capacity(), Eq(100));
    EXPECT_THAT(testSubstring1.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring1.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(20, 5);
    auto substr2 = testCxxString.substr(20, 5);
    EXPECT_THAT(substr2.has_value(), Eq(true));
    auto testSubstring2 = substr2.value();
    EXPECT_THAT(testSubstring2.capacity(), Eq(100));
    EXPECT_THAT(testSubstring2.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring2.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(0, 26);
    auto substr3 = testCxxString.substr(0, 26);
    EXPECT_THAT(substr3.has_value(), Eq(true));
    auto testSubstring3 = substr3.value();
    EXPECT_THAT(testSubstring3.capacity(), Eq(100));
    EXPECT_THAT(testSubstring3.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring3.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(11, 8);
    auto substr4 = testCxxString.substr(11, 8);
    EXPECT_THAT(substr4.has_value(), Eq(true));
    auto testSubstring4 = substr4.value();
    EXPECT_THAT(testSubstring4.capacity(), Eq(100));
    EXPECT_THAT(testSubstring4.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring4.c_str(), StrEq(testStdSubstring));

    testStdSubstring = testStdString.substr(13, 98);
    auto substr5 = testCxxString.substr(13, 98);
    EXPECT_THAT(substr5.has_value(), Eq(true));
    auto testSubstring5 = substr5.value();
    EXPECT_THAT(testSubstring5.capacity(), Eq(100));
    EXPECT_THAT(testSubstring5.size(), Eq(testStdSubstring.size()));
    EXPECT_THAT(testSubstring5.c_str(), StrEq(testStdSubstring));
}

TYPED_TEST(stringTyped_test, SubstrWithInvalidPosFails)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    auto substr = this->testSubject.substr(STRINGCAP + 1, STRINGCAP + 2);
    EXPECT_THAT(substr.has_value(), Eq(false));
}

/// @note template <typename T>
/// iox::cxx::optional<uint64_t> find(const T& t, uint64_t pos = 0) const noexcept
TYPED_TEST(stringTyped_test, FindEmptyStringInEmptyStringWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 5> testString;
    auto res = this->testSubject.find(testString);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0));

    res = this->testSubject.find("");
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0));

    std::string testStdString;
    res = this->testSubject.find(testStdString);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0));
}

TYPED_TEST(stringTyped_test, FindStringInEmptyStringFails)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 5> testString("a");
    auto res = this->testSubject.find(testString);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindStringInNotEmptyStringWorks)
{
    string<10> testString("R2-D2");
    string<100> substring("2");
    auto res = testString.find(substring);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1));

    res = testString.find(substring, 1);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1));

    res = testString.find(substring, 2);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4));
}

TEST(String100, FindNotIncludedStringFails)
{
    string<100> testString("Kernfusionsbaby");
    string<100> substring("abc");
    auto res = testString.find(substring);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find(substring, 0);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find(substring, 50);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindStringLiteralInNotEmptyStringWorks)
{
    string<100> testString1("Mueslimaedchen");
    auto res = testString1.find("lima");
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4));

    res = testString1.find("lima", 2);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4));

    res = testString1.find("e", 10);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(12));

    std::string testStdString{"ice\0ryx", 7};
    string<100> testString2(TruncateToCapacity, testStdString.c_str(), 7);
    res = testString2.find("e\0ry", 0);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(2));
}

TEST(String100, FindNotIncludedStringLiteralFails)
{
    string<100> testString("Kernfusionsbaby");
    auto res = testString.find("abc");
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find("abc", 50);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TYPED_TEST(stringTyped_test, FindSTDStringInEmptyStringFails)
{
    std::string testStdString = "a";
    auto res = this->testSubject.find(testStdString);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindSTDStringInNotEmptyStringWorks)
{
    string<100> testString("R2-D2");
    std::string testStdString = "2";
    auto res = testString.find(testStdString);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1));

    res = testString.find(testStdString, 1);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1));

    res = testString.find(testStdString, 2);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4));
}

TEST(String100, FindNotIncludedSTDStringFails)
{
    string<100> testString("Kernfusionsbaby");
    std::string testStdString = "abc";
    auto res = testString.find(testStdString);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find(testStdString, 0);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find(testStdString, 50);
    EXPECT_THAT(res.has_value(), Eq(false));
}

/// @note template <typename T>
/// iox::cxx::optional<uint64_t> find_first_of(const T& t, uint64_t pos = 0) const noexcept
TYPED_TEST(stringTyped_test, FindFirstOfFailsForEmptyStringInEmptyString)
{
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 5> testString;
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
    using myString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = myString().capacity();
    string<STRINGCAP + 5> testString("a");
    auto res = this->testSubject.find_first_of(testString);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindFirstOfForStringInNotEmptyStringWorks)
{
    string<10> testString("R2-D2");
    string<100> substring1("2");
    auto res = testString.find_first_of(substring1);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1));

    res = testString.find_first_of(substring1, 1);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1));

    res = testString.find_first_of(substring1, 2);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4));

    string<100> substring2("D3R");
    res = testString.find_first_of(substring2);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0));

    res = testString.find_first_of(substring2, 1);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(3));
}

TEST(String100, FindFirstOfForNotIncludedStringFails)
{
    string<100> testString("Kernfusionsbaby");
    string<100> substring("cdG");
    auto res = testString.find_first_of(substring);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of(substring, 0);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of(substring, 50);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindFirstOfForStringLiteralInNotEmptyStringWorks)
{
    string<100> testString1("Mueslimaedchen");
    auto res = testString1.find_first_of("lima");
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4));

    res = testString1.find_first_of("lima", 2);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4));

    res = testString1.find_first_of("e", 10);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(12));

    res = testString1.find_first_of("U3M");
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0));

    std::string testStdString{"ice\0ryx", 7};
    string<100> testString2(TruncateToCapacity, testStdString.c_str(), 7);
    res = testString2.find_first_of("e\0ry", 0);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(2));
}

TEST(String100, FindFirstOfForNotIncludedStringLiteralFails)
{
    string<100> testString("Kernfusionsbaby");
    auto res = testString.find_first_of("cd");
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of("cd", 0);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of("cd", 50);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TYPED_TEST(stringTyped_test, FindFirstOfForSTDStringInEmptyStringFails)
{
    std::string testStdString = "a";
    auto res = this->testSubject.find_first_of(testStdString);
    EXPECT_THAT(res.has_value(), Eq(false));
}

TEST(String100, FindFirstOfForSTDStringInNotEmptyStringWorks)
{
    string<100> testString("R2-D2");
    std::string testStdString1 = "2";
    auto res = testString.find_first_of(testStdString1);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1));

    res = testString.find_first_of(testStdString1, 1);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(1));

    res = testString.find_first_of(testStdString1, 2);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(4));

    std::string testStdString2 = "D3R";
    res = testString.find_first_of(testStdString2);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(0));

    res = testString.find_first_of(testStdString2, 1);
    EXPECT_THAT(res.has_value(), Eq(true));
    EXPECT_THAT(res.value(), Eq(3));
}

TEST(String100, FindFirstOfForNotIncludedSTDStringFails)
{
    string<100> testString("Kernfusionsbaby");
    std::string testStdString = "cd";
    auto res = testString.find_first_of(testStdString);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of(testStdString, 0);
    EXPECT_THAT(res.has_value(), Eq(false));

    res = testString.find_first_of(testStdString, 50);
    EXPECT_THAT(res.has_value(), Eq(false));
}
} // namespace
