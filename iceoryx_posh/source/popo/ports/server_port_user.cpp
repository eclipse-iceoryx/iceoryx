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

cxx::expected<const RequestHeader*, ServerRequestResult> ServerPortUser::getRequest() noexcept
{
    auto getChunkResult = m_chunkReceiver.tryGet();

    if (getChunkResult.has_error())
    {
        if (!isOffered())
        {
            return cxx::error<ServerRequestResult>(ServerRequestResult::NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER);
        }
        /// @todo iox-#1012 use cxx::error<E2>::from(E1); once available
        return cxx::error<ServerRequestResult>(cxx::into<ServerRequestResult>(getChunkResult.get_error()));
    }

    return cxx::success<const RequestHeader*>(static_cast<const RequestHeader*>(getChunkResult.value()->userHeader()));
}

void ServerPortUser::releaseRequest(const RequestHeader* const requestHeader) noexcept
{
    if (requestHeader)
    {
        m_chunkReceiver.release(requestHeader->getChunkHeader());
    }
    else
    {
        LogFatal() << "Provided RequestHeader is a nullptr";
        errorHandler(Error::kPOPO__SERVER_PORT_INVALID_REQUEST_TO_RELEASE_FROM_USER, nullptr, ErrorLevel::SEVERE);
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

cxx::expected<ResponseHeader*, AllocationError>
ServerPortUser::allocateResponse(const RequestHeader* const requestHeader,
                                 const uint32_t userPayloadSize,
                                 const uint32_t userPayloadAlignment) noexcept
{
    if (requestHeader == nullptr)
    {
        return cxx::error<AllocationError>(AllocationError::INVALID_PARAMETER_FOR_REQUEST_HEADER);
    }

    auto allocateResult = m_chunkSender.tryAllocate(
        getUniqueID(), userPayloadSize, userPayloadAlignment, sizeof(ResponseHeader), alignof(ResponseHeader));

    if (allocateResult.has_error())
    {
        return cxx::error<AllocationError>(allocateResult.get_error());
    }

    auto responseHeader =
        new (allocateResult.value()->userHeader()) ResponseHeader(requestHeader->m_uniqueClientQueueId,
                                                                  requestHeader->m_lastKnownClientQueueIndex,
                                                                  requestHeader->getSequenceId());

    return cxx::success<ResponseHeader*>(responseHeader);
}

void ServerPortUser::freeResponse(ResponseHeader* const responseHeader) noexcept
{
    if (responseHeader)
    {
        m_chunkSender.release(responseHeader->getChunkHeader());
    }
    else
    {
        LogFatal() << "Provided ResponseHeader is a nullptr";
        errorHandler(Error::kPOPO__SERVER_PORT_INVALID_RESPONSE_TO_FREE_FROM_USER, nullptr, ErrorLevel::SEVERE);
    }
}

void ServerPortUser::sendResponse(ResponseHeader* const responseHeader) noexcept
{
    if (responseHeader == nullptr)
    {
        LogFatal() << "Provided ResponseHeader is a nullptr";
        errorHandler(Error::kPOPO__SERVER_PORT_INVALID_RESPONSE_TO_SEND_FROM_USER, nullptr, ErrorLevel::SEVERE);
        return;
    }

    const auto offerRequested = getMembers()->m_offeringRequested.load(std::memory_order_relaxed);

    if (offerRequested)
    {
        m_chunkSender.getQueueIndex(responseHeader->m_uniqueClientQueueId, responseHeader->m_lastKnownClientQueueIndex)
            .and_then([&](auto queueIndex) {
                responseHeader->m_lastKnownClientQueueIndex = queueIndex;
                m_chunkSender.sendToQueue(
                    responseHeader->getChunkHeader(), responseHeader->m_uniqueClientQueueId, queueIndex);
            })
            .or_else([&] {
                freeResponse(responseHeader);
                LogWarn() << "Could not deliver to queue! Queue not available anymore!";
            });
    }
    else
    {
        freeResponse(responseHeader);
        LogWarn() << "Try to send response without having offered!";
    }
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
