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

#include "iceoryx_posh/popo/client_options.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

TEST(ClientOptions_test, SerializationRoundTripIsSuccessful)
{
    iox::popo::ClientOptions defaultOptions;
    iox::popo::ClientOptions testOptions;

    testOptions.responseQueueCapacity = 42;
    testOptions.nodeName = "hypnotoad";
    testOptions.connectOnCreate = false;
    testOptions.responseQueueFullPolicy = iox::popo::QueueFullPolicy2::BLOCK_PRODUCER;
    testOptions.serverTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    auto roundTripOptions = iox::popo::ClientOptions::deserialize(testOptions.serialize());

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
}

} // namespace
