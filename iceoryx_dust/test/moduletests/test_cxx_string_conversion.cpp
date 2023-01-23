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


template <typename T>
class StdString_test : public Test
{
  public:
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
    string<STRINGCAP> fuu = into<MyString>(testString);
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
    string<STRINGCAP> fuu = into<MyString>(testString);
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
    string<STRINGCAP> fuu = into<MyString>(testString);
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
    string<STRINGCAP> fuu = into<MyString>(testString);
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
} // namespace
