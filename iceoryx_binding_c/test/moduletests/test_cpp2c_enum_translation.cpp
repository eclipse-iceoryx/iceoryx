// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

using namespace ::testing;

TEST(cpp2c_enum_translation_test, SubscribeState)
{
    EXPECT_EQ(cpp2c::SubscribeState(iox::SubscribeState::NOT_SUBSCRIBED), SubscribeState_NOT_SUBSCRIBED);
    EXPECT_EQ(cpp2c::SubscribeState(iox::SubscribeState::SUBSCRIBE_REQUESTED), SubscribeState_SUBSCRIBE_REQUESTED);
    EXPECT_EQ(cpp2c::SubscribeState(iox::SubscribeState::SUBSCRIBED), SubscribeState_SUBSCRIBED);
    EXPECT_EQ(cpp2c::SubscribeState(iox::SubscribeState::UNSUBSCRIBE_REQUESTED), SubscribeState_UNSUBSCRIBE_REQUESTED);
    EXPECT_EQ(cpp2c::SubscribeState(iox::SubscribeState::WAIT_FOR_OFFER), SubscribeState_WAIT_FOR_OFFER);

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::SubscribeState(static_cast<iox::SubscribeState>(-1)), SubscribeState_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, ChunkReceiveResult)
{
    EXPECT_EQ(cpp2c::ChunkReceiveResult(iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE),
              ChunkReceiveResult_NO_CHUNK_AVAILABLE);

    EXPECT_EQ(cpp2c::ChunkReceiveResult(iox::popo::ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL),
              ChunkReceiveResult_TOO_MANY_CHUNKS_HELD_IN_PARALLEL);

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::ChunkReceiveResult(static_cast<iox::popo::ChunkReceiveResult>(-1)),
              ChunkReceiveResult_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, AllocationResult)
{
    EXPECT_EQ(cpp2c::AllocationResult(iox::popo::AllocationError::RUNNING_OUT_OF_CHUNKS),
              AllocationResult_RUNNING_OUT_OF_CHUNKS);
    EXPECT_EQ(cpp2c::AllocationResult(iox::popo::AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL),
              AllocationResult_TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL);

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::AllocationResult(static_cast<iox::popo::AllocationError>(-1)), AllocationResult_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, WaitSetResult)
{
    EXPECT_EQ(cpp2c::WaitSetResult(iox::popo::WaitSetError::WAIT_SET_FULL), WaitSetResult_WAIT_SET_FULL);
    EXPECT_EQ(cpp2c::WaitSetResult(iox::popo::WaitSetError::EVENT_ALREADY_ATTACHED),
              WaitSetResult_EVENT_ALREADY_ATTACHED);

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::WaitSetResult(static_cast<iox::popo::WaitSetError>(-1)), WaitSetResult_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}

TEST(cpp2c_enum_translation_test, ListenerResult)
{
    EXPECT_EQ(cpp2c::ListenerResult(iox::popo::ListenerError::LISTENER_FULL), ListenerResult_LISTENER_FULL);
    EXPECT_EQ(cpp2c::ListenerResult(iox::popo::ListenerError::EVENT_ALREADY_ATTACHED),
              ListenerResult_EVENT_ALREADY_ATTACHED);

    // ignore the warning since we would like to test the behavior of an invalid enum value
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    EXPECT_EQ(cpp2c::ListenerResult(static_cast<iox::popo::ListenerError>(-1)), ListenerResult_UNDEFINED_ERROR);
#pragma GCC diagnostic pop
}
