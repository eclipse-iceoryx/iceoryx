// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"

namespace iox
{
namespace popo
{
ClientPortUser::ClientPortUser(MemberType_t& clientPortData) noexcept
    : BasePort(&clientPortData)
    , m_chunkSender(&getMembers()->m_chunkSenderData)
    , m_chunkReceiver(&getMembers()->m_chunkReceiverData)
{
}

const ClientPortUser::MemberType_t* ClientPortUser::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

ClientPortUser::MemberType_t* ClientPortUser::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

expected<RequestHeader*, AllocationError> ClientPortUser::allocateRequest(const uint64_t userPayloadSize,
                                                                          const uint32_t userPayloadAlignment) noexcept
{
    auto allocateResult = m_chunkSender.tryAllocate(
        getUniqueID(), userPayloadSize, userPayloadAlignment, sizeof(RequestHeader), alignof(RequestHeader));

    if (allocateResult.has_error())
    {
        return err(allocateResult.error());
    }

    auto* requestHeader = new (allocateResult.value()->userHeader())
        RequestHeader(getMembers()->m_chunkReceiverData.m_uniqueId, RpcBaseHeader::UNKNOWN_CLIENT_QUEUE_INDEX);

    return ok(requestHeader);
}

void ClientPortUser::releaseRequest(const RequestHeader* const requestHeader) noexcept
{
    if (requestHeader != nullptr)
    {
        m_chunkSender.release(requestHeader->getChunkHeader());
    }
    else
    {
        IOX_REPORT(PoshError::POPO__CLIENT_PORT_INVALID_REQUEST_TO_FREE_FROM_USER, iox::er::RUNTIME_ERROR);
    }
}

expected<void, ClientSendError> ClientPortUser::sendRequest(RequestHeader* const requestHeader) noexcept
{
    if (requestHeader == nullptr)
    {
        IOX_LOG(ERROR, "Attempted to send a nullptr request!");
        IOX_REPORT(PoshError::POPO__CLIENT_PORT_INVALID_REQUEST_TO_SEND_FROM_USER, iox::er::RUNTIME_ERROR);
        return err(ClientSendError::INVALID_REQUEST);
    }

    const auto connectRequested = getMembers()->m_connectRequested.load(std::memory_order_relaxed);
    if (!connectRequested)
    {
        releaseRequest(requestHeader);
        IOX_LOG(WARN, "Try to send request without being connected!");
        return err(ClientSendError::NO_CONNECT_REQUESTED);
    }

    auto numberOfReceiver = m_chunkSender.send(requestHeader->getChunkHeader());
    if (numberOfReceiver == 0U)
    {
        IOX_LOG(WARN, "Try to send request but server is not available!");
        return err(ClientSendError::SERVER_NOT_AVAILABLE);
    }

    return ok();
}

void ClientPortUser::connect() noexcept
{
    if (!getMembers()->m_connectRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_connectRequested.store(true, std::memory_order_relaxed);
    }
}

void ClientPortUser::disconnect() noexcept
{
    if (getMembers()->m_connectRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_connectRequested.store(false, std::memory_order_relaxed);
    }
}

ConnectionState ClientPortUser::getConnectionState() const noexcept
{
    return getMembers()->m_connectionState.load(std::memory_order_relaxed);
}

expected<const ResponseHeader*, ChunkReceiveResult> ClientPortUser::getResponse() noexcept
{
    auto getChunkResult = m_chunkReceiver.tryGet();

    if (getChunkResult.has_error())
    {
        return err(getChunkResult.error());
    }

    return ok(static_cast<const ResponseHeader*>(getChunkResult.value()->userHeader()));
}

void ClientPortUser::releaseResponse(const ResponseHeader* const responseHeader) noexcept
{
    if (responseHeader != nullptr)
    {
        m_chunkReceiver.release(responseHeader->getChunkHeader());
    }
    else
    {
        IOX_REPORT(PoshError::POPO__CLIENT_PORT_INVALID_RESPONSE_TO_RELEASE_FROM_USER, iox::er::RUNTIME_ERROR);
    }
}

void ClientPortUser::releaseQueuedResponses() noexcept
{
    m_chunkReceiver.clear();
}

bool ClientPortUser::hasNewResponses() const noexcept
{
    return !m_chunkReceiver.empty();
}

bool ClientPortUser::hasLostResponsesSinceLastCall() noexcept
{
    return m_chunkReceiver.hasLostChunks();
}

void ClientPortUser::setConditionVariable(ConditionVariableData& conditionVariableData,
                                          const uint64_t notificationIndex) noexcept
{
    m_chunkReceiver.setConditionVariable(conditionVariableData, notificationIndex);
}

void ClientPortUser::unsetConditionVariable() noexcept
{
    m_chunkReceiver.unsetConditionVariable();
}

bool ClientPortUser::isConditionVariableSet() const noexcept
{
    return m_chunkReceiver.isConditionVariableSet();
}

} // namespace popo
} // namespace iox
