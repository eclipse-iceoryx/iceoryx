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
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu(this->testSubject);
    EXPECT_THAT(fuu.capacity(), Eq(stringCap));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, CopyConstructStringOfSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<stringCap> fuu(this->testSubject);
    EXPECT_THAT(fuu.capacity(), Eq(stringCap));
    EXPECT_THAT(fuu.size(), Eq(stringCap));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
}

/// @note string(string&& other) noexcept
TYPED_TEST(stringTyped_test, MoveConstructionWithStringOfSize0Works)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testString(std::move(this->testSubject));
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveConstructionWithStringOfSizeSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap - 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<stringCap> fuu(std::move(this->testSubject));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.size(), Eq(stringCap - 1));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveConstructionWithStringOfSizeCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<stringCap> fuu(std::move(this->testSubject));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.size(), Eq(stringCap));
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
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu;
    fuu = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, CopyAssignmentWithStringOfSizeSmallerCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap - 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<stringCap> fuu;
    fuu = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(stringCap - 1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testString.substr(0, stringCap - 1)));
    EXPECT_THAT(fuu.size(), Eq(stringCap - 1));
    EXPECT_THAT(fuu.c_str(), StrEq(testString.substr(0, stringCap - 1)));
}

TYPED_TEST(stringTyped_test, CopyAssignmentWithStringOfSizeCapaWorks)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<stringCap> fuu;
    fuu = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(stringCap));
    EXPECT_THAT(this->testSubject.c_str(), Eq(testString));
    EXPECT_THAT(fuu.size(), Eq(stringCap));
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
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfStringWithSmallerSizeResultsInSmallerSize)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap - 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<stringCap> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(fuu.size(), Eq(stringCap - 1));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, MoveAssignmentOfStringWithSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<stringCap> fuu;
    fuu = std::move(this->testSubject);
    EXPECT_THAT(fuu.size(), Eq(stringCap));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

/// @note template <int N>
/// string(const char (&other)[N]) noexcept
TYPED_TEST(stringTyped_test, CharToStringConvConstrWithSize0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("");
    EXPECT_THAT(fuu.capacity(), Eq(stringCap));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, CharToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    char testChar[stringCap];
    for (uint64_t i = 0; i < stringCap - 1; i++)
    {
        testChar[i] = 'M';
    }
    testChar[stringCap - 1] = '\0';
    string<stringCap> testSubject(testChar);
    EXPECT_THAT(testSubject.capacity(), Eq(stringCap));
    EXPECT_THAT(testSubject.size(), Eq(stringCap - 1));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
}

/// @note string(TruncateToCapacity_t, const char* const other) noexcept
TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithSize0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu(TruncateToCapacity, "");
    EXPECT_THAT(fuu.capacity(), Eq(stringCap));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    char testChar[stringCap];
    for (uint64_t i = 0; i < stringCap - 1; i++)
    {
        testChar[i] = 'M';
    }
    testChar[stringCap - 1] = '\0';
    string<stringCap> testSubject(TruncateToCapacity, testChar);
    EXPECT_THAT(testSubject.capacity(), Eq(stringCap));
    EXPECT_THAT(testSubject.size(), Eq(stringCap - 1));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConvConstrWithSizeGreaterCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    char testChar[stringCap + 1];
    for (uint64_t i = 0; i < stringCap; i++)
    {
        testChar[i] = 'M';
    }
    testChar[stringCap] = '\0';
    string<stringCap> testSubject(TruncateToCapacity, testChar);
    EXPECT_THAT(testSubject.capacity(), Eq(stringCap));
    EXPECT_THAT(testSubject.size(), Eq(stringCap));
}

/// @note string(TruncateToCapacity_t, const std::string& other) noexcept
TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSize0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString;
    string<stringCap> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(stringCap));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSizeSmallerCapaResultsInSizeSmallerCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap - 1, 'M');
    string<stringCap> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(stringCap));
    EXPECT_THAT(fuu.size(), Eq(stringCap - 1));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
}

TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap, 'M');
    string<stringCap> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(stringCap));
    EXPECT_THAT(fuu.size(), Eq(stringCap));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
}

TYPED_TEST(stringTyped_test, UnsafeSTDStringToStringConvConstrWithSizeGreaterCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap + 1, 'M');
    string<stringCap> fuu(TruncateToCapacity, testString);
    EXPECT_THAT(fuu.capacity(), Eq(stringCap));
    EXPECT_THAT(fuu.size(), Eq(stringCap));
    EXPECT_THAT(fuu.c_str(), Eq(testString.substr(0, stringCap)));
}

/// @note string(TruncateToCapacity_t, const char* const other, const uint64_t count) noexcept
TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithCount0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu(TruncateToCapacity, "Yoda", 0);
    EXPECT_THAT(fuu.capacity(), Eq(stringCap));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithCountEqCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap, 'M');
    string<stringCap> testSubject(TruncateToCapacity, testString.c_str(), stringCap);
    EXPECT_THAT(testSubject.capacity(), Eq(stringCap));
    EXPECT_THAT(testSubject.size(), Eq(stringCap));
    EXPECT_THAT(testSubject.c_str(), StrEq(testString));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithCountGreaterCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap + 1, 'M');
    string<stringCap> testSubject(TruncateToCapacity, testString.c_str(), stringCap + 1);
    EXPECT_THAT(testSubject.capacity(), Eq(stringCap));
    EXPECT_THAT(testSubject.size(), Eq(stringCap));
    EXPECT_THAT(testSubject.c_str(), StrEq(testString.substr(0, stringCap)));
}

TEST(String100, UnsafeCharToStringConstrIncludingNullCharWithCountResultsInSizeCount)
{
    std::string testString{"ice\0ryx", 7};
    string<100> testSubject(TruncateToCapacity, testString.c_str(), 7);
    EXPECT_THAT(testSubject.capacity(), Eq(100));
    EXPECT_THAT(testSubject.size(), Eq(7));
    EXPECT_THAT(testSubject.c_str(), StrEq("ice\0ryx"));
}

TEST(CharArrayAssignment, AssignCharArrayWithStringSizeLessThanArraySize)
{
    char testString[20] = "iceoryx";
    string<20> testSubject(testString);
    EXPECT_THAT(testSubject.size(), Eq(7));
    EXPECT_THAT(testSubject.c_str(), StrEq("iceoryx"));
}

TEST(CharArrayAssignment, AssignZeroTerminatedCharArrayOfSizeForFullCapa)
{
    char testString[8] = "iceoryx";
    string<7> testSubject(testString);
    EXPECT_THAT(testSubject.size(), Eq(7));
    EXPECT_THAT(testSubject.c_str(), StrEq("iceoryx"));
}

TEST(CharArrayAssignment, AssignNonZeroTerminatedCharArrayOfSizeForFullCapa)
{
    char testString[8] = "iceoryx";
    testString[7] = 'x'; // overwrite the 0 termination
    string<7> testSubject(testString);
    EXPECT_THAT(testSubject.size(), Eq(7));
    EXPECT_THAT(testSubject.c_str(), StrEq("iceoryx"));
}

TYPED_TEST(stringTyped_test, UnsafeCharToStringConstrWithNullPtrResultsEmptyString)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu(TruncateToCapacity, nullptr, stringCap);
    EXPECT_THAT(fuu.capacity(), Eq(stringCap));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

/// @note template <int N>
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
    constexpr auto stringCap = myString().capacity();
    char testChar[stringCap];
    for (uint64_t i = 0; i < stringCap - 1; i++)
    {
        testChar[i] = 'M';
    }
    testChar[stringCap - 1] = '\0';
    string<stringCap> testSubject;
    testSubject = testChar;
    EXPECT_THAT(testSubject.size(), Eq(stringCap - 1));
    EXPECT_THAT(testSubject.c_str(), StrEq(testChar));
}

/// @note string& assign(const string& str) noexcept
TYPED_TEST(stringTyped_test, SelfAssignmentIsExcluded)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testSubject;
    testSubject.assign(testSubject);
    EXPECT_THAT(testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, AssignStringOfSize0ResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu;
    this->testSubject.assign(fuu);
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
    EXPECT_THAT(fuu.size(), Eq(0));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, AssignStringOfSizeCapaResultsInSizeCapa)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap, 'M');
    string<stringCap> fuu(TruncateToCapacity, testString);
    this->testSubject.assign(fuu);
    EXPECT_THAT(this->testSubject.size(), Eq(stringCap));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testString));
    EXPECT_THAT(fuu.size(), Eq(stringCap));
    EXPECT_THAT(fuu.c_str(), StrEq(testString));
}

/// @note template <int N>
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
    constexpr auto stringCap = myString().capacity();
    char testChar[stringCap];
    for (uint64_t i = 0; i < stringCap - 1; i++)
    {
        testChar[i] = 'M';
    }
    testChar[stringCap - 1] = '\0';
    string<stringCap> testSubject;
    testSubject.assign(testChar);
    EXPECT_THAT(testSubject.size(), Eq(stringCap - 1));
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
    constexpr auto stringCap = myString().capacity();
    std::vector<char> testCharstring(stringCap, 'M');
    testCharstring.emplace_back('\0');
    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(stringCap));
}

TYPED_TEST(stringTyped_test, UnsafeAssignCStringOfSizeGreaterCapaResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::vector<char> testCharstring(stringCap + 1, 'M');
    testCharstring.emplace_back('\0');
    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfInvalidCStringFails)
{
    this->testSubject = "L";

    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::vector<char> testCharstring(stringCap + 1, 'M');
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
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(stringCap));
}

TYPED_TEST(stringTyped_test, UnsafeAssignSTDStringOfSizeGreaterCapaResultsInSize0)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap + 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, AssignOfInvalidSTDStringFails)
{
    this->testSubject = "L";

    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap + 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(1));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("L"));
}

/// @note int compare(const string other) const noexcept
TYPED_TEST(stringTyped_test, CompareEqStringsResultsInZero)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    string<stringCap> fuu;
    EXPECT_THAT(fuu.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.compare(this->testSubject), Eq(0));
    EXPECT_THAT(this->testSubject.compare(fuu), Eq(0));
}

TYPED_TEST(stringTyped_test, CompareResultNegative)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString1(stringCap, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    string<stringCap> fuu;
    std::string testString2(stringCap, 'L');
    EXPECT_THAT(fuu.unsafe_assign(testString2), Eq(true));
    EXPECT_THAT(fuu.compare(this->testSubject), Lt(0));
}

TYPED_TEST(stringTyped_test, CompareResultPositive)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    std::string testString1(stringCap, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    string<stringCap> fuu;
    std::string testString2(stringCap, 'L');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString2), Eq(true));
    EXPECT_THAT(this->testSubject.compare(fuu), Gt(0));
}

TYPED_TEST(stringTyped_test, CompareWithEmptyStringResultsInPositive)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
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

/// @note bool operator==(const string& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorEqualResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    EXPECT_THAT(fuu == fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorEqualResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    string<stringCap> bar("L");
    EXPECT_THAT(fuu == bar, Eq(false));
}

/// @note bool operator!=(const string& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorNotEqualResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    EXPECT_THAT(fuu != fuu, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorNotEqualResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    string<stringCap> bar("L");
    EXPECT_THAT(fuu != bar, Eq(true));
}

/// @note bool operator<(const string& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorLesserResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    string<stringCap> bar("L");
    EXPECT_THAT(bar < fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    string<stringCap> bar("L");
    EXPECT_THAT(fuu < bar, Eq(false));
    EXPECT_THAT(fuu < fuu, Eq(false));
}

/// @note bool operator<=(const string& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorLesserEqResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    string<stringCap> bar("L");
    EXPECT_THAT(this->testSubject <= fuu, Eq(true));
    EXPECT_THAT(bar <= fuu, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorLesserEqResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    string<stringCap> bar("L");
    EXPECT_THAT(fuu <= bar, Eq(false));
}

/// @note bool operator>(const string& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorGreaterResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    string<stringCap> bar("L");
    EXPECT_THAT(fuu > bar, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    string<stringCap> bar("L");
    EXPECT_THAT(bar > fuu, Eq(false));
    EXPECT_THAT(bar > bar, Eq(false));
}

/// @note bool operator>=(const string& rhs) const noexcept
TYPED_TEST(stringTyped_test, CompareOperatorGreaterEqResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    string<stringCap> bar("L");
    this->testSubject = "M";
    EXPECT_THAT(fuu >= bar, Eq(true));
    EXPECT_THAT(fuu >= this->testSubject, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorGreaterEqResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> fuu("M");
    string<stringCap> bar("L");
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
    constexpr auto stringCap = myString().capacity();
    std::string testString1(stringCap, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString1), Eq(true));
    std::string testString2 = std::string(this->testSubject);
    EXPECT_THAT(testString2.size(), Eq(stringCap));
    EXPECT_THAT(testString2.c_str(), StrEq(testString1.substr(0, stringCap)));
}

/// @note template <uint64_t Capacity>
/// inline bool operator==(const std::string& lhs, const string<Capacity>& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorSTDStringEqualFixedStringResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testSTDString == testFixedString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringEqualFixedStringWithSameSizeResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testSTDString == testFixedString, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringEqualFixedStringResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "ML";
    EXPECT_THAT(testSTDString == testFixedString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline bool operator==(const string<Capacity>& lhs, const std::string& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorFixedStringEqualSTDStringResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testFixedString == testSTDString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringEqualSTDStringWithSameSizeResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testFixedString == testSTDString, Eq(false));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringEqualSTDStringResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "ML";
    EXPECT_THAT(testFixedString == testSTDString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline bool operator!=(const std::string& lhs, const string<Capacity>& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorSTDStringNotEqualFixedStringResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "Ferdinand Spitzschnüffler";
    EXPECT_THAT(testSTDString != testFixedString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringNotEqualFixedStringWithSameSizeResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testSTDString != testFixedString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorSTDStringNotEqualFixedStringResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "M";
    EXPECT_THAT(testSTDString != testFixedString, Eq(false));
}

/// @note template <uint64_t Capacity>
/// inline bool operator!=(const string<Capacity>& lhs, const std::string& rhs)
TYPED_TEST(stringTyped_test, CompareOperatorFixedStringNotEqualSTDStringResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "Müslimädchen";
    EXPECT_THAT(testFixedString != testSTDString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringNotEqualSTDStringWithSameSizeResultTrue)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
    std::string testSTDString = "L";
    EXPECT_THAT(testFixedString != testSTDString, Eq(true));
}

TYPED_TEST(stringTyped_test, CompareOperatorFixedStringNotEqualSTDStringResultFalse)
{
    using myString = typename TestFixture::stringType;
    constexpr auto stringCap = myString().capacity();
    string<stringCap> testFixedString("M");
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
    constexpr auto stringCap = myString().capacity();
    std::string testString(stringCap, 'M');
    string<stringCap> testFixedString(TruncateToCapacity, testString);
    std::ostringstream testStream;
    testStream << testFixedString;
    EXPECT_THAT(testStream.str(), Eq(testFixedString.c_str()));
}

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
