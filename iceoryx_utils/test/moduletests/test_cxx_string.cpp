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

template <typename stringType>
class stringTyped_test : public Test
{
  protected:
    stringType testSubject;
};

using Implementations = Types<string<15>, string<100>, string<1000>>;
TYPED_TEST_CASE(stringTyped_test, Implementations);

TYPED_TEST(stringTyped_test, EmptyInitializationResultsInSize0)
{
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, EmptyInitializationResultsInEmptyString)
{
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, AssignCStringOfSize0WithOperatorResultsInSize0)
{
    this->testSubject = "";
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, AssignCStringOfSizeSmallerCapaWithOperatorResultsInSizeSmallerCapa)
{
    this->testSubject = "R2D2";
    EXPECT_THAT(this->testSubject.size(), Eq(4));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("R2D2"));
}

TYPED_TEST(stringTyped_test, SelfCopyAssignmentExcluded)
{
    this->testSubject = "C-3PO";
    this->testSubject = this->testSubject;
    EXPECT_THAT(this->testSubject.size(), Eq(5));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("C-3PO"));
}

TYPED_TEST(stringTyped_test, SelfMoveAssignmentExcluded)
{
    this->testSubject = "C-3PO";
    this->testSubject = std::move(this->testSubject);
    EXPECT_THAT(this->testSubject.size(), Eq(5));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("C-3PO"));
}

TYPED_TEST(stringTyped_test, FreshlyAssignNothingResultsInZeroSize)
{
    this->testSubject.assign("");
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, ReassignNothingResultsInZeroSize)
{
    this->testSubject.assign("Picard");
    this->testSubject.assign("");
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, AssignCStringOfSizeSmallerCapaResultsInSizeSmallerCapa)
{
    this->testSubject.assign("R2D2");
    EXPECT_THAT(this->testSubject.size(), Eq(4));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("R2D2"));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfCStringOfSize0ResultsInSize0)
{
    EXPECT_THAT(this->testSubject.unsafe_assign(""), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfCStringOfSize4ResultsInSize4)
{
    EXPECT_THAT(this->testSubject.unsafe_assign("Worf"), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(4));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("Worf"));
}

TYPED_TEST(stringTyped_test, UnsafeAssignCStringOfSizeCapaResultsInSizeCapa)
{
    uint64_t testSubjectCapacity = this->testSubject.capacity();
    std::vector<char> testCharstring(testSubjectCapacity, 'M');
    testCharstring.emplace_back('\0');
    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(testSubjectCapacity));
}

TYPED_TEST(stringTyped_test, UnsafeAssignCStringOfSizeGreaterCapaResultsInSize0)
{
    uint64_t testSubjectCapacity = this->testSubject.capacity();
    std::vector<char> testCharstring(testSubjectCapacity + 1, 'M');
    testCharstring.emplace_back('\0');
    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfInvalidCStringFails)
{
    this->testSubject = "R2D2";

    uint64_t testSubjectCapacity = this->testSubject.capacity();
    std::vector<char> testCharstring(testSubjectCapacity + 1, 'M');
    testCharstring.emplace_back('\0');

    EXPECT_THAT(this->testSubject.unsafe_assign(testCharstring.data()), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(4));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("R2D2"));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfCharPointerPointingToSameAddress)
{
    this->testSubject = "BB-8";
    const char* fuu = this->testSubject.c_str();
    EXPECT_THAT(this->testSubject.unsafe_assign(fuu), Eq(false));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfSTDStringOfSize0ResultsInSize0)
{
    std::string testString;
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, UnsafeAssignOfSTDStringOfSize4ResultsInSize4)
{
    std::string testString = "Worf";
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(4));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("Worf"));
}

TYPED_TEST(stringTyped_test, AssignSTDStringOfSizeCapaResultsInSizeCapa)
{
    uint64_t testSubjectCapacity = this->testSubject.capacity();
    std::string testString(testSubjectCapacity, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(this->testSubject.size(), Eq(testSubjectCapacity));
}

TYPED_TEST(stringTyped_test, AssignSTDStringOfSizeGreaterCapaResultsInSize0)
{
    uint64_t testSubjectCapacity = this->testSubject.capacity();
    std::string testString(testSubjectCapacity + 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(0));
}

TYPED_TEST(stringTyped_test, AssignOfInvalidSTDStringFails)
{
    this->testSubject = "Worf";

    uint64_t testSubjectCapacity = this->testSubject.capacity();
    std::string testString(testSubjectCapacity + 1, 'M');
    EXPECT_THAT(this->testSubject.unsafe_assign(testString), Eq(false));
    EXPECT_THAT(this->testSubject.size(), Eq(4));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("Worf"));
}

TYPED_TEST(stringTyped_test, EmptyStringToSTDStringConvResultsInZeroSize)
{
    std::string testString = std::string(this->testSubject);
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq(""));
}

TYPED_TEST(stringTyped_test, StringToSTDStringConvResultsInStringSize)
{
    this->testSubject = "Picard";
    std::string testString = std::string(this->testSubject);
    EXPECT_THAT(testString.size(), Eq(6));
    EXPECT_THAT(testString.c_str(), StrEq("Picard"));
}

TEST(String1, CopyConstructEmptyStringResultsInSize0)
{
    string<1> fuu;
    string<1> testSubject(fuu);
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, CharToStringConvConstrWithSize0ResultsInSize0)
{
    string<1> testSubject("");
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, CharToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    string<1> testSubject("M");
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("M"));
}

TEST(String1, UnsafeCharToStringConvConstrWithSize0ResultsInSize0)
{
    string<1> testSubject(UnsafeCheckPreconditions, "");
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, UnsafeCharToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    string<1> testSubject(UnsafeCheckPreconditions, "M");
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("M"));
}

TEST(String1, UnsafeCharToStringConvConstrWithSizeGreaterCapaResultsInSize1)
{
    string<1> testSubject(UnsafeCheckPreconditions, "Ferdinand Spitzschnüffler");
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("F"));
}

TEST(String1, UnsafeSTDStringToStringConvConstrWithSize0ResultsInSize0)
{
    std::string testString;
    string<1> testSubject(UnsafeCheckPreconditions, testString);
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, UnsafeSTDStringToStringConvConstrWithSizeGreaterCapaResultsInSizeCapa)
{
    std::string testString(101, 'M');
    string<1> testSubject(UnsafeCheckPreconditions, testString);
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("M"));
}

TEST(String1, UnsafeCharToStringConvConstrWithCount0ResultsInSize0)
{
    string<1> testSubject(UnsafeCheckPreconditions, "Yoda", 0);
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, UnsafeCharToStringConvConstrWithCountEqCapaResultsInSizeCapa)
{
    string<1> testSubject(UnsafeCheckPreconditions, "M", 1);
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("M"));
}

TEST(String1, UnsafeCharToStringConvConstrWithCountGreaterCapaResultsInSizeCapa)
{
    string<1> testSubject(UnsafeCheckPreconditions, "Yoda", 4);
    EXPECT_THAT(testSubject.capacity(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("Y"));
}

TEST(String1, MoveConstructionWithStringOfSize0Works)
{
    string<1> testSubject;
    string<1> testString(std::move(testSubject));
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq(""));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, MoveConstructionWithStringOfSizeCapaWorks)
{
    string<1> testSubject("M");
    string<1> testString(std::move(testSubject));
    EXPECT_THAT(testString.size(), Eq(1));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq("M"));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, MoveSelfAssingmentExcluded)
{
    string<1> testSubject("M");
    testSubject = std::move(testSubject);
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("M"));
}

TEST(String1, AssignStringOfSize0WithOperatorResultsInSize0)
{
    string<1> testSubject1;
    string<1> testSubject2 = "";
    testSubject1 = testSubject2;
    EXPECT_THAT(testSubject1.size(), Eq(0));
    EXPECT_THAT(testSubject1.c_str(), StrEq(""));
    EXPECT_THAT(testSubject2.size(), Eq(0));
    EXPECT_THAT(testSubject2.c_str(), StrEq(""));
}

TEST(String1, AssignStringOfSizeCapaWithOperatorResultsInSizeCapa)
{
    string<1> testSubject1;
    string<1> testSubject2 = "M";
    testSubject1 = testSubject2;
    EXPECT_THAT(testSubject1.size(), Eq(1));
    EXPECT_THAT(testSubject1.c_str(), StrEq("M"));
    EXPECT_THAT(testSubject2.size(), Eq(1));
    EXPECT_THAT(testSubject2.c_str(), StrEq("M"));
}

TEST(String1, AssignCStringOfSize0WithOperatorResultsInSize0)
{
    string<1> testSubject;
    testSubject = "";
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, AssignCStringOfSizeCapaWithOperatorResultsInSizeCapa)
{
    string<1> testSubject;
    testSubject = "M";
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("M"));
}

TEST(String1, MoveAssignmentOfStringWithSize0ResultsInSize0)
{
    string<1> testSubject1;
    string<1> testSubject2("");
    testSubject1 = std::move(testSubject2);
    EXPECT_THAT(testSubject1.size(), Eq(0));
    EXPECT_THAT(testSubject2.size(), Eq(0));
    EXPECT_THAT(testSubject1.c_str(), StrEq(""));
    EXPECT_THAT(testSubject2.c_str(), StrEq(""));
}

TEST(String1, MoveAssignmentOfStringWithSizeCapaResultsInSizeCapa)
{
    string<1> testSubject1("M");
    string<1> testSubject2("L");
    testSubject1 = std::move(testSubject2);
    EXPECT_THAT(testSubject1.size(), Eq(1));
    EXPECT_THAT(testSubject2.size(), Eq(0));
    EXPECT_THAT(testSubject1.c_str(), StrEq("L"));
    EXPECT_THAT(testSubject2.c_str(), StrEq(""));
}

TEST(String1, AssignStringOfSize0ResultsInSize0)
{
    string<1> testSubject1("");
    string<1> testSubject2("M");
    testSubject2.assign(testSubject1);
    EXPECT_THAT(testSubject1.size(), Eq(0));
    EXPECT_THAT(testSubject1.c_str(), StrEq(""));
    EXPECT_THAT(testSubject2.size(), Eq(0));
    EXPECT_THAT(testSubject2.c_str(), StrEq(""));
}

TEST(String1, AssignStringOfSizeCapaResultsInSizeCapa)
{
    string<1> testSubject1("L");
    string<1> testSubject2("M");
    testSubject2.assign(testSubject1);
    EXPECT_THAT(testSubject1.size(), Eq(1));
    EXPECT_THAT(testSubject1.c_str(), StrEq("L"));
    EXPECT_THAT(testSubject2.size(), Eq(1));
    EXPECT_THAT(testSubject2.c_str(), StrEq("L"));
}

TEST(String1, AssignCStringOfSizeCapaResultsInSizeCapa)
{
    string<1> testSubject;
    testSubject.assign("M");
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("M"));
}

TEST(String1, FreshlyAssignNothingResultsInZeroSize)
{
    string<1> testSubject;
    testSubject.assign("");
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, ReassignNothingResultsInZeroSize)
{
    string<1> testSubject("M");
    testSubject.assign("");
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, SelfAssignmentIsExcluded)
{
    string<1> testSubject;
    testSubject.assign(testSubject);
    EXPECT_THAT(testSubject.size(), Eq(0));
}

TEST(String1, UnsafeAssignOfCStringOfSize0ResultsInSize0)
{
    string<1> testSubject;
    EXPECT_THAT(testSubject.unsafe_assign(""), Eq(true));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, UnsafeAssignOfCStringOfSizeCapaResultsInSizeCapa)
{
    string<1> testSubject;
    EXPECT_THAT(testSubject.unsafe_assign("M"), Eq(true));
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("M"));
}

TEST(String1, UnsafeAssignOfCStringOfSizeGreaterCapaResultsInSize0)
{
    string<1> testSubject;
    EXPECT_THAT(testSubject.unsafe_assign("ML"), Eq(false));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, UnsafeAssignOfCharPointerPointingToSameAddress)
{
    string<1> testSubject("");
    const char* fuu = testSubject.c_str();
    EXPECT_THAT(testSubject.unsafe_assign(fuu), Eq(false));
}

TEST(String1, UnsafeAssignOfSTDStringOfSize0ResultsInSize0)
{
    string<1> testSubject;
    std::string testString;
    EXPECT_THAT(testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, UnsafeAssignOfSTDStringOfSizeCapaResultsInSizeCapa)
{
    string<1> testSubject;
    std::string testString(1, 'M');
    EXPECT_THAT(testSubject.unsafe_assign(testString), Eq(true));
    EXPECT_THAT(testSubject.size(), Eq(1));
    EXPECT_THAT(testSubject.c_str(), StrEq("M"));
}

TEST(String1, UnsafeAssignOfSTDStringOfSizeGreaterCapaResultsInSize0)
{
    string<1> testSubject;
    std::string testString(29, 'M');
    EXPECT_THAT(testSubject.unsafe_assign(testString), Eq(false));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String1, CompareEqStringsResultsInZero)
{
    string<1> testSubject1("M");
    string<1> testSubject2("M");
    EXPECT_THAT(testSubject1.compare(testSubject1), Eq(0));
    EXPECT_THAT(testSubject1.compare(testSubject2), Eq(0));
}

TEST(String1, CompareResultNegative)
{
    string<1> testSubject1("L");
    string<1> testSubject2("M");
    EXPECT_THAT(testSubject1.compare(testSubject2), Lt(0));
}

TEST(String1, CompareResultPositive)
{
    string<1> testSubject1("L");
    string<1> testSubject2("M");
    EXPECT_THAT(testSubject2.compare(testSubject1), Gt(0));
}

TEST(String1, CompareWithEmptyStringResultsInPositive)
{
    string<1> testSubject1("");
    string<1> testSubject2("M");
    EXPECT_THAT(testSubject2.compare(testSubject1), Gt(0));
}

TEST(String1, CompareOperatorEqualResultTrue)
{
    string<1> testSubject1("M");
    EXPECT_THAT(testSubject1 == testSubject1, Eq(true));
}

TEST(String1, CompareOperatorEqualResultFalse)
{
    string<1> testSubject1("M");
    string<1> testSubject2("L");
    EXPECT_THAT(testSubject1 == testSubject2, Eq(false));
}

TEST(String1, CompareOperatorNotEqualResultFalse)
{
    string<1> testSubject1("M");
    EXPECT_THAT(testSubject1 != testSubject1, Eq(false));
}

TEST(String1, CompareOperatorNotEqualResultTrue)
{
    string<1> testSubject1("M");
    string<1> testSubject2("L");
    EXPECT_THAT(testSubject1 != testSubject2, Eq(true));
}

TEST(String1, CompareOperatorLesserResultTrue)
{
    string<1> testSubject1("L");
    string<1> testSubject2("M");
    EXPECT_THAT(testSubject1 < testSubject2, Eq(true));
}

TEST(String1, CompareOperatorLesserResultFalse)
{
    string<1> testSubject1("M");
    string<1> testSubject2("L");
    EXPECT_THAT(testSubject1 < testSubject2, Eq(false));
    EXPECT_THAT(testSubject1 < testSubject1, Eq(false));
}

TEST(String1, CompareOperatorLesserEqResultTrue)
{
    string<1> testSubject1("L");
    string<1> testSubject2("M");
    string<1> testSubject3("M");
    EXPECT_THAT(testSubject1 <= testSubject2, Eq(true));
    EXPECT_THAT(testSubject2 <= testSubject3, Eq(true));
}

TEST(String1, CompareOperatorLesserEqResultFalse)
{
    string<1> testSubject1("M");
    string<1> testSubject2("L");
    EXPECT_THAT(testSubject1 <= testSubject2, Eq(false));
}

TEST(String1, CompareOperatorGreaterResultTrue)
{
    string<1> testSubject1("M");
    string<1> testSubject2("L");
    EXPECT_THAT(testSubject1 > testSubject2, Eq(true));
}

TEST(String1, CompareOperatorGreaterResultFalse)
{
    string<1> testSubject1("L");
    string<1> testSubject2("M");
    EXPECT_THAT(testSubject1 > testSubject2, Eq(false));
    EXPECT_THAT(testSubject1 > testSubject1, Eq(false));
}

TEST(String1, CompareOperatorGreaterEqResultTrue)
{
    string<1> testSubject1("L");
    string<1> testSubject2("M");
    string<1> testSubject3("M");
    EXPECT_THAT(testSubject2 >= testSubject1, Eq(true));
    EXPECT_THAT(testSubject2 >= testSubject3, Eq(true));
}

TEST(String1, CompareOperatorGreaterEqResultFalse)
{
    string<1> testSubject1("M");
    string<1> testSubject2("L");
    EXPECT_THAT(testSubject2 >= testSubject1, Eq(false));
}

TEST(String1, EmptyStringToSTDStringConvResultsInZeroSize)
{
    string<1> testSubject = "";
    std::string testString = std::string(testSubject);
    EXPECT_THAT(testString.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq(""));
}

TEST(String100, CharToStringConvConstrWithSize4ResultsInSize4)
{
    string<100> testSubject("R2D2");
    EXPECT_THAT(testSubject.capacity(), Eq(100));
    EXPECT_THAT(testSubject.size(), Eq(4));
    EXPECT_THAT(testSubject.c_str(), StrEq("R2D2"));
}

TEST(String100, MoveConstructionWithStringOfSizeSmallerCapaWorks)
{
    string<100> testSubject("Picard");
    string<100> testString(std::move(testSubject));
    EXPECT_THAT(testString.size(), Eq(6));
    EXPECT_THAT(testSubject.size(), Eq(0));
    EXPECT_THAT(testString.c_str(), StrEq("Picard"));
    EXPECT_THAT(testSubject.c_str(), StrEq(""));
}

TEST(String100, MoveAssignmentOfStringWithSmallerSizeResultsInSmallerSize)
{
    string<100> testSubject1("Data");
    string<100> testSubject2("Picard");
    testSubject1 = std::move(testSubject2);
    EXPECT_THAT(testSubject1.size(), Eq(6));
    EXPECT_THAT(testSubject2.size(), Eq(0));
    EXPECT_THAT(testSubject1.c_str(), StrEq("Picard"));
    EXPECT_THAT(testSubject2.c_str(), StrEq(""));
}

TEST(String100, UnsafeCharToStringConvConstrIncludingNullCharWithCountResultsInSizeCount)
{
    string<100> testSubject(UnsafeCheckPreconditions, "abc\0", 4);
    EXPECT_THAT(testSubject.capacity(), Eq(100));
    EXPECT_THAT(testSubject.size(), Eq(4));
    EXPECT_THAT(testSubject.c_str(), StrEq("abc"));
}