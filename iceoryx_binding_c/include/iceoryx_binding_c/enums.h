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

#ifndef IOX_BINDING_C_ENUMS_H
#define IOX_BINDING_C_ENUMS_H

/// @brief describes states which can be triggered by a subscriber
enum iox_SubscriberState
{
    SubscriberState_HAS_DATA,
};

/// @brief describes events which can be triggered by a subscriber
enum iox_SubscriberEvent
{
    SubscriberEvent_DATA_RECEIVED,
};

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

/// @brief describes events which can be triggered by a client
enum iox_ClientEvent
{
    ClientEvent_RESPONSE_RECEIVED
};

/// @brief describes states which can be triggered by a client
enum iox_ClientState
{
    ClientState_HAS_RESPONSE
};

/// @brief describes events which can be triggered by a server
enum iox_ServerEvent
{
    ServerEvent_REQUEST_RECEIVED
};

/// @brief describes states which can be triggered by a server
enum iox_ServerState
{
    ServerState_HAS_REQUEST
};

/// @brief describes the current connection state of a client
enum iox_ConnectionState
{
    ConnectionState_NOT_CONNECTED = 0,
    ConnectionState_CONNECT_REQUESTED,
    ConnectionState_CONNECTED,
    ConnectionState_DISCONNECT_REQUESTED,
    ConnectionState_WAIT_FOR_OFFER
};

/// @brief describes the state of getChunk in the subscriber
enum iox_ChunkReceiveResult
{
    ChunkReceiveResult_TOO_MANY_CHUNKS_HELD_IN_PARALLEL,
    ChunkReceiveResult_NO_CHUNK_AVAILABLE,
    ChunkReceiveResult_UNDEFINED_ERROR,
    ChunkReceiveResult_SUCCESS,
};

/// @brief describes events which can be triggered by a service discovery
enum iox_ServiceDiscoveryEvent
{
    ServiceDiscoveryEvent_SERVICE_REGISTRY_CHANGED
};

/// @brief Used by consumers to request a specific behavior from the producer; describes whether a producer blocks when
/// consumer queue is full
enum iox_QueueFullPolicy
{
    QueueFullPolicy_BLOCK_PRODUCER,
    QueueFullPolicy_DISCARD_OLDEST_DATA,
};

/// @brief Used by producers how to adjust to slow consumer; describes whether a producer blocks when consumer queue is
/// full
enum iox_ConsumerTooSlowPolicy
{
    ConsumerTooSlowPolicy_WAIT_FOR_CONSUMER,
    ConsumerTooSlowPolicy_DISCARD_OLDEST_DATA,
};

/// @brief state of allocateChunk
enum iox_AllocationResult
{
    AllocationResult_NO_MEMPOOLS_AVAILABLE,
    AllocationResult_RUNNING_OUT_OF_CHUNKS,
    AllocationResult_TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL,
    AllocationResult_INVALID_CHUNK,
    AllocationResult_INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER,
    AllocationResult_UNDEFINED_ERROR,
    AllocationResult_INVALID_PARAMETER_FOR_CHUNK,
    AllocationResult_INVALID_PARAMETER_FOR_REQUEST_HEADER,
    AllocationResult_SUCCESS,
};

/// @brief client send result
enum iox_ClientSendResult
{
    ClientSendResult_SUCCESS,
    ClientSendResult_UNDEFINED_ERROR,
    ClientSendResult_NO_CONNECT_REQUESTED,
    ClientSendResult_SERVER_NOT_AVAILABLE,
    ClientSendResult_INVALID_REQUEST,
};

/// @brief server send result
enum iox_ServerSendResult
{
    ServerSendResult_SUCCESS,
    ServerSendResult_UNDEFINED_ERROR,
    ServerSendResult_NOT_OFFERED,
    ServerSendResult_CLIENT_NOT_AVAILABLE,
    ServerSendResult_INVALID_RESPONSE,
};

/// @brief used to describe if attaching an object to a waitset was successful or the kind of attachment error
enum iox_WaitSetResult
{
    WaitSetResult_WAIT_SET_FULL,
    WaitSetResult_ALREADY_ATTACHED,
    WaitSetResult_UNDEFINED_ERROR,
    WaitSetResult_SUCCESS
};

/// @brief used to describe if attaching an object to a listener was successful or the kind of attachment error
enum iox_ListenerResult
{
    ListenerResult_LISTENER_FULL,
    ListenerResult_EVENT_ALREADY_ATTACHED,
    ListenerResult_EMPTY_EVENT_CALLBACK,
    ListenerResult_UNDEFINED_ERROR,
    ListenerResult_SUCCESS
};

enum iox_ServerRequestResult
{
    ServerRequestResult_TOO_MANY_REQUESTS_HELD_IN_PARALLEL,
    ServerRequestResult_NO_PENDING_REQUESTS,
    ServerRequestResult_UNDEFINED_CHUNK_RECEIVE_ERROR,
    ServerRequestResult_NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER,
    ServerRequestResult_SUCCESS
};

/// @brief used to describe the messaging pattern
enum iox_MessagingPattern
{
    MessagingPattern_PUB_SUB,
    MessagingPattern_REQ_RES
};

#endif
