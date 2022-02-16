// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

template <typename CPP, typename C>
struct EnumMapping
{
    CPP cpp;
    C c;
};

TEST(cpp2c_enum_translation_test, SubscribeStateCorrectAndFullTranslation)
{
    ::testing::Test::RecordProperty("TEST_ID", "47d1e613-b9cf-492f-af0a-7da9d46399ba");
    constexpr EnumMapping<iox::SubscribeState, iox_SubscribeState> SUBSCRIBE_STATES[]{
        {iox::SubscribeState::NOT_SUBSCRIBED, SubscribeState_NOT_SUBSCRIBED},
        {iox::SubscribeState::SUBSCRIBE_REQUESTED, SubscribeState_SUBSCRIBE_REQUESTED},
        {iox::SubscribeState::SUBSCRIBED, SubscribeState_SUBSCRIBED},
        {iox::SubscribeState::UNSUBSCRIBE_REQUESTED, SubscribeState_UNSUBSCRIBE_REQUESTED},
        {iox::SubscribeState::WAIT_FOR_OFFER, SubscribeState_WAIT_FOR_OFFER}};

    for (const auto subscribeState : SUBSCRIBE_STATES)
    {
        switch (subscribeState.cpp)
        {
        case iox::SubscribeState::NOT_SUBSCRIBED:
            EXPECT_EQ(cpp2c::subscribeState(subscribeState.cpp), subscribeState.c);
            break;
        case iox::SubscribeState::SUBSCRIBE_REQUESTED:
            EXPECT_EQ(cpp2c::subscribeState(subscribeState.cpp), subscribeState.c);
            break;
        case iox::SubscribeState::SUBSCRIBED:
            EXPECT_EQ(cpp2c::subscribeState(subscribeState.cpp), subscribeState.c);
            break;
        case iox::SubscribeState::UNSUBSCRIBE_REQUESTED:
            EXPECT_EQ(cpp2c::subscribeState(subscribeState.cpp), subscribeState.c);
            break;
        case iox::SubscribeState::WAIT_FOR_OFFER:
            EXPECT_EQ(cpp2c::subscribeState(subscribeState.cpp), subscribeState.c);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::subscribeState(static_cast<iox::SubscribeState>(-1)), SubscribeState_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, ChunkReceiveResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "873adad3-9eb4-496f-9ecc-a941e225d0cb");
    constexpr EnumMapping<iox::popo::ChunkReceiveResult, iox_ChunkReceiveResult> CHUNK_RECEIVE_RESULTS[]{
        {iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE, ChunkReceiveResult_NO_CHUNK_AVAILABLE},
        {iox::popo::ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL,
         ChunkReceiveResult_TOO_MANY_CHUNKS_HELD_IN_PARALLEL}};

    for (const auto chunkReceiveResult : CHUNK_RECEIVE_RESULTS)
    {
        switch (chunkReceiveResult.cpp)
        {
        case iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE:
            EXPECT_EQ(cpp2c::chunkReceiveResult(chunkReceiveResult.cpp), chunkReceiveResult.c);
            break;
        case iox::popo::ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL:
            EXPECT_EQ(cpp2c::chunkReceiveResult(chunkReceiveResult.cpp), chunkReceiveResult.c);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::chunkReceiveResult(static_cast<iox::popo::ChunkReceiveResult>(-1)),
              ChunkReceiveResult_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, AllocationResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "43296599-2b53-4b9d-8e37-2911bbd1b397");
    constexpr EnumMapping<iox::popo::AllocationError, iox_AllocationResult> ALLOCATION_ERRORS[]{
        {iox::popo::AllocationError::UNDEFINED_ERROR, AllocationResult_UNDEFINED_ERROR},
        {iox::popo::AllocationError::NO_MEMPOOLS_AVAILABLE, AllocationResult_NO_MEMPOOLS_AVAILABLE},
        {iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS, AllocationResult_RUNNING_OUT_OF_CHUNKS},
        {iox::popo::AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL,
         AllocationResult_TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL},
        {iox::popo::AllocationError::INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER,
         AllocationResult_INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER},
        {iox::popo::AllocationError::INVALID_PARAMETER_FOR_REQUEST_HEADER,
         AllocationResult_INVALID_PARAMETER_FOR_REQUEST_HEADER}};

    for (const auto allocationError : ALLOCATION_ERRORS)
    {
        switch (allocationError.cpp)
        {
        case iox::popo::AllocationError::UNDEFINED_ERROR:
            EXPECT_EQ(cpp2c::allocationResult(allocationError.cpp), allocationError.c);
            break;
        case iox::popo::AllocationError::NO_MEMPOOLS_AVAILABLE:
            EXPECT_EQ(cpp2c::allocationResult(allocationError.cpp), allocationError.c);
            break;
        case iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS:
            EXPECT_EQ(cpp2c::allocationResult(allocationError.cpp), allocationError.c);
            break;
        case iox::popo::AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL:
            EXPECT_EQ(cpp2c::allocationResult(allocationError.cpp), allocationError.c);
            break;
        case iox::popo::AllocationError::INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER:
            EXPECT_EQ(cpp2c::allocationResult(allocationError.cpp), allocationError.c);
            break;
        case iox::popo::AllocationError::INVALID_PARAMETER_FOR_REQUEST_HEADER:
            EXPECT_EQ(cpp2c::allocationResult(allocationError.cpp), allocationError.c);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::allocationResult(static_cast<iox::popo::AllocationError>(-1)), AllocationResult_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, WaitSetResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b2fbd01-38b4-414d-be21-70d00d2d8fbf");
    constexpr EnumMapping<iox::popo::WaitSetError, iox_WaitSetResult> WAIT_SET_ERRORS[]{
        {iox::popo::WaitSetError::WAIT_SET_FULL, WaitSetResult_WAIT_SET_FULL},
        {iox::popo::WaitSetError::ALREADY_ATTACHED, WaitSetResult_ALREADY_ATTACHED}};

    for (const auto waitSetError : WAIT_SET_ERRORS)
    {
        switch (waitSetError.cpp)
        {
        case iox::popo::WaitSetError::WAIT_SET_FULL:
            EXPECT_EQ(cpp2c::waitSetResult(waitSetError.cpp), waitSetError.c);
            break;
        case iox::popo::WaitSetError::ALREADY_ATTACHED:
            EXPECT_EQ(cpp2c::waitSetResult(waitSetError.cpp), waitSetError.c);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::waitSetResult(static_cast<iox::popo::WaitSetError>(-1)), WaitSetResult_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, ListenerResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e46311d-4579-4437-adf7-0f5c3adcd511");
    constexpr EnumMapping<iox::popo::ListenerError, iox_ListenerResult> LISTENER_ERRORS[]{
        {iox::popo::ListenerError::LISTENER_FULL, ListenerResult_LISTENER_FULL},
        {iox::popo::ListenerError::EVENT_ALREADY_ATTACHED, ListenerResult_EVENT_ALREADY_ATTACHED},
        {iox::popo::ListenerError::EMPTY_EVENT_CALLBACK, ListenerResult_EMPTY_EVENT_CALLBACK},
        {iox::popo::ListenerError::EMPTY_INVALIDATION_CALLBACK, ListenerResult_EMPTY_INVALIDATION_CALLBACK}};

    for (const auto listenerError : LISTENER_ERRORS)
    {
        switch (listenerError.cpp)
        {
        case iox::popo::ListenerError::LISTENER_FULL:
            EXPECT_EQ(cpp2c::listenerResult(listenerError.cpp), listenerError.c);
            break;
        case iox::popo::ListenerError::EVENT_ALREADY_ATTACHED:
            EXPECT_EQ(cpp2c::listenerResult(listenerError.cpp), listenerError.c);
            break;
        case iox::popo::ListenerError::EMPTY_EVENT_CALLBACK:
            EXPECT_EQ(cpp2c::listenerResult(listenerError.cpp), listenerError.c);
            break;
        case iox::popo::ListenerError::EMPTY_INVALIDATION_CALLBACK:
            EXPECT_EQ(cpp2c::listenerResult(listenerError.cpp), listenerError.c);
            break;
            // default intentionally left out in order to get a compiler warning if the enum gets extended and we forgot
            // to extend the test
        }
    }

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::listenerResult(static_cast<iox::popo::ListenerError>(-1)), ListenerResult_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, ConsumerTooSlowPolicy)
{
    ::testing::Test::RecordProperty("TEST_ID", "aad706c2-5216-4835-b29f-8a89829d7238");
    EXPECT_EQ(cpp2c::consumerTooSlowPolicy(iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER),
              ConsumerTooSlowPolicy_WAIT_FOR_CONSUMER);
    EXPECT_EQ(cpp2c::consumerTooSlowPolicy(iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA),
              ConsumerTooSlowPolicy_DISCARD_OLDEST_DATA);
    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::consumerTooSlowPolicy(static_cast<iox::popo::ConsumerTooSlowPolicy>(-1)),
              ConsumerTooSlowPolicy_DISCARD_OLDEST_DATA);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, QueueFullPolicy)
{
    ::testing::Test::RecordProperty("TEST_ID", "067a164b-b143-47c7-8bab-962dbe519726");
    EXPECT_EQ(cpp2c::queueFullPolicy(iox::popo::QueueFullPolicy::BLOCK_PRODUCER), QueueFullPolicy_BLOCK_PRODUCER);
    EXPECT_EQ(cpp2c::queueFullPolicy(iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA),
              QueueFullPolicy_DISCARD_OLDEST_DATA);
    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::queueFullPolicy(static_cast<iox::popo::QueueFullPolicy>(-1)), QueueFullPolicy_DISCARD_OLDEST_DATA);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, ClientEvent)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab11a6c4-309f-4403-8feb-d65ec8ab7b2d");
    EXPECT_EQ(cpp2c::clientEvent(iox::popo::ClientEvent::RESPONSE_RECEIVED), ClientEvent_RESPONSE_RECEIVED);
    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::clientEvent(static_cast<iox::popo::ClientEvent>(-1)), ClientEvent_RESPONSE_RECEIVED);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, ClientState)
{
    ::testing::Test::RecordProperty("TEST_ID", "7dd9ddce-2316-4a61-ace7-0c83b1b8e4c7");
    EXPECT_EQ(cpp2c::clientState(iox::popo::ClientState::HAS_RESPONSE), ClientState_HAS_RESPONSE);
    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::clientState(static_cast<iox::popo::ClientState>(-1)), ClientState_HAS_RESPONSE);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, ServerEvent)
{
    ::testing::Test::RecordProperty("TEST_ID", "e6e4511f-1f14-405a-9edc-6100862a3a9f");
    EXPECT_EQ(cpp2c::serverEvent(iox::popo::ServerEvent::REQUEST_RECEIVED), ServerEvent_REQUEST_RECEIVED);
    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::serverEvent(static_cast<iox::popo::ServerEvent>(-1)), ServerEvent_REQUEST_RECEIVED);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, ServerState)
{
    ::testing::Test::RecordProperty("TEST_ID", "f9750af5-9002-4b08-bde5-16181b50b7d9");
    EXPECT_EQ(cpp2c::serverState(iox::popo::ServerState::HAS_REQUEST), ServerState_HAS_REQUEST);
    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::serverState(static_cast<iox::popo::ServerState>(-1)), ServerState_HAS_REQUEST);
#pragma GCC diagnostic pop
}
} // namespace
