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

#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"

namespace iox
{
namespace popo
{
ServerPortUser::ServerPortUser(MemberType_t& serverPortData) noexcept
    : BasePort(&serverPortData)
    , m_chunkSender(&getMembers()->m_chunkSenderData)
    , m_chunkReceiver(&getMembers()->m_chunkReceiverData)
{
}

const ServerPortUser::MemberType_t* ServerPortUser::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

ServerPortUser::MemberType_t* ServerPortUser::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

expected<const RequestHeader*, ServerRequestResult> ServerPortUser::getRequest() noexcept
{
    auto getChunkResult = m_chunkReceiver.tryGet();

    if (getChunkResult.has_error())
    {
        if (!isOffered())
        {
            return err(ServerRequestResult::NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER);
        }
        /// @todo iox-#1012 use error<E2>::from(E1); once available
        return err(into<ServerRequestResult>(getChunkResult.error()));
    }

    return ok(static_cast<const RequestHeader*>(getChunkResult.value()->userHeader()));
}

void ServerPortUser::releaseRequest(const RequestHeader* const requestHeader) noexcept
{
    if (requestHeader != nullptr)
    {
        m_chunkReceiver.release(requestHeader->getChunkHeader());
    }
    else
    {
        IOX_LOG(ERROR, "Provided RequestHeader is a nullptr");
        IOX_REPORT(PoshError::POPO__SERVER_PORT_INVALID_REQUEST_TO_RELEASE_FROM_USER, iox::er::RUNTIME_ERROR);
    }
}

void ServerPortUser::releaseQueuedRequests() noexcept
{
    m_chunkReceiver.clear();
}

bool ServerPortUser::hasNewRequests() const noexcept
{
    return !m_chunkReceiver.empty();
}

bool ServerPortUser::hasLostRequestsSinceLastCall() noexcept
{
    return m_chunkReceiver.hasLostChunks();
}

expected<ResponseHeader*, AllocationError>
ServerPortUser::allocateResponse(const RequestHeader* const requestHeader,
                                 const uint64_t userPayloadSize,
                                 const uint32_t userPayloadAlignment) noexcept
{
    if (requestHeader == nullptr)
    {
        return err(AllocationError::INVALID_PARAMETER_FOR_REQUEST_HEADER);
    }

    auto allocateResult = m_chunkSender.tryAllocate(
        getUniqueID(), userPayloadSize, userPayloadAlignment, sizeof(ResponseHeader), alignof(ResponseHeader));

    if (allocateResult.has_error())
    {
        return err(allocateResult.error());
    }

    auto* responseHeader =
        new (allocateResult.value()->userHeader()) ResponseHeader(requestHeader->m_uniqueClientQueueId,
                                                                  requestHeader->m_lastKnownClientQueueIndex,
                                                                  requestHeader->getSequenceId());

    return ok(responseHeader);
}

void ServerPortUser::releaseResponse(const ResponseHeader* const responseHeader) noexcept
{
    if (responseHeader != nullptr)
    {
        m_chunkSender.release(responseHeader->getChunkHeader());
    }
    else
    {
        IOX_LOG(ERROR, "Provided ResponseHeader is a nullptr");
        IOX_REPORT(PoshError::POPO__SERVER_PORT_INVALID_RESPONSE_TO_FREE_FROM_USER, iox::er::RUNTIME_ERROR);
    }
}

expected<void, ServerSendError> ServerPortUser::sendResponse(ResponseHeader* const responseHeader) noexcept
{
    if (responseHeader == nullptr)
    {
        IOX_LOG(ERROR, "Provided ResponseHeader is a nullptr");
        IOX_REPORT(PoshError::POPO__SERVER_PORT_INVALID_RESPONSE_TO_SEND_FROM_USER, iox::er::RUNTIME_ERROR);
        return err(ServerSendError::INVALID_RESPONSE);
    }

    const auto offerRequested = getMembers()->m_offeringRequested.load(std::memory_order_relaxed);
    if (!offerRequested)
    {
        releaseResponse(responseHeader);
        IOX_LOG(WARN, "Try to send response without having offered!");
        return err(ServerSendError::NOT_OFFERED);
    }

    bool responseSent{false};
    m_chunkSender.getQueueIndex(responseHeader->m_uniqueClientQueueId, responseHeader->m_lastKnownClientQueueIndex)
        .and_then([&](auto queueIndex) {
            responseHeader->m_lastKnownClientQueueIndex = queueIndex;
            responseSent = m_chunkSender.sendToQueue(
                responseHeader->getChunkHeader(), responseHeader->m_uniqueClientQueueId, queueIndex);
        })
        .or_else([&] { releaseResponse(responseHeader); });

    if (!responseSent)
    {
        IOX_LOG(WARN, "Could not deliver to client! Client not available anymore!");
        return err(ServerSendError::CLIENT_NOT_AVAILABLE);
    }

    return ok();
}

void ServerPortUser::offer() noexcept
{
    if (!getMembers()->m_offeringRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_offeringRequested.store(true, std::memory_order_relaxed);
    }
}

void ServerPortUser::stopOffer() noexcept
{
    if (getMembers()->m_offeringRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_offeringRequested.store(false, std::memory_order_relaxed);
    }
}

bool ServerPortUser::isOffered() const noexcept
{
    return getMembers()->m_offeringRequested.load(std::memory_order_relaxed);
}

bool ServerPortUser::hasClients() const noexcept
{
    return m_chunkSender.hasStoredQueues();
}

void ServerPortUser::setConditionVariable(ConditionVariableData& conditionVariableData,
                                          const uint64_t notificationIndex) noexcept
{
    m_chunkReceiver.setConditionVariable(conditionVariableData, notificationIndex);
}

void ServerPortUser::unsetConditionVariable() noexcept
{
    m_chunkReceiver.unsetConditionVariable();
}

bool ServerPortUser::isConditionVariableSet() const noexcept
{
    return m_chunkReceiver.isConditionVariableSet();
}

} // namespace popo
} // namespace iox
