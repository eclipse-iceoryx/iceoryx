// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_GW_CHANNEL_INL
#define IOX_POSH_GW_CHANNEL_INL

#include "iceoryx_posh/gateway/channel.hpp"

namespace iox
{
namespace gw
{
// Typedefs
template <typename IceoryxTerminal>
using IceoryxTerminalPool = FixedPositionContainer<IceoryxTerminal, MAX_CHANNEL_NUMBER>;
template <typename ExternalTerminal>
using ExternalTerminalPool = FixedPositionContainer<ExternalTerminal, MAX_CHANNEL_NUMBER>;

// Statics
template <typename IceoryxTerminal, typename ExternalTerminal>
IceoryxTerminalPool<IceoryxTerminal> Channel<IceoryxTerminal, ExternalTerminal>::s_iceoryxTerminals{};
template <typename IceoryxTerminal, typename ExternalTerminal>
ExternalTerminalPool<ExternalTerminal> Channel<IceoryxTerminal, ExternalTerminal>::s_externalTerminals{};

template <typename IceoryxTerminal, typename ExternalTerminal>
inline constexpr Channel<IceoryxTerminal, ExternalTerminal>::Channel(
    const capro::ServiceDescription& service,
    const IceoryxTerminalPtr iceoryxTerminal,
    const ExternalTerminalPtr externalTerminal) noexcept
    : m_service(service)
    , m_iceoryxTerminal(iceoryxTerminal)
    , m_externalTerminal(externalTerminal)
{
}

template <typename IceoryxTerminal, typename ExternalTerminal>
constexpr inline bool Channel<IceoryxTerminal, ExternalTerminal>::operator==(
    const Channel<IceoryxTerminal, ExternalTerminal>& rhs) const noexcept
{
    return m_service == rhs.getService();
}

template <typename IceoryxTerminal, typename ExternalTerminal>
template <typename IceoryxPubSubOptions>
inline expected<Channel<IceoryxTerminal, ExternalTerminal>, ChannelError>
Channel<IceoryxTerminal, ExternalTerminal>::create(const capro::ServiceDescription& service,
                                                   const IceoryxPubSubOptions& options) noexcept
{
    // Create objects in the pool.
    auto rawIceoryxTerminal = s_iceoryxTerminals.emplace(std::forward<const capro::ServiceDescription&>(service),
                                                         std::forward<const IceoryxPubSubOptions&>(options));
    if (rawIceoryxTerminal == s_iceoryxTerminals.end())
    {
        return err(ChannelError::OBJECT_POOL_FULL);
    }
    auto rawExternalTerminal = s_externalTerminals.emplace(
        service.getServiceIDString(), service.getInstanceIDString(), service.getEventIDString());
    if (rawExternalTerminal == s_externalTerminals.end())
    {
        return err(ChannelError::OBJECT_POOL_FULL);
    }

    // Wrap in smart pointer with custom deleter to ensure automatic cleanup.
    auto iceoryxTerminalPtr = IceoryxTerminalPtr(
        rawIceoryxTerminal.to_ptr(), [](IceoryxTerminal* const p) { Channel::s_iceoryxTerminals.erase(p); });
    auto externalTerminalPtr = ExternalTerminalPtr(
        rawExternalTerminal.to_ptr(), [](ExternalTerminal* const p) { Channel::s_externalTerminals.erase(p); });

    return ok(Channel(service, iceoryxTerminalPtr, externalTerminalPtr));
}

template <typename IceoryxTerminal, typename ExternalTerminal>
inline capro::ServiceDescription Channel<IceoryxTerminal, ExternalTerminal>::getServiceDescription() const noexcept
{
    return m_service;
}

template <typename IceoryxTerminal, typename ExternalTerminal>
inline std::shared_ptr<IceoryxTerminal> Channel<IceoryxTerminal, ExternalTerminal>::getIceoryxTerminal() const noexcept
{
    return m_iceoryxTerminal;
}

template <typename IceoryxTerminal, typename ExternalTerminal>
inline std::shared_ptr<ExternalTerminal>
Channel<IceoryxTerminal, ExternalTerminal>::getExternalTerminal() const noexcept
{
    return m_externalTerminal;
}

} // namespace gw
} // namespace iox

#endif // IOX_POSH_GW_CHANNEL_INL
