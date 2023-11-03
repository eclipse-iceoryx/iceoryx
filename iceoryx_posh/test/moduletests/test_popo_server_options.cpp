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

#include "iceoryx_posh/popo/server_options.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::popo;

TEST(ServerOptions_test, SerializationRoundTripIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "888f49c2-0b70-4033-a13a-175dbc1b8e38");
    iox::popo::ServerOptions defaultOptions;
    iox::popo::ServerOptions testOptions;

    testOptions.requestQueueCapacity = 42;
    testOptions.nodeName = "hypnotoad";
    testOptions.offerOnCreate = false;
    testOptions.requestQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    testOptions.clientTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    iox::popo::ServerOptions::deserialize(testOptions.serialize())
        .and_then([&](auto& roundTripOptions) {
            EXPECT_THAT(roundTripOptions.requestQueueCapacity, Ne(defaultOptions.requestQueueCapacity));
            EXPECT_THAT(roundTripOptions.requestQueueCapacity, Eq(testOptions.requestQueueCapacity));

            EXPECT_THAT(roundTripOptions.nodeName, Ne(defaultOptions.nodeName));
            EXPECT_THAT(roundTripOptions.nodeName, Eq(testOptions.nodeName));

            EXPECT_THAT(roundTripOptions.offerOnCreate, Ne(defaultOptions.offerOnCreate));
            EXPECT_THAT(roundTripOptions.offerOnCreate, Eq(testOptions.offerOnCreate));

            EXPECT_THAT(roundTripOptions.requestQueueFullPolicy, Ne(defaultOptions.requestQueueFullPolicy));
            EXPECT_THAT(roundTripOptions.requestQueueFullPolicy, Eq(testOptions.requestQueueFullPolicy));

            EXPECT_THAT(roundTripOptions.clientTooSlowPolicy, Ne(defaultOptions.clientTooSlowPolicy));
            EXPECT_THAT(roundTripOptions.clientTooSlowPolicy, Eq(testOptions.clientTooSlowPolicy));
        })
        .or_else([&](auto&) { GTEST_FAIL() << "Serialization/Deserialization of ServerOptions failed!"; });
}

TEST(ServerOptions_test, DeserializingBogusDataFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ebc97c23-87df-484c-8c3c-1b76f1351997");
    const auto bogusSerialization = iox::Serialization::create("hypnotoad", "brain slug", "rock star");
    iox::popo::ServerOptions::deserialize(bogusSerialization)
        .and_then([&](auto&) { GTEST_FAIL() << "Deserialization is expected to fail!"; })
        .or_else([&](auto&) { GTEST_SUCCEED(); });
}

using QueueFullPolicyUT = std::underlying_type_t<iox::popo::QueueFullPolicy>;
using ConsumerTooSlowPolicyUT = std::underlying_type_t<iox::popo::ConsumerTooSlowPolicy>;
iox::Serialization enumSerialization(QueueFullPolicyUT requsetQueueFullPolicy,
                                     ConsumerTooSlowPolicyUT clientTooSlowPolicy)
{
    constexpr uint64_t REQUEST_QUEUE_CAPACITY{42U};
    const iox::NodeName_t NODE_NAME{"harr-harr"};
    constexpr bool OFFER_ON_CREATE{true};

    return iox::Serialization::create(
        REQUEST_QUEUE_CAPACITY, NODE_NAME, OFFER_ON_CREATE, requsetQueueFullPolicy, clientTooSlowPolicy);
}

TEST(ServerOptions_test, DeserializingValidRequestQueueFullPolicyAndClientTooSlowPolicyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "95cd1efc-63c8-4eee-9f4e-ed105e653d71");
    constexpr QueueFullPolicyUT REQUEST_QUEUE_FULL_POLICY{
        static_cast<QueueFullPolicyUT>(iox::popo::QueueFullPolicy::BLOCK_PRODUCER)};
    constexpr ConsumerTooSlowPolicyUT CLIENT_TOO_SLOW_POLICY{
        static_cast<ConsumerTooSlowPolicyUT>(iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER)};

    const auto serialized = enumSerialization(REQUEST_QUEUE_FULL_POLICY, CLIENT_TOO_SLOW_POLICY);
    iox::popo::ServerOptions::deserialize(serialized).and_then([&](auto&) { GTEST_SUCCEED(); }).or_else([&](auto&) {
        GTEST_FAIL() << "Serialization/Deserialization of ServerOptions failed!";
    });
}

TEST(ServerOptions_test, DeserializingInvalidRequestQueueFullPolicyFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "3d392b0a-6140-4b06-a08d-b06ad27f31cd");
    constexpr QueueFullPolicyUT REQUEST_QUEUE_FULL_POLICY{123};
    constexpr ConsumerTooSlowPolicyUT CLIENT_TOO_SLOW_POLICY{
        static_cast<ConsumerTooSlowPolicyUT>(iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA)};

    const auto serialized = enumSerialization(REQUEST_QUEUE_FULL_POLICY, CLIENT_TOO_SLOW_POLICY);
    iox::popo::ServerOptions::deserialize(serialized)
        .and_then([&](auto&) { GTEST_FAIL() << "Deserialization is expected to fail!"; })
        .or_else([&](auto&) { GTEST_SUCCEED(); });
}

TEST(ServerOptions_test, DeserializingInvalidClientTooSlowPolicyFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "35b85d5a-7e59-4f0c-8afc-38f1eec914b8");
    constexpr QueueFullPolicyUT REQUEST_QUEUE_FULL_POLICY{
        static_cast<QueueFullPolicyUT>(iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA)};
    constexpr ConsumerTooSlowPolicyUT CLIENT_TOO_SLOW_POLICY{111};

    const auto serialized = enumSerialization(REQUEST_QUEUE_FULL_POLICY, CLIENT_TOO_SLOW_POLICY);
    iox::popo::ServerOptions::deserialize(serialized)
        .and_then([&](auto&) { GTEST_FAIL() << "Deserialization is expected to fail!"; })
        .or_else([&](auto&) { GTEST_SUCCEED(); });
}

TEST(ServerOptions_test, ComparisonOperatorReturnsTrueWhenEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "98e68269-94d0-41bb-b8a8-5b06ac0b7bc0");
    ServerOptions options1;
    ServerOptions options2;

    EXPECT_TRUE(options1 == options1);
    EXPECT_TRUE(options1 == options2);
    EXPECT_TRUE(options2 == options1);
}

TEST(ServerOptions_test, ComparisonOperatorReturnsFalseWhenRequestQueueCapacityDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "5fede46a-ddfd-426b-a237-25b3088ee011");
    ServerOptions options1;
    options1.requestQueueCapacity = 42;
    ServerOptions options2;
    options2.requestQueueCapacity = 73;

    EXPECT_FALSE(options1 == options2);
    EXPECT_FALSE(options2 == options1);
}

TEST(ServerOptions_test, ComparisonOperatorReturnsFalseWhenNodeNameDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "fed82e4a-5037-4a77-9b28-e0ca8ec7ad5d");
    ServerOptions options1;
    options1.nodeName = "kirk";
    ServerOptions options2;
    options2.nodeName = "picard";

    EXPECT_FALSE(options1 == options2);
    EXPECT_FALSE(options2 == options1);
}

TEST(ServerOptions_test, ComparisonOperatorReturnsFalseWhenOfferOnCreateDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "7831b7c7-72b1-4acf-8a95-fd7ee2348835");
    ServerOptions options1;
    options1.offerOnCreate = false;
    ServerOptions options2;
    options2.offerOnCreate = true;

    EXPECT_FALSE(options1 == options2);
    EXPECT_FALSE(options2 == options1);
}

TEST(ServerOptions_test, ComparisonOperatorReturnsFalseRequestQueueFullPolicyDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "cc97e01c-94f7-41a9-8fac-19db1fd2d20e");
    ServerOptions options1;
    options1.requestQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;
    ServerOptions options2;
    options2.requestQueueFullPolicy = QueueFullPolicy::DISCARD_OLDEST_DATA;

    EXPECT_FALSE(options1 == options2);
    EXPECT_FALSE(options2 == options1);
}

TEST(ServerOptions_test, ComparisonOperatorReturnsFalseClientTooSlowPolicyDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "80c7e7a3-084c-48e1-aa3c-d51688c41682");
    ServerOptions options1;
    options1.clientTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    ServerOptions options2;
    options2.clientTooSlowPolicy = ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;

    EXPECT_FALSE(options1 == options2);
    EXPECT_FALSE(options2 == options1);
}

} // namespace
