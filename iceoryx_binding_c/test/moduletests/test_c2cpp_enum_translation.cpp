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

#include "iceoryx_binding_c/enums.h"
#include "iceoryx_binding_c/internal/binding_c_error_reporting.hpp"
#include "iceoryx_binding_c/internal/c2cpp_enum_translation.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace ::iox::testing;

template <typename CPP, typename C>
struct EnumMapping
{
    CPP cpp;
    C c;
};

TEST(c2cpp_enum_translation_test, SubscriberState)
{
    ::testing::Test::RecordProperty("TEST_ID", "7f942bb1-be58-4aff-b05c-2e78c4648be3");
    constexpr EnumMapping<iox::popo::SubscriberState, iox_SubscriberState> SUBSCRIBER_STATES[]{
        {iox::popo::SubscriberState::HAS_DATA, SubscriberState_HAS_DATA}};

    for (const auto subscriberState : SUBSCRIBER_STATES)
    {
        switch (subscriberState.cpp)
        {
        case iox::popo::SubscriberState::HAS_DATA:
            EXPECT_EQ(c2cpp::subscriberState(subscriberState.c), subscriberState.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
// explicitly commented out since we are testing undefined behavior here and that we
// return the default value DISCARD_OLDEST_DATA always in the undefined behavior case
// the clang sanitizer detects this successfully and this leads to termination, and with this the test fails
#if !defined(__clang__)
    IOX_EXPECT_FATAL_FAILURE(
        [&] { c2cpp::subscriberState(iox_test_binding_c::maxUnderlyingCEnumValue<iox_SubscriberState>()); },
        iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_STATE_VALUE);
#endif
}

TEST(c2cpp_enum_translation_test, SubscriberEvent)
{
    ::testing::Test::RecordProperty("TEST_ID", "eac05952-7bb1-4265-bd96-1c9c2b5f7327");
    constexpr EnumMapping<iox::popo::SubscriberEvent, iox_SubscriberEvent> SUBSCRIBER_EVENTS[]{
        {iox::popo::SubscriberEvent::DATA_RECEIVED, SubscriberEvent_DATA_RECEIVED}};

    for (const auto subscriberEvent : SUBSCRIBER_EVENTS)
    {
        switch (subscriberEvent.cpp)
        {
        case iox::popo::SubscriberEvent::DATA_RECEIVED:
            EXPECT_EQ(c2cpp::subscriberEvent(subscriberEvent.c), subscriberEvent.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
// explicitly commented out since we are testing undefined behavior here and that we
// return the default value DATA_RECEIVED always in the undefined behavior case
// the clang sanitizer detects this successfully and this leads to termination, and with this the test fails
#if !defined(__clang__)
    IOX_EXPECT_FATAL_FAILURE(
        [&] { c2cpp::subscriberEvent(iox_test_binding_c::maxUnderlyingCEnumValue<iox_SubscriberEvent>()); },
        iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_EVENT_VALUE);
#endif
}

TEST(c2cpp_enum_translation_test, ConsumerTooSlowPolicy)
{
    ::testing::Test::RecordProperty("TEST_ID", "2984ede1-7ef3-412e-8b4d-d597d305ad41");
    constexpr EnumMapping<iox::popo::ConsumerTooSlowPolicy, iox_ConsumerTooSlowPolicy> STATES[]{
        {iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER, ConsumerTooSlowPolicy_WAIT_FOR_CONSUMER},
        {iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA, ConsumerTooSlowPolicy_DISCARD_OLDEST_DATA}};

    for (const auto state : STATES)
    {
        switch (state.cpp)
        {
        case iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER:
            EXPECT_EQ(c2cpp::consumerTooSlowPolicy(state.c), state.cpp);
            break;
        case iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA:
            EXPECT_EQ(c2cpp::consumerTooSlowPolicy(state.c), state.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
// explicitly commented out since we are testing undefined behavior here and that we
// return the default value DISCARD_OLDEST_DATA always in the undefined behavior case
// the clang sanitizer detects this successfully and this leads to termination, and with this the test fails
#if !defined(__clang__)
    EXPECT_EQ(c2cpp::consumerTooSlowPolicy(iox_test_binding_c::maxUnderlyingCEnumValue<iox_ConsumerTooSlowPolicy>()),
              iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA);

    IOX_TESTING_EXPECT_ERROR(iox::CBindingError::BINDING_C__UNDEFINED_STATE_IN_IOX_CONSUMER_TOO_SLOW_POLICY);
#endif
}

TEST(c2cpp_enum_translation_test, QueueFullPolicy)
{
    ::testing::Test::RecordProperty("TEST_ID", "741e6e92-43c5-4218-ba15-05b0a510f489");
    constexpr EnumMapping<iox::popo::QueueFullPolicy, iox_QueueFullPolicy> STATES[]{
        {iox::popo::QueueFullPolicy::BLOCK_PRODUCER, QueueFullPolicy_BLOCK_PRODUCER},
        {iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA, QueueFullPolicy_DISCARD_OLDEST_DATA}};

    for (const auto state : STATES)
    {
        switch (state.cpp)
        {
        case iox::popo::QueueFullPolicy::BLOCK_PRODUCER:
            EXPECT_EQ(c2cpp::queueFullPolicy(state.c), state.cpp);
            break;
        case iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA:
            EXPECT_EQ(c2cpp::queueFullPolicy(state.c), state.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
// explicitly commented out since we are testing undefined behavior here and that we
// return the default value DISCARD_OLDEST_DATA always in the undefined behavior case
// the clang sanitizer detects this successfully and this leads to termination, and with this the test fails
#if !defined(__clang__)
    EXPECT_EQ(c2cpp::queueFullPolicy(iox_test_binding_c::maxUnderlyingCEnumValue<iox_QueueFullPolicy>()),
              iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA);

    IOX_TESTING_EXPECT_ERROR(iox::CBindingError::BINDING_C__UNDEFINED_STATE_IN_IOX_QUEUE_FULL_POLICY);
#endif
}

TEST(c2cpp_enum_translation_test, ClientState)
{
    ::testing::Test::RecordProperty("TEST_ID", "e1cbdde1-7ead-42e2-8ba3-f680629c7c09");
    constexpr EnumMapping<iox::popo::ClientState, iox_ClientState> STATES[]{
        {iox::popo::ClientState::HAS_RESPONSE, ClientState_HAS_RESPONSE}};

    for (const auto state : STATES)
    {
        switch (state.cpp)
        {
        case iox::popo::ClientState::HAS_RESPONSE:
            EXPECT_EQ(c2cpp::clientState(state.c), state.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
// explicitly commented out since we are testing undefined behavior here and that we
// return the default value HAS_RESPONSE always in the undefined behavior case
// the clang sanitizer detects this successfully and this leads to termination, and with this the test fails
#if !defined(__clang__)
    IOX_EXPECT_FATAL_FAILURE(
        [&] { c2cpp::clientState(iox_test_binding_c::maxUnderlyingCEnumValue<iox_ClientState>()); },
        iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_CLIENT_STATE_VALUE);
#endif
}

TEST(c2cpp_enum_translation_test, ClientEvent)
{
    ::testing::Test::RecordProperty("TEST_ID", "2d0be79d-dd12-4195-bf11-079efeea51d9");
    constexpr EnumMapping<iox::popo::ClientEvent, iox_ClientEvent> EVENTS[]{
        {iox::popo::ClientEvent::RESPONSE_RECEIVED, ClientEvent_RESPONSE_RECEIVED}};

    for (const auto event : EVENTS)
    {
        switch (event.cpp)
        {
        case iox::popo::ClientEvent::RESPONSE_RECEIVED:
            EXPECT_EQ(c2cpp::clientEvent(event.c), event.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
// explicitly commented out since we are testing undefined behavior here and that we
// return the default value RESPONSE_RECEIVED always in the undefined behavior case
// the clang sanitizer detects this successfully and this leads to termination, and with this the test fails
#if !defined(__clang__)
    IOX_EXPECT_FATAL_FAILURE(
        [&] { c2cpp::clientEvent(iox_test_binding_c::maxUnderlyingCEnumValue<iox_ClientEvent>()); },
        iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_CLIENT_EVENT_VALUE);
#endif
}

TEST(c2cpp_enum_translation_test, ServerState)
{
    ::testing::Test::RecordProperty("TEST_ID", "eb66ed46-948c-4e99-a13e-696bd7484388");
    constexpr EnumMapping<iox::popo::ServerState, iox_ServerState> STATES[]{
        {iox::popo::ServerState::HAS_REQUEST, ServerState_HAS_REQUEST}};

    for (const auto state : STATES)
    {
        switch (state.cpp)
        {
        case iox::popo::ServerState::HAS_REQUEST:
            EXPECT_EQ(c2cpp::serverState(state.c), state.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
// explicitly commented out since we are testing undefined behavior here and that we
// return the default value HAS_REQUEST always in the undefined behavior case
// the clang sanitizer detects this successfully and this leads to termination, and with this the test fails
#if !defined(__clang__)
    IOX_EXPECT_FATAL_FAILURE(
        [&] { c2cpp::serverState(iox_test_binding_c::maxUnderlyingCEnumValue<iox_ServerState>()); },
        iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SERVER_STATE_VALUE);
#endif
}

TEST(c2cpp_enum_translation_test, ServerEvent)
{
    ::testing::Test::RecordProperty("TEST_ID", "23c2f96a-a000-4f9d-8899-d763e98cc985");
    constexpr EnumMapping<iox::popo::ServerEvent, iox_ServerEvent> EVENTS[]{
        {iox::popo::ServerEvent::REQUEST_RECEIVED, ServerEvent_REQUEST_RECEIVED}};

    for (const auto event : EVENTS)
    {
        switch (event.cpp)
        {
        case iox::popo::ServerEvent::REQUEST_RECEIVED:
            EXPECT_EQ(c2cpp::serverEvent(event.c), event.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
// explicitly commented out since we are testing undefined behavior here and that we
// return the default value REQUEST_RECEIVED always in the undefined behavior case
// the clang sanitizer detects this successfully and this leads to termination, and with this the test fails
#if !defined(__clang__)
    IOX_EXPECT_FATAL_FAILURE(
        [&] { c2cpp::serverEvent(iox_test_binding_c::maxUnderlyingCEnumValue<iox_ServerEvent>()); },
        iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SERVER_EVENT_VALUE);
#endif
}

TEST(c2cpp_enum_translation_test, ServiceDiscoveryEvent)
{
    ::testing::Test::RecordProperty("TEST_ID", "9eb978fa-8fa9-452f-b0e3-cb26c0cecfbf");
    constexpr EnumMapping<iox::runtime::ServiceDiscoveryEvent, iox_ServiceDiscoveryEvent> EVENTS[]{
        {iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
         ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED}};

    for (const auto event : EVENTS)
    {
        switch (event.cpp)
        {
        case iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED:
            EXPECT_EQ(c2cpp::serviceDiscoveryEvent(event.c), event.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
// explicitly commented out since we are testing undefined behavior here and that we
// return the default value SERVICE_REGISTRY_CHANGED always in the undefined behavior case
// the clang sanitizer detects this successfully and this leads to termination, and with this the test fails
#if !defined(__clang__)
    IOX_EXPECT_FATAL_FAILURE(
        [&] { c2cpp::serviceDiscoveryEvent(iox_test_binding_c::maxUnderlyingCEnumValue<iox_ServiceDiscoveryEvent>()); },
        iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SERVICE_DISCOVERY_EVENT_VALUE);
#endif
}

TEST(c2cpp_enum_translation_test, MessagingPattern)
{
    ::testing::Test::RecordProperty("TEST_ID", "934d1fa5-e345-4a3b-9730-5467ec03c281");
    constexpr EnumMapping<iox::popo::MessagingPattern, iox_MessagingPattern> VALUE[]{
        {iox::popo::MessagingPattern::PUB_SUB, MessagingPattern_PUB_SUB},
        {iox::popo::MessagingPattern::REQ_RES, MessagingPattern_REQ_RES},
    };

    for (const auto value : VALUE)
    {
        switch (value.cpp)
        {
        case iox::popo::MessagingPattern::PUB_SUB:
            EXPECT_EQ(c2cpp::messagingPattern(value.c), value.cpp);
            break;
        case iox::popo::MessagingPattern::REQ_RES:
            EXPECT_EQ(c2cpp::messagingPattern(value.c), value.cpp);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

// ignored for now since the undefined behavior sanitizer correctly detects the undefined behavior
// which is tested and handled here
// explicitly commented out since we are testing undefined behavior here and that we
// return the default value PUB_SUB always in the undefined behavior case
// the clang sanitizer detects this successfully and this leads to termination, and with this the test fails
#if !defined(__clang__)
    IOX_EXPECT_FATAL_FAILURE(
        [&] { c2cpp::messagingPattern(iox_test_binding_c::maxUnderlyingCEnumValue<iox_MessagingPattern>()); },
        iox::CBindingError::BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_MESSAGING_PATTERN_VALUE);
#endif
}

} // namespace
