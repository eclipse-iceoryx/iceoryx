// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_GW_CHANNEL_HPP
#define IOX_POSH_GW_CHANNEL_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iox/expected.hpp"
#include "iox/fixed_position_container.hpp"
#include "iox/optional.hpp"

#include <memory>

namespace iox
{
namespace gw
{
enum class ChannelError : uint8_t
{
    OBJECT_POOL_FULL
};

///
/// @class Channel
/// @brief A data structure representing a channel between Iceoryx and an external system.
///
/// The class couples related iceoryx and external interfaces that communicate with eachother to form the communication
/// channel.
/// These interfaces are conceptualized as channel "Terminals".
///
/// The structure holds pointers to the instances of the terminals.
/// The terminals can be created and managed externally, in which case the structure only serves as a means of coupling
/// the two.
/// This can be achieved by simply calling the constructor with pointers to them.
///
/// Alternatively, the class can manage the terminals internally in a static object pool, automatically
/// cleaning them up when the channel is discarded.
/// This can be achieved via the Channel::create method.
///
template <typename IceoryxTerminal, typename ExternalTerminal>
class Channel
{
    using IceoryxTerminalPtr = std::shared_ptr<IceoryxTerminal>;
    using IceoryxTerminalPool = FixedPositionContainer<IceoryxTerminal, MAX_CHANNEL_NUMBER>;
    using ExternalTerminalPtr = std::shared_ptr<ExternalTerminal>;
    using ExternalTerminalPool = FixedPositionContainer<ExternalTerminal, MAX_CHANNEL_NUMBER>;

  public:
    constexpr Channel(const capro::ServiceDescription& service,
                      const IceoryxTerminalPtr iceoryxTerminal,
                      const ExternalTerminalPtr externalTerminal) noexcept;

    constexpr bool operator==(const Channel<IceoryxTerminal, ExternalTerminal>& rhs) const noexcept;

    ///
    /// @brief create Creates a channel for the given service whose terminals reside in a static object pool.
    /// @param service The service to create the channel for.
    /// @param options The PublisherOptions or SubscriberOptions with historyCapacity and queueCapacity.
    /// @return A copy of the created channel, if successful.
    ///
    template <typename IceoryxPubSubOptions>
    static expected<Channel, ChannelError> create(const capro::ServiceDescription& service,
                                                  const IceoryxPubSubOptions& options) noexcept;

    capro::ServiceDescription getServiceDescription() const noexcept;
    IceoryxTerminalPtr getIceoryxTerminal() const noexcept;
    ExternalTerminalPtr getExternalTerminal() const noexcept;

  private:
    static IceoryxTerminalPool s_iceoryxTerminals;
    static ExternalTerminalPool s_externalTerminals;

    capro::ServiceDescription m_service;
    IceoryxTerminalPtr m_iceoryxTerminal;
    ExternalTerminalPtr m_externalTerminal;
};

} // namespace gw
} // namespace iox

#include "iceoryx_posh/internal/gateway/channel.inl"

#endif // IOX_POSH_GW_CHANNEL_HPP
