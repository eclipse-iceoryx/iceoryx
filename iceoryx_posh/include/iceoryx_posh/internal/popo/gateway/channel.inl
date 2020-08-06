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

#ifndef IOX_POSH_GATEWAY_CHANNEL_INL
#define IOX_POSH_GATEWAY_CHANNEL_INL

#include "iceoryx_posh/popo/gateway/channel.hpp"

namespace iox
{
namespace popo
{
// Typedefs
template <typename IceoryxTerminal>
using IceoryxTerminalPool = iox::cxx::ObjectPool<IceoryxTerminal, MAX_CHANNEL_NUMBER>;
template <typename ExternalTerminal>
using ExternalTerminalPool = iox::cxx::ObjectPool<ExternalTerminal, MAX_CHANNEL_NUMBER>;

// Statics
template <typename IceoryxTerminal, typename ExternalTerminal>
IceoryxTerminalPool<IceoryxTerminal> Channel<IceoryxTerminal, ExternalTerminal>::s_iceoryxTerminals = IceoryxTerminalPool();
template <typename IceoryxTerminal, typename ExternalTerminal>
ExternalTerminalPool<ExternalTerminal> Channel<IceoryxTerminal, ExternalTerminal>::s_externalTerminals = ExternalTerminalPool();

template <typename IceoryxTerminal, typename ExternalTerminal>
inline Channel<IceoryxTerminal, ExternalTerminal>::Channel(const iox::capro::ServiceDescription& service,
                                                      const IceoryxTerminalPtr iceoryxTerminal,
                                                      const ExternalTerminalPtr externalTerminal) noexcept
    : m_service(service)
    , m_iceoryxTerminal(iceoryxTerminal)
    , m_externalTerminal(externalTerminal)
{
}

template <typename IceoryxTerminal, typename ExternalTerminal>
constexpr inline bool
Channel<IceoryxTerminal, ExternalTerminal>::operator==(const Channel<IceoryxTerminal, ExternalTerminal>& rhs) const noexcept
{
    return m_service == rhs.getService();
}

template <typename IceoryxTerminal, typename ExternalTerminal>
inline iox::cxx::expected<Channel<IceoryxTerminal, ExternalTerminal>, ChannelError>
Channel<IceoryxTerminal, ExternalTerminal>::create(const iox::capro::ServiceDescription& service) noexcept
{
    // Create objects in the pool.
    auto rawIceoryxTerminalPtr = s_iceoryxTerminals.create(std::forward<const iox::capro::ServiceDescription>(service));
    if (rawIceoryxTerminalPtr == nullptr)
    {
        return iox::cxx::error<ChannelError>(ChannelError::OBJECT_POOL_FULL);
    }
    auto rawExternalTerminalPtr =
        s_externalTerminals.create(service.getServiceIDString(), service.getInstanceIDString(), service.getEventIDString());
    if (rawExternalTerminalPtr == nullptr)
    {
        return iox::cxx::error<ChannelError>(ChannelError::OBJECT_POOL_FULL);
    }

    // Wrap in smart pointer with custom deleter to ensure automatic cleanup.
    auto iceoryxTerminalPtr =
        IceoryxTerminalPtr(rawIceoryxTerminalPtr, [](IceoryxTerminal* const p) { s_iceoryxTerminals.free(p); });
    auto externalTerminalPtr = ExternalTerminalPtr(rawExternalTerminalPtr, [](ExternalTerminal* const p) { s_externalTerminals.free(p); });

    return iox::cxx::success<Channel>(Channel(service, iceoryxTerminalPtr, externalTerminalPtr));
}

template <typename IceoryxTerminal, typename ExternalTerminal>
inline iox::capro::ServiceDescription Channel<IceoryxTerminal, ExternalTerminal>::getServiceDescription() const noexcept
{
    return m_service;
}

template <typename IceoryxTerminal, typename ExternalTerminal>
inline std::shared_ptr<IceoryxTerminal> Channel<IceoryxTerminal, ExternalTerminal>::getIceoryxTerminal() const noexcept
{
    return m_iceoryxTerminal;
}

template <typename IceoryxTerminal, typename ExternalTerminal>
inline std::shared_ptr<ExternalTerminal> Channel<IceoryxTerminal, ExternalTerminal>::getExternalTerminal() const noexcept
{
    return m_externalTerminal;
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_GATEWAY_CHANNEL_INL
