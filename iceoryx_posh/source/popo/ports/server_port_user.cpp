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
// limitations under the License

#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"

namespace iox
{
namespace popo
{
ServerPortUser::ServerPortUser(cxx::not_null<MemberType_t* const> serverPortDataPtr) noexcept
    : BasePort(serverPortDataPtr)
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


cxx::expected<cxx::optional<const mepoo::ChunkHeader*>, ChunkReceiveError> ServerPortUser::getRequest() noexcept
{
    return m_chunkReceiver.get();
}

void ServerPortUser::releaseRequest(const mepoo::ChunkHeader* chunkHeader) noexcept
{
    m_chunkReceiver.release(chunkHeader);
}

bool ServerPortUser::hasNewRequests() noexcept
{
    return !m_chunkReceiver.empty();
}

bool ServerPortUser::hasLostRequests() noexcept
{
    return m_chunkReceiver.hasOverflown();
}

cxx::expected<mepoo::ChunkHeader*, AllocationError>
ServerPortUser::allocateResponse(const uint32_t payloadSize) noexcept
{
    return m_chunkSender.allocate(payloadSize, getUniqueID());
}

void ServerPortUser::freeResponse(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    m_chunkSender.release(chunkHeader);
}

void ServerPortUser::sendResponse(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    /// @todo
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

bool ServerPortUser::setConditionVariable(ConditionVariableData* conditionVariableDataPtr) noexcept
{
    return m_chunkReceiver.attachConditionVariable(conditionVariableDataPtr);
}

bool ServerPortUser::unsetConditionVariable() noexcept
{
    return m_chunkReceiver.detachConditionVariable();
}

bool ServerPortUser::isConditionVariableSet() noexcept
{
    return m_chunkReceiver.isConditionVariableAttached();
}

} // namespace popo
} // namespace iox
