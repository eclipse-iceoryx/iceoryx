// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/serialization.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

class Serialization_test : public Test
{
  public:
    void SetUp()
    {
        internal::CaptureStderr();
    }
    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

TEST_F(Serialization_test, CreateSingleEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "501d7767-f522-4e3c-a367-9c659e109b1b");
    auto serial = iox::cxx::Serialization::create("hello world");
    EXPECT_THAT(serial.toString(), Eq("11:hello world"));
}

TEST_F(Serialization_test, CreateMultiEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "5baf4079-20f4-4ec2-a5f9-38f61460b61b");
    auto serial = iox::cxx::Serialization::create("hello world", 12345);
    EXPECT_THAT(static_cast<std::string>(serial), Eq("11:hello world5:12345"));
}

TEST_F(Serialization_test, ExtractSingleEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "47b75a1a-f133-453b-adad-c82e5ea77565");
    auto serial = iox::cxx::Serialization::create(12345);
    int i;
    EXPECT_THAT(serial.extract(i), Eq(true));
    EXPECT_THAT(i, Eq(12345));
}

TEST_F(Serialization_test, ExtractSingleEntryWrongType)
{
    ::testing::Test::RecordProperty("TEST_ID", "4bba7ed7-0485-48fc-b979-76a7b8e97b2a");
    auto serial = iox::cxx::Serialization::create("asd");
    int i;
    EXPECT_THAT(serial.extract(i), Eq(false));
}

TEST_F(Serialization_test, ExtractMultiEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf0be6f1-e986-499c-b96e-339cdc9cb534");
    auto serial = iox::cxx::Serialization::create(12345, 'c', "aasd");
    int i;
    char c;
    std::string s;
    EXPECT_THAT(serial.extract(i, c, s), Eq(true));
    EXPECT_THAT(i, Eq(12345));
    EXPECT_THAT(c, Eq('c'));
    EXPECT_THAT(s, Eq("aasd"));
}

TEST_F(Serialization_test, ExtractMultiEntryWrongType)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e0cdb0f-7a6f-4fce-a04d-bb665f5692e1");
    auto serial = iox::cxx::Serialization::create(12345, 'c', "aasd");
    int i;
    char c;
    char s;
    EXPECT_THAT(serial.extract(i, c, s), Eq(false));
}

TEST_F(Serialization_test, GetNthSingleEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "39b3a9d5-e3e1-4131-8f1a-5bbeaf199fcb");
    auto serial = iox::cxx::Serialization::create(12345);
    int i;
    EXPECT_THAT(serial.getNth(0, i), Eq(true));
    EXPECT_THAT(i, Eq(12345));
}

TEST_F(Serialization_test, GetNthSingleEntryWrongType)
{
    ::testing::Test::RecordProperty("TEST_ID", "83d32bc1-d732-483b-a8e8-ca71b9ca9c9c");
    auto serial = iox::cxx::Serialization::create("a1234a5");
    int i;
    EXPECT_THAT(serial.getNth(0, i), Eq(false));
}

TEST_F(Serialization_test, GetNthMultiEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "00fb34ec-2135-4824-bd8f-a681ac21d215");
    auto serial = iox::cxx::Serialization::create(12345, "asdasd", 'x', -123);
    int v1;
    std::string v2;
    char v3;
    int v4;
    EXPECT_THAT(serial.getNth(0, v1), Eq(true));
    EXPECT_THAT(serial.getNth(1, v2), Eq(true));
    EXPECT_THAT(serial.getNth(2, v3), Eq(true));
    EXPECT_THAT(serial.getNth(3, v4), Eq(true));

    EXPECT_THAT(v1, Eq(12345));
    EXPECT_THAT(v2, Eq("asdasd"));
    EXPECT_THAT(v3, Eq('x'));
    EXPECT_THAT(v4, Eq(-123));
}

TEST_F(Serialization_test, ExtractFromGivenSerialization)
{
    ::testing::Test::RecordProperty("TEST_ID", "38d6ffe5-6ca2-4dd6-9b2d-0002eb4e312f");
    iox::cxx::Serialization serial("6:hello!4:1234");
    std::string v1;
    int v2;
    EXPECT_THAT(serial.extract(v1, v2), Eq(true));
    EXPECT_THAT(v1, Eq("hello!"));
    EXPECT_THAT(v2, Eq(1234));
}

TEST_F(Serialization_test, SerializeSerializableClass)
{
    ::testing::Test::RecordProperty("TEST_ID", "f9a4d22a-a4ca-4451-8f35-27ffff65cd2d");
    struct A
    {
        A()
        {
        }
        A(const iox::cxx::Serialization&)
        {
        }
        operator iox::cxx::Serialization() const
        {
            return iox::cxx::Serialization("5:asdgg");
        }
    };

    A obj;
    auto serial = iox::cxx::Serialization::create(obj, "asd");
    EXPECT_THAT(serial.toString(), Eq("7:5:asdgg3:asd"));
}
} // namespace
