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

#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/enums.h"

using namespace iox;
using namespace iox::popo;

namespace cpp2c
{
iox_SubscribeState SubscribeState(const iox::SubscribeState value)
{
    switch (value)
    {
    case iox::SubscribeState::NOT_SUBSCRIBED:
        return iox_SubscribeState::SubscribeState_NOT_SUBSCRIBED;
    case iox::SubscribeState::SUBSCRIBE_REQUESTED:
        return iox_SubscribeState::SubscribeState_SUBSCRIBE_REQUESTED;
    case iox::SubscribeState::SUBSCRIBED:
        return iox_SubscribeState::SubscribeState_SUBSCRIBED;
    case iox::SubscribeState::UNSUBSCRIBE_REQUESTED:
        return iox_SubscribeState::SubscribeState_UNSUBSCRIBE_REQUESTED;
    case iox::SubscribeState::WAIT_FOR_OFFER:
        return iox_SubscribeState::SubscribeState_WAIT_FOR_OFFER;
    default:
        return iox_SubscribeState::SubscribeState_UNDEFINED_ERROR;
    }
}

iox_ChunkReceiveResult ChunkReceiveResult(const iox::popo::ChunkReceiveResult value)
{
    switch (value)
    {
    case ChunkReceiveResult::NO_CHUNK_AVAILABLE:
        return ChunkReceiveResult_NO_CHUNK_AVAILABLE;
    case ChunkReceiveResult::TOO_MANY_CHUNKS_HELD_IN_PARALLEL:
        return ChunkReceiveResult_TOO_MANY_CHUNKS_HELD_IN_PARALLEL;
    default:
        return ChunkReceiveResult_UNDEFINED_ERROR;
    }
}

iox_AllocationResult AllocationResult(const iox::popo::AllocationError value)
{
    switch (value)
    {
    case AllocationError::RUNNING_OUT_OF_CHUNKS:
        return AllocationResult_RUNNING_OUT_OF_CHUNKS;
    case AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL:
        return AllocationResult_TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL;
    default:
        return AllocationResult_UNDEFINED_ERROR;
    }
}

iox_WaitSetResult WaitSetResult(const iox::popo::WaitSetError value)
{
    switch (value)
    {
    case WaitSetError::WAIT_SET_FULL:
        return WaitSetResult_WAIT_SET_FULL;
    case WaitSetError::EVENT_ALREADY_ATTACHED:
        return WaitSetResult_EVENT_ALREADY_ATTACHED;
    default:
        return WaitSetResult_UNDEFINED_ERROR;
    }
}

} // namespace cpp2c
