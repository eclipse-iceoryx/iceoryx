// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/subscriber_options.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

TEST(SubscriberOptions_test, SerializationRoundTripIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "c8e9480d-15be-43d7-9218-fb6a2ce9b91e");
    iox::popo::SubscriberOptions defaultOptions;
    iox::popo::SubscriberOptions testOptions;

    testOptions.queueCapacity = 73;
    testOptions.historyRequest = 42;
    testOptions.nodeName = "hypnotoad";
    testOptions.subscribeOnCreate = false;
    testOptions.queueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    testOptions.requiresPublisherHistorySupport = true;

    iox::popo::SubscriberOptions::deserialize(testOptions.serialize())
        .and_then([&](auto& roundTripOptions) {
            EXPECT_THAT(roundTripOptions.queueCapacity, Ne(defaultOptions.queueCapacity));
            EXPECT_THAT(roundTripOptions.queueCapacity, Eq(testOptions.queueCapacity));

            EXPECT_THAT(roundTripOptions.historyRequest, Ne(defaultOptions.historyRequest));
            EXPECT_THAT(roundTripOptions.historyRequest, Eq(testOptions.historyRequest));

            EXPECT_THAT(roundTripOptions.nodeName, Ne(defaultOptions.nodeName));
            EXPECT_THAT(roundTripOptions.nodeName, Eq(testOptions.nodeName));

            EXPECT_THAT(roundTripOptions.subscribeOnCreate, Ne(defaultOptions.subscribeOnCreate));
            EXPECT_THAT(roundTripOptions.subscribeOnCreate, Eq(testOptions.subscribeOnCreate));

            EXPECT_THAT(roundTripOptions.queueFullPolicy, Ne(defaultOptions.queueFullPolicy));
            EXPECT_THAT(roundTripOptions.queueFullPolicy, Eq(testOptions.queueFullPolicy));
            EXPECT_THAT(roundTripOptions.requiresPublisherHistorySupport,
                        Eq(testOptions.requiresPublisherHistorySupport));
        })
        .or_else([&](auto&) { GTEST_FAIL() << "Serialization/Deserialization of SubscriberOptions failed!"; });
}

TEST(SubscriberOptions_test, DeserializingBogusDataFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "6b4b77cc-09ce-4f71-b2b5-371be27f863a");
    const auto bogusSerialization = iox::Serialization::create("hypnotoad", "brain slug", "rock star");
    iox::popo::SubscriberOptions::deserialize(bogusSerialization)
        .and_then([&](auto&) { GTEST_FAIL() << "Deserialization is expected to fail!"; })
        .or_else([&](auto&) { GTEST_SUCCEED(); });
}

TEST(SubscriberOptions_test, DeserializingInvalidQueueFullPolicyFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "c41116d5-315d-4921-a322-03a6a26df4e0");
    constexpr uint64_t QUEUE_CAPACITY{73U};
    constexpr uint64_t HISTORY_REQUEST{42U};
    const iox::NodeName_t NODE_NAME{"harr-harr"};
    constexpr bool SUBSCRIBE_ON_CREATE{true};
    constexpr std::underlying_type_t<iox::popo::QueueFullPolicy> QUEUE_FULL_POLICY{111};

    const auto serialized =
        iox::Serialization::create(QUEUE_CAPACITY, HISTORY_REQUEST, NODE_NAME, SUBSCRIBE_ON_CREATE, QUEUE_FULL_POLICY);
    iox::popo::SubscriberOptions::deserialize(serialized)
        .and_then([&](auto&) { GTEST_FAIL() << "Deserialization is expected to fail!"; })
        .or_else([&](auto&) { GTEST_SUCCEED(); });
}

} // namespace
