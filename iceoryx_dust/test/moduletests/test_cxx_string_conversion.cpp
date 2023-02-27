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

#include "iceoryx_dust/cxx/std_string_support.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::cxx;
using namespace iox;


template <typename T>
class StdString_test : public Test
{
  public:
    T testSubject;

    using stringType = T;
};

using Implementations = Types<string<1>, string<15>, string<100>, string<1000>>;

TYPED_TEST_SUITE(StdString_test, Implementations, );

TYPED_TEST(StdString_test, STDStringToStringConvConstrWithSize0ResultsInSize0)
{
    ::testing::Test::RecordProperty("TEST_ID", "83e1b7b2-8487-4c71-ac86-f4d5d98c1918");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString;
    string<STRINGCAP> fuu = into<lossy<MyString>>(testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(0U));
    EXPECT_THAT(fuu.c_str(), StrEq(""));
}

TYPED_TEST(StdString_test, STDStringToStringConvConstrWithSizeSmallerCapaResultsInSizeSmallerCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "1bd6cd60-0487-4ba2-9e51-3a9297078454");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP - 1U, 'M');
    string<STRINGCAP> fuu = into<lossy<MyString>>(testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP - 1U));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
}

TYPED_TEST(StdString_test, STDStringToStringConvConstrWithSizeCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "afa37f19-fde0-40ab-b1bd-10862f623ae7");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    string<STRINGCAP> fuu = into<lossy<MyString>>(testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString));
}

TYPED_TEST(StdString_test, STDStringToStringConvConstrWithSizeGreaterCapaResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "67cba3f0-30ed-415d-8232-8e8b5898fe04");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP + 1U, 'M');
    string<STRINGCAP> fuu = into<lossy<MyString>>(testString);
    EXPECT_THAT(fuu.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.size(), Eq(STRINGCAP));
    EXPECT_THAT(fuu.c_str(), Eq(testString.substr(0U, STRINGCAP)));
}

TYPED_TEST(StdString_test, EmptyStringToSTDStringConvResultsInZeroSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "753888b8-12e2-4534-a2fd-32b29b457803");
    using MyString = typename TestFixture::stringType;
    MyString sut;
    std::string testString = into<std::string>(sut);
    EXPECT_THAT(testString.size(), Eq(0U));
    EXPECT_THAT(testString.c_str(), StrEq(""));
}

TYPED_TEST(StdString_test, StringOfSizeCapaToSTDStringConvResultsInSizeCapa)
{
    ::testing::Test::RecordProperty("TEST_ID", "50e727f3-c855-4613-9e38-a56429fa5748");
    using MyString = typename TestFixture::stringType;
    MyString sut;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString1(STRINGCAP, 'M');
    EXPECT_THAT(sut.unsafe_assign(testString1.c_str()), Eq(true));
    std::string testString2 = into<std::string>(sut);

    EXPECT_THAT(testString2.size(), Eq(STRINGCAP));
    EXPECT_THAT(testString2.c_str(), StrEq(testString1.substr(0, STRINGCAP)));
}

TYPED_TEST(StdString_test, IoxStringCanBeConvertedToStdString)
{
    ::testing::Test::RecordProperty("TEST_ID", "698be7cc-f911-42be-b512-b5336eb1d4bd");
    using MyString = typename TestFixture::stringType;
    MyString ioxString("B");

    auto sut = into<std::string>(ioxString);

    EXPECT_THAT(sut.c_str(), StrEq(ioxString.c_str()));
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

TYPED_TEST(StdString_test, AppendEmptyStdStringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6b3fb31a-a92b-4013-ba61-12b4846e8593");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    std::string testStdString;
    this->testSubject.append(TruncateToCapacity, testStdString);
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(StdString_test, AppendStdStringToEmptyStringResultsInConcatenatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "2eebdc68-c495-4531-bb66-ee4ede8d86e3");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testStdString = "M";
    this->testSubject.append(TruncateToCapacity, testStdString);
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(1U));
    EXPECT_THAT(this->testSubject.c_str(), StrEq("M"));
}

TYPED_TEST(StdString_test, AppendStdStringResultsInConcatenatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "7159ef46-f441-4cc7-8eff-46b5f3d33597");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    string<STRINGCAP + 5U> sut = "P";
    std::string testStdString("icard");
    sut.append(TruncateToCapacity, testStdString);
    EXPECT_THAT(sut.capacity(), Eq(STRINGCAP + 5U));
    EXPECT_THAT(sut.size(), Eq(6U));
    EXPECT_THAT(sut.c_str(), StrEq("Picard"));
}

TYPED_TEST(StdString_test, AppendTooLargeStdStringResultsInTruncatedString)
{
    ::testing::Test::RecordProperty("TEST_ID", "76a4752d-282f-47ad-87eb-7c8aab982c0c");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    this->testSubject = "M";
    std::string testStdString(STRINGCAP + 1U, 'M');
    this->testSubject.append(TruncateToCapacity, testStdString);
    EXPECT_THAT(this->testSubject.capacity(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.size(), Eq(STRINGCAP));
    EXPECT_THAT(this->testSubject.c_str(), StrEq(testStdString.substr(0, STRINGCAP)));
}

TYPED_TEST(StdString_test, CompareWithStdStringResultPositiveWithDifferentSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "08891d54-3db7-47cd-8e42-9beb7035c044");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP + 6U, 'M');
    string<STRINGCAP + 6U> sut;
    ASSERT_THAT(sut.unsafe_assign(testString.c_str()), Eq(true));
    std::string foo(STRINGCAP, 'M');
    EXPECT_THAT(sut.compare(foo), Gt(0));
}

/// @note constexpr int64_t compare(const std::string& other) const noexcept
TYPED_TEST(StdString_test, CompareWithStdStringResultNegativeWithDifferentSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "fbfa1376-8474-4cee-8c83-5adb6dc115a8");
    using MyString = typename TestFixture::stringType;
    constexpr auto STRINGCAP = MyString::capacity();
    std::string testString(STRINGCAP, 'M');
    ASSERT_THAT(this->testSubject.unsafe_assign(testString.c_str()), Eq(true));
    std::string foo(STRINGCAP + 4, 'M');
    EXPECT_THAT(this->testSubject.compare(foo), Lt(0));
}

// const std::string testStdString = &testCharArray[0];
// EXPECT_THAT(this->testSubject < testStdString, Eq(false));
// EXPECT_THAT(this->testSubject <= testStdString, Eq(true));
// EXPECT_THAT(this->testSubject > testStdString, Eq(false));
// EXPECT_THAT(this->testSubject >= testStdString, Eq(true));

// EXPECT_THAT(testStdString < this->testSubject, Eq(false));
// EXPECT_THAT(testStdString <= this->testSubject, Eq(true));
// EXPECT_THAT(testStdString > this->testSubject, Eq(false));
// EXPECT_THAT(testStdString >= this->testSubject, Eq(true));

// const std::string testStdString = &testCharArray[0];
// EXPECT_THAT(this->testSubject == testStdString, Eq(false));
// EXPECT_THAT(testStdString == this->testSubject, Eq(false));
// EXPECT_THAT(this->testSubject != testStdString, Eq(true));
// EXPECT_THAT(testStdString != this->testSubject, Eq(true));


// std::string testStdString = "M";
// testStdString.reserve(STRINGCAP + 5U);
// EXPECT_THAT(this->testSubject == testStdString, Eq(true));
// EXPECT_THAT(testStdString == this->testSubject, Eq(true));
// EXPECT_THAT(this->testSubject != testStdString, Eq(false));
// EXPECT_THAT(testStdString != this->testSubject, Eq(false));

//     const std::string testStdString = &testCharArray[0];
// EXPECT_THAT(this->testSubject == testStdString, Eq(false));
// EXPECT_THAT(testStdString == this->testSubject, Eq(false));
// EXPECT_THAT(this->testSubject != testStdString, Eq(true));
// EXPECT_THAT(testStdString != this->testSubject, Eq(true));

//     const std::string testStdString = &testCharArray[0];
// EXPECT_THAT(this->testSubject == testStdString, Eq(true));
// EXPECT_THAT(testStdString == this->testSubject, Eq(true));
// EXPECT_THAT(this->testSubject != testStdString, Eq(false));
// EXPECT_THAT(testStdString != this->testSubject, Eq(false));

// EXPECT_THAT(sutGreater < testStdString, Eq(false));
// EXPECT_THAT(sutGreater <= testStdString, Eq(false));
// EXPECT_THAT(sutGreater > testStdString, Eq(true));
// EXPECT_THAT(sutGreater >= testStdString, Eq(true));
// EXPECT_THAT(testStdString < sutGreater, Eq(true));
// EXPECT_THAT(testStdString <= sutGreater, Eq(true));
// EXPECT_THAT(testStdString > sutGreater, Eq(false));
// EXPECT_THAT(testStdString >= sutGreater, Eq(false));

// EXPECT_THAT(sutLess < testStdString, Eq(true));
// EXPECT_THAT(sutLess <= testStdString, Eq(true));
// EXPECT_THAT(sutLess > testStdString, Eq(false));
// EXPECT_THAT(sutLess >= testStdString, Eq(false));
// EXPECT_THAT(testStdString < sutLess, Eq(false));
// EXPECT_THAT(testStdString <= sutLess, Eq(false));
// EXPECT_THAT(testStdString > sutLess, Eq(true));
// EXPECT_THAT(testStdString >= sutLess, Eq(true));


// EXPECT_THAT(sutGreater < testStdString, Eq(false));
// EXPECT_THAT(sutGreater <= testStdString, Eq(false));
// EXPECT_THAT(sutGreater > testStdString, Eq(true));
// EXPECT_THAT(sutGreater >= testStdString, Eq(true));
// EXPECT_THAT(testStdString < sutGreater, Eq(true));
// EXPECT_THAT(testStdString <= sutGreater, Eq(true));
// EXPECT_THAT(testStdString > sutGreater, Eq(false));
// EXPECT_THAT(testStdString >= sutGreater, Eq(false));

// EXPECT_THAT(sutLess < testStdString, Eq(true));
// EXPECT_THAT(sutLess <= testStdString, Eq(true));
// EXPECT_THAT(sutLess > testStdString, Eq(false));
// EXPECT_THAT(sutLess >= testStdString, Eq(false));
// EXPECT_THAT(testStdString < sutLess, Eq(false));
// EXPECT_THAT(testStdString <= sutLess, Eq(false));
// EXPECT_THAT(testStdString > sutLess, Eq(true));
// EXPECT_THAT(testStdString >= sutLess, Eq(true));

// const std::string testStdString = &testCharArray[0];
// EXPECT_THAT(this->testSubject < testStdString, Eq(false));
// EXPECT_THAT(this->testSubject <= testStdString, Eq(true));
// EXPECT_THAT(this->testSubject > testStdString, Eq(false));
// EXPECT_THAT(this->testSubject >= testStdString, Eq(true));

// EXPECT_THAT(testStdString < this->testSubject, Eq(false));
// EXPECT_THAT(testStdString <= this->testSubject, Eq(true));
// EXPECT_THAT(testStdString > this->testSubject, Eq(false));
// EXPECT_THAT(testStdString >= this->testSubject, Eq(true));

// const std::string testStdString;
// EXPECT_THAT(this->testSubject.compare(testStdString), Gt(0));

//     EXPECT_THAT(this->testSubject.compare(testStdString), Eq(0));

// const std::string testStdString = &testCharArray[0];
// EXPECT_THAT(this->testSubject.compare(testStdString.c_str()), Lt(0));

// const std::string testStdString = &testCharArray[0];
// EXPECT_THAT(this->testSubject.compare(testStdString.c_str()), Gt(0));


// std::string testStdString(STRINGCAP, 'M');
// testStdString.reserve(STRINGCAP + 13U);
// EXPECT_THAT(this->testSubject.compare(testStdString.c_str()), Eq(0));

// std::string testStdString = 'a';
// res = this->testSubject.find_first_of(testStdString);
// EXPECT_THAT(res.has_value(), Eq(false));
} // namespace
