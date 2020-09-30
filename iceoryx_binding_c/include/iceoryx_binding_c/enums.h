// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_BINDING_C_ENUMS_H
#define IOX_BINDING_C_ENUMS_H

/// @brief describes the current state of a subscriber
enum iox_SubscribeState
{
    SubscribeState_NOT_SUBSCRIBED = 0,
    SubscribeState_SUBSCRIBE_REQUESTED,
    SubscribeState_SUBSCRIBED,
    SubscribeState_UNSUBSCRIBE_REQUESTED,
    SubscribeState_WAIT_FOR_OFFER,
    SubscribeState_UNDEFINED_ERROR,
};

/// @brief describes the state of getChunk in the subscriber
enum iox_ChunkReceiveResult
{
    ChunkReceiveResult_TOO_MANY_CHUNKS_HELD_IN_PARALLEL,
    ChunkReceiveResult_NO_CHUNK_RECEIVED,
    ChunkReceiveResult_UNDEFINED_ERROR,
    ChunkReceiveResult_SUCCESS,
};

/// @brief state of allocateChunk
enum iox_AllocationResult
{
    AllocationResult_RUNNING_OUT_OF_CHUNKS,
    AllocationResult_TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL,
    AllocationResult_UNDEFINED_ERROR,
    AllocationResult_SUCCESS,
};

enum iox_WaitSetResult
{
    WaitSetResult_CONDITION_VECTOR_OVERFLOW,
    WaitSetResult_CONDITION_VARIABLE_ALREADY_SET,
    WaitSetResult_CONDITION_VARIABLE_ATTACH_FAILED,
    WaitSetResult_UNDEFINED_ERROR,
    WaitSetResult_SUCCESS
};


#endif
