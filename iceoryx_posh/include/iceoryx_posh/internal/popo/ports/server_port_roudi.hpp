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
#ifndef IOX_POSH_POPO_PORTS_SERVER_PORT_ROUDI_HPP
#define IOX_POSH_POPO_PORTS_SERVER_PORT_ROUDI_HPP

#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_receiver.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_data.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

namespace iox
{
namespace popo
{
/// @brief The ServerPortRouDi provides the API for accessing a server port from the RouDi middleware daemon side.
/// The server port is divided in the three parts ServerPortData, ServerPortRouDi and ServerPortUser.
/// The ServerPortRouDi provides service discovery functionality that is based on CaPro messages. With this API the
/// dynamic connections between clients and servers ports can be established
class ServerPortRouDi : public BasePort
{
  public:
    using MemberType_t = ServerPortData;

    explicit ServerPortRouDi(cxx::not_null<MemberType_t* const> serverPortDataPtr) noexcept;

    ServerPortRouDi(const ServerPortRouDi& other) = delete;
    ServerPortRouDi& operator=(const ServerPortRouDi&) = delete;
    ServerPortRouDi(ServerPortRouDi&& rhs) = default;
    ServerPortRouDi& operator=(ServerPortRouDi&& rhs) = default;
    ~ServerPortRouDi() = default;

    /// @brief get an optional CaPro message that changes the offer state of the server
    /// @return CaPro message with the new offer state, empty optional if no state change
    cxx::optional<capro::CaproMessage> tryGetCaProMessage() noexcept;

    /// @brief dispatch a CaPro message to the server for processing
    /// @param[in] caProMessage to process
    /// @return CaPro message with an immediate response the provided CaPro message, empty optional if no response
    cxx::optional<capro::CaproMessage>
    dispatchCaProMessageAndGetPossibleResponse(const capro::CaproMessage& caProMessage) noexcept;

    /// @brief cleanup the server and release all the chunks it currently holds
    /// Caution: Contract is that user process is no more running when cleanup is called
    void releaseAllChunks() noexcept;

  private:
    const MemberType_t* getMembers() const noexcept;
    MemberType_t* getMembers() noexcept;

    ChunkSender<ServerChunkSenderData_t> m_chunkSender;
    ChunkReceiver<ServerChunkReceiverData_t> m_chunkReceiver;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_SERVER_PORT_ROUDI_HPP
