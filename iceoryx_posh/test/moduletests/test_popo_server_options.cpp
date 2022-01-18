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

TEST(ServerOptions_test, SerializationRoundTripIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "888f49c2-0b70-4033-a13a-175dbc1b8e38");
    iox::popo::ServerOptions defaultOptions;
    iox::popo::ServerOptions testOptions;

    testOptions.requestQueueCapacity = 42;
    testOptions.nodeName = "hypnotoad";
    testOptions.offerOnCreate = false;
    testOptions.clientTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    iox::popo::ServerOptions::deserialize(testOptions.serialize())
        .and_then([&](auto& roundTripOptions) {
            EXPECT_THAT(roundTripOptions.requestQueueCapacity, Ne(defaultOptions.requestQueueCapacity));
            EXPECT_THAT(roundTripOptions.requestQueueCapacity, Eq(testOptions.requestQueueCapacity));

            EXPECT_THAT(roundTripOptions.nodeName, Ne(defaultOptions.nodeName));
            EXPECT_THAT(roundTripOptions.nodeName, Eq(testOptions.nodeName));

            EXPECT_THAT(roundTripOptions.offerOnCreate, Ne(defaultOptions.offerOnCreate));
            EXPECT_THAT(roundTripOptions.offerOnCreate, Eq(testOptions.offerOnCreate));

            EXPECT_THAT(roundTripOptions.clientTooSlowPolicy, Ne(defaultOptions.clientTooSlowPolicy));
            EXPECT_THAT(roundTripOptions.clientTooSlowPolicy, Eq(testOptions.clientTooSlowPolicy));
        })
        .or_else([&](auto&) { FAIL() << "Serialization/Deserialization of ServerOptions failed!"; });
}

TEST(ServerOptions_test, DeserializingBogusDataFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "ebc97c23-87df-484c-8c3c-1b76f1351997");
    const auto bogusSerialization = iox::cxx::Serialization::create("hypnotoad", "brain slug", "rock star");
    iox::popo::ServerOptions::deserialize(bogusSerialization)
        .and_then([&](auto&) { FAIL() << "Deserialization is expected to fail!"; })
        .or_else([&](auto&) { SUCCEED(); });
}

using ConsumerTooSlowPolicyUT = std::underlying_type_t<iox::popo::ConsumerTooSlowPolicy>;
iox::cxx::Serialization enumSerialization(ConsumerTooSlowPolicyUT clientTooSlowPolicy)
{
    constexpr uint64_t REQUEST_QUEUE_CAPACITY{42U};
    const iox::NodeName_t NODE_NAME{"harr-harr"};
    constexpr bool OFFER_ON_CREATE{true};

    return iox::cxx::Serialization::create(REQUEST_QUEUE_CAPACITY, NODE_NAME, OFFER_ON_CREATE, clientTooSlowPolicy);
}

TEST(ServerOptions_test, DeserializingValidClientTooSlowPolicyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "95cd1efc-63c8-4eee-9f4e-ed105e653d71");
    constexpr ConsumerTooSlowPolicyUT CLIENT_TOO_SLOW_POLICY{
        static_cast<ConsumerTooSlowPolicyUT>(iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER)};

    const auto serialized = enumSerialization(CLIENT_TOO_SLOW_POLICY);
    iox::popo::ServerOptions::deserialize(serialized).and_then([&](auto&) { SUCCEED(); }).or_else([&](auto&) {
        FAIL() << "Serialization/Deserialization of ServerOptions failed!";
    });
}

TEST(ServerOptions_test, DeserializingInvalidClientTooSlowPolicyFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "35b85d5a-7e59-4f0c-8afc-38f1eec914b8");
    constexpr ConsumerTooSlowPolicyUT CLIENT_TOO_SLOW_POLICY{111};

    const auto serialized = enumSerialization(CLIENT_TOO_SLOW_POLICY);
    iox::popo::ServerOptions::deserialize(serialized)
        .and_then([&](auto&) { FAIL() << "Deserialization is expected to fail!"; })
        .or_else([&](auto&) { SUCCEED(); });
}

} // namespace
