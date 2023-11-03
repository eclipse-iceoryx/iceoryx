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

#include "iceoryx_posh/popo/client_options.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::popo;

TEST(ClientOptions_test, SerializationRoundTripIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "1d803ab0-a657-4a84-b261-ec289a7353e6");
    iox::popo::ClientOptions defaultOptions;
    iox::popo::ClientOptions testOptions;

    testOptions.responseQueueCapacity = 42;
    testOptions.nodeName = "hypnotoad";
    testOptions.connectOnCreate = false;
    testOptions.responseQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    testOptions.serverTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    iox::popo::ClientOptions::deserialize(testOptions.serialize())
        .and_then([&](auto& roundTripOptions) {
            EXPECT_THAT(roundTripOptions.responseQueueCapacity, Ne(defaultOptions.responseQueueCapacity));
            EXPECT_THAT(roundTripOptions.responseQueueCapacity, Eq(testOptions.responseQueueCapacity));

            EXPECT_THAT(roundTripOptions.nodeName, Ne(defaultOptions.nodeName));
            EXPECT_THAT(roundTripOptions.nodeName, Eq(testOptions.nodeName));

            EXPECT_THAT(roundTripOptions.connectOnCreate, Ne(defaultOptions.connectOnCreate));
            EXPECT_THAT(roundTripOptions.connectOnCreate, Eq(testOptions.connectOnCreate));

            EXPECT_THAT(roundTripOptions.responseQueueFullPolicy, Ne(defaultOptions.responseQueueFullPolicy));
            EXPECT_THAT(roundTripOptions.responseQueueFullPolicy, Eq(testOptions.responseQueueFullPolicy));

            EXPECT_THAT(roundTripOptions.serverTooSlowPolicy, Ne(defaultOptions.serverTooSlowPolicy));
            EXPECT_THAT(roundTripOptions.serverTooSlowPolicy, Eq(testOptions.serverTooSlowPolicy));
        })
        .or_else([&](auto&) {
            constexpr bool DESERIALZATION_ERROR_OCCURED{true};
            EXPECT_FALSE(DESERIALZATION_ERROR_OCCURED);
        });
}

TEST(ClientOptions_test, DeserializingBogusDataFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "eb7341fd-f216-4422-8065-cbbadefd567b");
    const auto bogusSerialization = iox::Serialization::create("hypnotoad", "brain slug", "rock star");
    iox::popo::ClientOptions::deserialize(bogusSerialization)
        .and_then([&](auto&) {
            constexpr bool DESERIALZATION_SUCCESSFUL{true};
            EXPECT_FALSE(DESERIALZATION_SUCCESSFUL);
        })
        .or_else([&](auto&) {
            constexpr bool DESERIALZATION_ERROR_OCCURED{true};
            EXPECT_TRUE(DESERIALZATION_ERROR_OCCURED);
        });
}

using QueueFullPolicyUT = std::underlying_type_t<iox::popo::QueueFullPolicy>;
using ConsumerTooSlowPolicyUT = std::underlying_type_t<iox::popo::ConsumerTooSlowPolicy>;
iox::Serialization enumSerialization(QueueFullPolicyUT responseQueueFullPolicy,
                                     ConsumerTooSlowPolicyUT serverTooSlowPolicy)
{
    constexpr uint64_t RESPONSE_QUEUE_CAPACITY{42U};
    const iox::NodeName_t NODE_NAME{"harr-harr"};
    constexpr bool CONNECT_ON_CREATE{true};

    return iox::Serialization::create(
        RESPONSE_QUEUE_CAPACITY, NODE_NAME, CONNECT_ON_CREATE, responseQueueFullPolicy, serverTooSlowPolicy);
}

TEST(ClientOptions_test, DeserializingValidResponseQueueFullAndServerTooSlowPolicyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "877ad373-eb51-4613-9b68-b93baa4c6eae");
    constexpr QueueFullPolicyUT RESPONSE_QUEUE_FULL_POLICY{
        static_cast<QueueFullPolicyUT>(iox::popo::QueueFullPolicy::BLOCK_PRODUCER)};
    constexpr ConsumerTooSlowPolicyUT SERVER_TOO_SLOW_POLICY{
        static_cast<ConsumerTooSlowPolicyUT>(iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER)};

    const auto serialized = enumSerialization(RESPONSE_QUEUE_FULL_POLICY, SERVER_TOO_SLOW_POLICY);
    iox::popo::ClientOptions::deserialize(serialized)
        .and_then([&](auto&) {
            constexpr bool DESERIALZATION_SUCCESSFUL{true};
            EXPECT_TRUE(DESERIALZATION_SUCCESSFUL);
        })
        .or_else([&](auto&) {
            constexpr bool DESERIALZATION_ERROR_OCCURED{true};
            EXPECT_FALSE(DESERIALZATION_ERROR_OCCURED);
        });
}

TEST(ClientOptions_test, DeserializingInvalidResponseQueueFullPolicyFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5495324-67d0-4f0c-b979-f93d4b68adc5");
    constexpr QueueFullPolicyUT RESPONSE_QUEUE_FULL_POLICY{111};
    constexpr ConsumerTooSlowPolicyUT SERVER_TOO_SLOW_POLICY{
        static_cast<ConsumerTooSlowPolicyUT>(iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA)};

    const auto serialized = enumSerialization(RESPONSE_QUEUE_FULL_POLICY, SERVER_TOO_SLOW_POLICY);
    iox::popo::ClientOptions::deserialize(serialized)
        .and_then([&](auto&) {
            constexpr bool DESERIALZATION_SUCCESSFUL{true};
            EXPECT_FALSE(DESERIALZATION_SUCCESSFUL);
        })
        .or_else([&](auto&) {
            constexpr bool DESERIALZATION_ERROR_OCCURED{true};
            EXPECT_TRUE(DESERIALZATION_ERROR_OCCURED);
        });
}

TEST(ClientOptions_test, DeserializingInvalidServerTooSlowPolicyFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6485377c-9a75-4aba-8045-8b129aa1c529");
    constexpr QueueFullPolicyUT RESPONSE_QUEUE_FULL_POLICY{
        static_cast<QueueFullPolicyUT>(iox::popo::QueueFullPolicy::BLOCK_PRODUCER)};
    constexpr ConsumerTooSlowPolicyUT SERVER_TOO_SLOW_POLICY{111};

    const auto serialized = enumSerialization(RESPONSE_QUEUE_FULL_POLICY, SERVER_TOO_SLOW_POLICY);
    iox::popo::ClientOptions::deserialize(serialized)
        .and_then([&](auto&) {
            constexpr bool DESERIALZATION_SUCCESSFUL{true};
            EXPECT_FALSE(DESERIALZATION_SUCCESSFUL);
        })
        .or_else([&](auto&) {
            constexpr bool DESERIALZATION_ERROR_OCCURED{true};
            EXPECT_TRUE(DESERIALZATION_ERROR_OCCURED);
        });
}

TEST(ClientOptions_test, ComparisonOperatorReturnsTrueWhenEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba0554b8-6b25-45c2-8a4c-66b6663de586");
    ClientOptions options1;
    ClientOptions options2;

    EXPECT_TRUE(options1 == options1);
    EXPECT_TRUE(options1 == options2);
    EXPECT_TRUE(options2 == options1);
}

TEST(ClientOptions_test, ComparisonOperatorReturnsFalseWhenRessponseQueueCapacityDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "39ffdd52-e068-49f8-aca2-9638162f6b7d");
    ClientOptions options1;
    options1.responseQueueCapacity = 42;
    ClientOptions options2;
    options2.responseQueueCapacity = 73;

    EXPECT_FALSE(options1 == options2);
    EXPECT_FALSE(options2 == options1);
}

TEST(ClientOptions_test, ComparisonOperatorReturnsFalseWhenNodeNameDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "e5b5bdeb-e173-4dcd-b9b3-080466e70c4d");
    ClientOptions options1;
    options1.nodeName = "kirk";
    ClientOptions options2;
    options2.nodeName = "picard";

    EXPECT_FALSE(options1 == options2);
    EXPECT_FALSE(options2 == options1);
}

TEST(ClientOptions_test, ComparisonOperatorReturnsFalseWhenConnectOnCreateDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "829d8f19-ebc6-401e-a10d-05dfe5062380");
    ClientOptions options1;
    options1.connectOnCreate = false;
    ClientOptions options2;
    options2.connectOnCreate = true;

    EXPECT_FALSE(options1 == options2);
    EXPECT_FALSE(options2 == options1);
}

TEST(ClientOptions_test, ComparisonOperatorReturnsFalseResponseQueueFullPolicyDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ab2afdd-f75f-4340-8ac0-2288fec030fa");
    ClientOptions options1;
    options1.responseQueueFullPolicy = QueueFullPolicy::BLOCK_PRODUCER;
    ClientOptions options2;
    options2.responseQueueFullPolicy = QueueFullPolicy::DISCARD_OLDEST_DATA;

    EXPECT_FALSE(options1 == options2);
    EXPECT_FALSE(options2 == options1);
}

TEST(ClientOptions_test, ComparisonOperatorReturnsFalseServerTooSlowPolicyDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "05f8aeff-8b64-42be-9640-b8bdf129e48c");
    ClientOptions options1;
    options1.serverTooSlowPolicy = ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    ClientOptions options2;
    options2.serverTooSlowPolicy = ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;

    EXPECT_FALSE(options1 == options2);
    EXPECT_FALSE(options2 == options1);
}

} // namespace
