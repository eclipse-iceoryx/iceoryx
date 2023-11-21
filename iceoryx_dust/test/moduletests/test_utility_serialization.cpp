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

#include "iox/detail/serialization.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

class Serialization_test : public Test
{
  public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_F(Serialization_test, CreateSingleEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "501d7767-f522-4e3c-a367-9c659e109b1b");
    auto serial = iox::Serialization::create("hello world");
    EXPECT_THAT(serial.toString(), Eq("11:hello world"));
}

TEST_F(Serialization_test, CreateMultiEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "5baf4079-20f4-4ec2-a5f9-38f61460b61b");
    const auto serial = iox::Serialization::create("hello world", 12345);
    EXPECT_THAT(serial.toString(), StrEq("11:hello world5:12345"));
}

TEST_F(Serialization_test, ExtractSingleEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "47b75a1a-f133-453b-adad-c82e5ea77565");
    constexpr uint64_t NUMBER{12345};
    auto serial = iox::Serialization::create(NUMBER);
    uint64_t i{0};
    EXPECT_THAT(serial.extract(i), Eq(true));
    EXPECT_THAT(i, Eq(NUMBER));
}

TEST_F(Serialization_test, ExtractSingleEntryWrongType)
{
    ::testing::Test::RecordProperty("TEST_ID", "4bba7ed7-0485-48fc-b979-76a7b8e97b2a");
    auto serial = iox::Serialization::create("asd");
    uint64_t i{0};
    EXPECT_THAT(serial.extract(i), Eq(false));
}

TEST_F(Serialization_test, ExtractMultiEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf0be6f1-e986-499c-b96e-339cdc9cb534");
    constexpr uint64_t I{1234};
    constexpr char C{'c'};
    constexpr const char* S{"aasd"};
    const auto serial = iox::Serialization::create(I, C, S);
    uint64_t i{0};
    char c{'a'};
    std::string s;
    EXPECT_THAT(serial.extract(i, c, s), Eq(true));
    EXPECT_THAT(i, Eq(I));
    EXPECT_THAT(c, Eq(C));
    EXPECT_THAT(s, StrEq(S));
}

TEST_F(Serialization_test, ExtractMultiEntryWrongType)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e0cdb0f-7a6f-4fce-a04d-bb665f5692e1");
    constexpr uint64_t I{12345};
    constexpr char C{'x'};
    constexpr const char* S{"asdasd"};
    auto serial = iox::Serialization::create(I, C, S);
    uint64_t i{0};
    char c1{'a'};
    char c2{'a'};
    EXPECT_THAT(serial.extract(i, c1, c2), Eq(false));
}

TEST_F(Serialization_test, GetNthSingleEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "39b3a9d5-e3e1-4131-8f1a-5bbeaf199fcb");
    constexpr uint64_t I{123456};
    auto serial = iox::Serialization::create(I);
    uint64_t i{0};
    EXPECT_THAT(serial.getNth(0, i), Eq(true));
    EXPECT_THAT(i, Eq(I));
}

TEST_F(Serialization_test, GetNthSingleEntryWrongType)
{
    ::testing::Test::RecordProperty("TEST_ID", "83d32bc1-d732-483b-a8e8-ca71b9ca9c9c");
    auto serial = iox::Serialization::create("a1234a5");
    uint64_t i{0};
    EXPECT_THAT(serial.getNth(0, i), Eq(false));
}

TEST_F(Serialization_test, GetNthMultiEntry)
{
    ::testing::Test::RecordProperty("TEST_ID", "00fb34ec-2135-4824-bd8f-a681ac21d215");
    constexpr uint64_t V1{12345};
    constexpr const char* V2{"asdasd"};
    constexpr char V3{'x'};
    constexpr int64_t V4{-123};
    const auto serial = iox::Serialization::create(V1, V2, V3, V4);
    uint64_t v1{0};
    std::string v2;
    char v3{'a'};
    int64_t v4{0};
    EXPECT_THAT(serial.getNth(0, v1), Eq(true));
    EXPECT_THAT(serial.getNth(1, v2), Eq(true));
    EXPECT_THAT(serial.getNth(2, v3), Eq(true));
    EXPECT_THAT(serial.getNth(3, v4), Eq(true));

    EXPECT_THAT(v1, Eq(V1));
    EXPECT_THAT(v2, StrEq(V2));
    EXPECT_THAT(v3, Eq(V3));
    EXPECT_THAT(v4, Eq(V4));
}

TEST_F(Serialization_test, ExtractFromGivenSerialization)
{
    ::testing::Test::RecordProperty("TEST_ID", "38d6ffe5-6ca2-4dd6-9b2d-0002eb4e312f");
    iox::Serialization serial("6:hello!4:1234");
    std::string v1;
    uint64_t v2{0};
    EXPECT_THAT(serial.extract(v1, v2), Eq(true));
    EXPECT_THAT(v1, StrEq("hello!"));
    EXPECT_THAT(v2, Eq(1234));
}

TEST_F(Serialization_test, SerializeSerializableClass)
{
    ::testing::Test::RecordProperty("TEST_ID", "f9a4d22a-a4ca-4451-8f35-27ffff65cd2d");
    struct A
    {
        A() = default;
        explicit A(const iox::Serialization& serialized [[maybe_unused]])
        {
        }
        // NOLINTNEXTLINE(hicpp-explicit-conversions) required by the Serialization API
        operator iox::Serialization() const
        {
            return iox::Serialization("5:asdgg");
        }
    };

    A obj;
    auto serial = iox::Serialization::create(obj, "asd");
    EXPECT_THAT(serial.toString(), Eq("7:5:asdgg3:asd"));
}
} // namespace
