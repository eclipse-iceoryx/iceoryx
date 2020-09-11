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

#include "iceoryx_posh/internal/popo/ports/client_port_roudi.hpp"

namespace iox
{
namespace popo
{
ClientPortRouDi::ClientPortRouDi(cxx::not_null<MemberType_t* const> clientPortDataPtr) noexcept
    : BasePort(clientPortDataPtr)
    , m_chunkSender(&getMembers()->m_chunkSenderData)
    , m_chunkReceiver(&getMembers()->m_chunkReceiverData)

{
}

const ClientPortRouDi::MemberType_t* ClientPortRouDi::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

ClientPortRouDi::MemberType_t* ClientPortRouDi::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

cxx::optional<capro::CaproMessage> ClientPortRouDi::tryGetCaProMessage() noexcept
{
    /// @todo

    // nothing to change
    return cxx::nullopt_t();
}

cxx::optional<capro::CaproMessage>
ClientPortRouDi::dispatchCaProMessageAndGetPossibleResponse(const capro::CaproMessage& /*caProMessage*/) noexcept
{
    /// @todo

    capro::CaproMessage responseMessage(
        capro::CaproMessageType::NACK, this->getCaProServiceDescription(), capro::CaproMessageSubType::NOSUBTYPE);

    return cxx::make_optional<capro::CaproMessage>(responseMessage);
}

void ClientPortRouDi::releaseAllChunks() noexcept
{
    m_chunkSender.releaseAll();
    m_chunkReceiver.releaseAll();
}

} // namespace popo
} // namespace iox
