// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2022 by NXP. All rights reserved.
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

#include "iceoryx_dust/cxx/convert.hpp"
#include "test.hpp"


#include <cstdint>
namespace
{
using namespace ::testing;

using NumberType = iox::cxx::convert::NumberType;

class convert_test : public Test
{
  public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};


TEST_F(convert_test, toString_uint8_t)
{
    ::testing::Test::RecordProperty("TEST_ID", "24321fe1-52e2-48e1-b31a-436473b3e5f0");
    constexpr uint8_t DATA = 131U;
    EXPECT_THAT(iox::cxx::convert::toString(DATA), Eq("131"));
}

TEST_F(convert_test, toString_int8_t)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ec95300-04e9-4282-a7a4-92a7d8717343");
    constexpr int8_t DATA = 31U;
    EXPECT_THAT(iox::cxx::convert::toString(DATA), Eq("31"));
}

TEST_F(convert_test, toString_Integer)
{
    ::testing::Test::RecordProperty("TEST_ID", "c426bfd9-3cfa-4986-90ed-d55147434a3e");
    constexpr int DATA = 33331;
    EXPECT_THAT(iox::cxx::convert::toString(DATA), Eq("33331"));
}

TEST_F(convert_test, toString_Float)
{
    ::testing::Test::RecordProperty("TEST_ID", "e00f7b9c-325c-4eb1-885c-83f8d5fa3f72");
    constexpr float DATA = 333.1F;
    EXPECT_THAT(iox::cxx::convert::toString(DATA), Eq("333.1"));
}

TEST_F(convert_test, toString_LongLongUnsignedInt)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d70c7e8-801e-4492-9f01-036c62b4ce54");
    constexpr long long unsigned DATA = 123LLU;
    EXPECT_THAT(iox::cxx::convert::toString(DATA), Eq("123"));
}

TEST_F(convert_test, toString_Char)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb223438-73e9-409a-b644-088bb6509d9c");
    constexpr char DATA = 'x';
    EXPECT_THAT(iox::cxx::convert::toString(DATA), Eq("x"));
}

TEST_F(convert_test, toString_String)
{
    ::testing::Test::RecordProperty("TEST_ID", "43eb7090-619c-42a5-bad9-9f452e81228b");
    const std::string DATA = "hello";
    EXPECT_THAT(iox::cxx::convert::toString(DATA), Eq("hello"));
}

TEST_F(convert_test, toString_StringConvertableClass)
{
    ::testing::Test::RecordProperty("TEST_ID", "39601439-ec94-49d0-ac30-168dd0598bdc");
    struct A
    {
        // we want to test the implicit conversion
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        operator std::string() const
        {
            return "fuu";
        }
    };

    EXPECT_THAT(iox::cxx::convert::toString(A()), Eq("fuu"));
}

TEST_F(convert_test, FromString_String)
{
    ::testing::Test::RecordProperty("TEST_ID", "22463da5-0fcb-4aa2-a7e5-68b863278a81");
    std::string source = "hello";
    std::string destination;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(source, Eq(destination));
}

TEST_F(convert_test, fromString_Char_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "a15825c9-536a-4671-a502-6973490022e7");
    std::string source = "h";
    char destination = '\0';
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(source[0], Eq(destination));
}

TEST_F(convert_test, fromString_Char_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "656e87ad-6fdb-42d7-bf49-23f81a4f5a31");
    std::string source = "hasd";
    char destination = '\0';
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, stringIsNumber_IsINTEGER)
{
    ::testing::Test::RecordProperty("TEST_ID", "d3961dde-2e76-4e8d-b448-d1ad0f734ab1");
    EXPECT_THAT(iox::cxx::convert::stringIsNumber("123921301", NumberType::INTEGER), Eq(true));
}

TEST_F(convert_test, stringIsNumber_IsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "1cdf0515-e021-4df1-ac08-b0f2b366097c");
    EXPECT_THAT(iox::cxx::convert::stringIsNumber("", NumberType::INTEGER), Eq(false));
}

TEST_F(convert_test, stringIsNumber_IsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "f36cbb74-35fc-4a2b-b6b4-8bea810dcf3f");
    EXPECT_THAT(iox::cxx::convert::stringIsNumber("0", NumberType::INTEGER), Eq(true));
}

TEST_F(convert_test, stringIsNumber_INTEGERWithSign)
{
    ::testing::Test::RecordProperty("TEST_ID", "af9e7aeb-b6c5-440d-a706-e869dd8454f3");
    EXPECT_THAT(iox::cxx::convert::stringIsNumber("-521", NumberType::INTEGER), Eq(true));
}

TEST_F(convert_test, stringIsNumber_INTEGERWithSignPlacedWrongly)
{
    ::testing::Test::RecordProperty("TEST_ID", "32cc349b-baaf-4128-97c9-da3687097f4b");
    EXPECT_THAT(iox::cxx::convert::stringIsNumber("2-3", NumberType::UNSIGNED_INTEGER), Eq(false));
}

TEST_F(convert_test, stringIsNumber_SimpleFLOAT)
{
    ::testing::Test::RecordProperty("TEST_ID", "34407449-1eed-440d-be9e-f8d25ddd700e");
    EXPECT_THAT(iox::cxx::convert::stringIsNumber("123.456", NumberType::FLOAT), Eq(true));
}

TEST_F(convert_test, stringIsNumber_MultiDotFLOAT)
{
    ::testing::Test::RecordProperty("TEST_ID", "d6ae9709-0fe5-472f-a786-90a4cc11db6a");
    EXPECT_THAT(iox::cxx::convert::stringIsNumber("11.1.123", NumberType::FLOAT), Eq(false));
}

TEST_F(convert_test, stringIsNumber_FLOATWithSign)
{
    ::testing::Test::RecordProperty("TEST_ID", "c840aa92-aab7-4d01-a6ce-d722bd1296dd");
    EXPECT_THAT(iox::cxx::convert::stringIsNumber("+123.321", NumberType::FLOAT), Eq(true));
}

TEST_F(convert_test, stringIsNumber_NumberWithLetters)
{
    ::testing::Test::RecordProperty("TEST_ID", "2039f8e9-70f7-451c-9415-287269e97f83");
    EXPECT_THAT(iox::cxx::convert::stringIsNumber("+123a.123", NumberType::FLOAT), Eq(false));
}

TEST_F(convert_test, fromString_FLOAT_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "d6255c3e-369e-43a0-a1ab-03f7b13d03c2");
    std::string source = "123.01";
    float destination = 0.0F;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_FLOAT_EQ(destination, 123.01F);
}

TEST_F(convert_test, fromString_FLOAT_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "e2b94d50-664c-4f9e-be4f-99212c6fa165");
    std::string source = "hasd";
    float destination = 0.0F;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_Double_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "95ba379e-120e-4b80-a829-33fe54f1bfed");
    std::string source = "123.04";
    double destination = 0.0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(destination, Eq(123.04));
}

TEST_F(convert_test, fromString_Double_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "f4ace11b-a056-47b1-b6c5-6fb2c58e1a06");
    std::string source = "hasd";
    double destination = 0.0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_LongDouble_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "2864fbae-ef1c-48ab-97f2-745baadc4dc5");
    std::string source = "121.01";
    long double destination = 0.0;
    constexpr long double VERIFY = 121.01;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(static_cast<double>(destination), DoubleEq(static_cast<double>(VERIFY)));
}

TEST_F(convert_test, fromString_LongDouble_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "519f2ac5-8836-419e-8034-377230a88a09");
    std::string source = "hasd";
    double destination = 0.0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_UNSIGNED_Int_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "1edb8d5f-c42d-4d02-bc31-477f48898bbb");
    std::string source = "100";
    unsigned int destination = 0.0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(destination, Eq(100U));
}

TEST_F(convert_test, fromString_UNSIGNED_Int_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "6ce6de82-a6c0-4562-9c5c-663b93d768b3");
    std::string source = "-331";
    unsigned int destination = 0U;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_UNSIGNED_LongInt_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "054b08b2-54e1-4191-91b6-e6bec415612f");
    std::string source = "999";
    uint64_t destination = 0U;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(destination, Eq(999LU));
}

TEST_F(convert_test, fromString_UNSIGNED_LongInt_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "4b215747-90b2-4ca2-97ee-517c07597b1b");
    std::string source = "-a123";
    uint64_t destination = 0U;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_Int_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "9318ee60-f2e0-445a-b32d-c718cf918b18");
    std::string source = "3331";
    int destination = 0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(destination, Eq(3331));
}

TEST_F(convert_test, fromString_Int_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8e698a9-054d-4441-b196-bcd58a72b1d9");
    std::string source = "-+321";
    int destination = 0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_ShortInt_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "e804f821-157d-4c52-81a7-75fce5a43805");
    std::string source = "12345";
    short destination = 0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(destination, Eq(12345));
}

TEST_F(convert_test, fromString_ShortInt_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "1150066b-cb42-4055-9927-2f20fb40bc87");
    std::string source = "-+123321";
    short destination = 0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_Bool_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "893723fc-dfb8-46a4-b446-badaf8bad25a");
    std::string source = "1";
    bool destination = false;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(destination, Eq(true));
}

TEST_F(convert_test, fromString_Bool_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "1c937da6-29ea-49cf-a7d0-4c46f564c16e");
    std::string source = "-+222";
    bool destination = false;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_UShortInt_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "99d22d80-3860-47fa-9f98-f11ff9629815");
    std::string source = "333";
    unsigned short destination = 0U;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(destination, Eq(333));
}

TEST_F(convert_test, fromString_UShortInt_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "6ab6ded6-dff3-401a-8a7f-98326da7cca6");
    std::string source = "-+111";
    unsigned short destination = 0U;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_LongInt_Success)
{
    ::testing::Test::RecordProperty("TEST_ID", "37133256-ae79-45c7-8c86-56bd33fa7bd8");
    std::string source = "-1123";
    int64_t destination = 0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    EXPECT_THAT(destination, Eq(-1123L));
}

TEST_F(convert_test, fromString_LongInt_Fail)
{
    ::testing::Test::RecordProperty("TEST_ID", "0e368bf3-cb16-4829-a4cc-dc56e0bde958");
    std::string source = "-a121";
    int64_t destination = 0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_MinMaxShort)
{
    ::testing::Test::RecordProperty("TEST_ID", "98e33efd-ba39-4b88-8307-358be30e4e73");
    std::string source = "32767";
    int16_t destination = 0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "32768";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
    source = "-32768";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "-32769";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_MinMaxUNSIGNED_Short)
{
    ::testing::Test::RecordProperty("TEST_ID", "f9196939-ae5d-4c27-85bf-b3b084343261");
    std::string source = "65535";
    uint16_t destination = 0U;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "65536";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
    source = "0";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "-1";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_MinMaxInt)
{
    ::testing::Test::RecordProperty("TEST_ID", "abf0fda5-044e-4f1b-bb1e-31b701578a3d");
    std::string source = "2147483647";
    int32_t destination = 0;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "2147483648";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
    source = "-2147483648";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "-2147483649";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_MinMaxUNSIGNED_Int)
{
    ::testing::Test::RecordProperty("TEST_ID", "c2a832ef-3e86-4303-a98c-63c7b11ea789");
    std::string source = "4294967295";
    uint32_t destination = 0U;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "4294967296";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
    source = "0";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "-1";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

TEST_F(convert_test, fromString_cxxString)
{
    ::testing::Test::RecordProperty("TEST_ID", "dbf015bb-5f51-47e1-9d0e-0525f65e7803");
    std::string source = "hello";
    iox::string<8> destination;
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "12345678";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(true));
    source = "123456789";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
    source = "this_is_a_very_long_string";
    EXPECT_THAT(iox::cxx::convert::fromString(source.c_str(), destination), Eq(false));
}

} // namespace
