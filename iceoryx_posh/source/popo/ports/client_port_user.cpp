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

#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"

namespace iox
{
namespace popo
{
ClientPortUser::ClientPortUser(cxx::not_null<MemberType_t* const> clientPortDataPtr) noexcept
    : BasePort(clientPortDataPtr)
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

cxx::expected<mepoo::ChunkHeader*, AllocationError>
ClientPortUser::allocateRequest(const uint32_t payloadSize) noexcept
{
    return m_chunkSender.allocate(payloadSize, getUniqueID());
}

void ClientPortUser::freeRequest(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    m_chunkSender.release(chunkHeader);
}

void ClientPortUser::sendRequest(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    /// @todo
}

void ClientPortUser::connect() noexcept
{
    /// @todo
}

void ClientPortUser::disconnect() noexcept
{
    /// @todo
}

ConnectionState ClientPortUser::getConnectionState() const noexcept
{
    return getMembers()->m_connectionState;
}

cxx::expected<cxx::optional<const mepoo::ChunkHeader*>, ChunkReceiveError> ClientPortUser::getResponse() noexcept
{
    return m_chunkReceiver.get();
}

void ClientPortUser::releaseResponse(const mepoo::ChunkHeader* chunkHeader) noexcept
{
    m_chunkReceiver.release(chunkHeader);
}

bool ClientPortUser::hasNewResponses() noexcept
{
    return !m_chunkReceiver.empty();
}

bool ClientPortUser::hasLostResponses() noexcept
{
    return m_chunkReceiver.hasOverflown();
}

bool ClientPortUser::setConditionVariable(ConditionVariableData* conditionVariableDataPtr) noexcept
{
    return m_chunkReceiver.attachConditionVariable(conditionVariableDataPtr);
}

bool ClientPortUser::unsetConditionVariable() noexcept
{
    return m_chunkReceiver.detachConditionVariable();
}

bool ClientPortUser::isConditionVariableSet() noexcept
{
    return m_chunkReceiver.isConditionVariableAttached();
}

} // namespace popo
} // namespace iox
