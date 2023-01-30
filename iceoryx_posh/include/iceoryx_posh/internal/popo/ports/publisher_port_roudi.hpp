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
#ifndef IOX_POSH_POPO_PORTS_PUBLISHER_PORT_ROUDI_HPP
#define IOX_POSH_POPO_PORTS_PUBLISHER_PORT_ROUDI_HPP

#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iox/not_null.hpp"
#include "iox/optional.hpp"

namespace iox
{
namespace popo
{
/// @brief The PublisherPortRouDi provides the API for accessing a publisher port from the RouDi middleware daemon side.
/// The publisher port is divided in the three parts PublisherPortData, PublisherPortRouDi and PublisherPortUser.
/// The PublisherPortRouDi provides service discovery functionality that is based on CaPro messages. With this API the
/// dynamic connections between publisher and subscriber ports can be established
class PublisherPortRouDi : public BasePort
{
  public:
    using MemberType_t = PublisherPortData;

    explicit PublisherPortRouDi(not_null<MemberType_t* const> publisherPortDataPtr) noexcept;

    PublisherPortRouDi(const PublisherPortRouDi& other) = delete;
    PublisherPortRouDi& operator=(const PublisherPortRouDi&) = delete;
    PublisherPortRouDi(PublisherPortRouDi&& rhs) noexcept = default;
    PublisherPortRouDi& operator=(PublisherPortRouDi&& rhs) noexcept = default;
    ~PublisherPortRouDi() = default;

    /// @brief Returns publisher options
    /// @return publisher options
    const PublisherOptions& getOptions() const noexcept;

    /// @brief get an optional CaPro message that changes the offer state of the publisher
    /// @return CaPro message with the new offer state, empty optional if no state change
    optional<capro::CaproMessage> tryGetCaProMessage() noexcept;

    /// @brief dispatch a CaPro message to the publisher for processing
    /// @param[in] caProMessage to process
    /// @return CaPro message with an immediate response the provided CaPro message, empty optional if no response
    optional<capro::CaproMessage>
    dispatchCaProMessageAndGetPossibleResponse(const capro::CaproMessage& caProMessage) noexcept;

    /// @brief cleanup the publisher and release all the chunks it currently holds
    /// Caution: Contract is that user process is no more running when cleanup is called
    void releaseAllChunks() noexcept;

  private:
    const MemberType_t* getMembers() const noexcept;
    MemberType_t* getMembers() noexcept;

    ChunkSender<PublisherPortData::ChunkSenderData_t> m_chunkSender;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_PUBLISHER_PORT_ROUDI_HPP
