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

#include "ioxdds/gateway/channel.hpp"

namespace iox
{
namespace dds
{

// Typedefs
template <typename IoxTerminal>
using IoxTerminalPool = iox::cxx::ObjectPool<IoxTerminal, MAX_CHANNEL_NUMBER>;
template <typename DDSTerminal>
using DDSTerminalPool = iox::cxx::ObjectPool<DDSTerminal, MAX_CHANNEL_NUMBER>;

// Statics
template <typename IoxTerminal, typename DDSTerminal>
IoxTerminalPool<IoxTerminal> Channel<IoxTerminal, DDSTerminal>::s_ioxTerminals = IoxTerminalPool();
template <typename IoxTerminal, typename DDSTerminal>
DDSTerminalPool<DDSTerminal> Channel<IoxTerminal, DDSTerminal>::s_ddsTerminals = DDSTerminalPool();

template <typename IoxTerminal, typename DDSTerminal>
inline Channel<IoxTerminal, DDSTerminal>::Channel(const iox::capro::ServiceDescription& service,
                                                  const IoxTerminalPtr ioxTerminal,
                                                  const DDSTerminalPtr ddsTerminal) noexcept
    : m_service(service),
      m_ioxTerminal(ioxTerminal),
      m_ddsTerminal(ddsTerminal)
{
}

template <typename IoxTerminal, typename DDSTerminal>
inline Channel<IoxTerminal, DDSTerminal>
Channel<IoxTerminal, DDSTerminal>::create(const iox::capro::ServiceDescription& service) noexcept
{
    // Create objects in the pool.
    auto rawIoxTerminalPtr = s_ioxTerminals.create(std::forward<const iox::capro::ServiceDescription>(service));
    auto rawDDSTerminalPtr = s_ddsTerminals.create(
        service.getServiceIDString(), service.getInstanceIDString(), service.getEventIDString());

    // Wrap in smart pointer with custom deleter to ensure automatic cleanup.
    auto ioxTerminalPtr = IoxTerminalPtr(rawIoxTerminalPtr, [](IoxTerminal* p) { s_ioxTerminals.free(p); });
    auto ddsTerminalPtr = DDSTerminalPtr(rawDDSTerminalPtr, [](DDSTerminal* p) { s_ddsTerminals.free(p); });

    return Channel(service, ioxTerminalPtr, ddsTerminalPtr);
}

template <typename IoxTerminal, typename DDSTerminal>
inline iox::capro::ServiceDescription Channel<IoxTerminal, DDSTerminal>::getService() const noexcept
{
    return m_service;
}

template <typename IoxTerminal, typename DDSTerminal>
inline std::shared_ptr<IoxTerminal> Channel<IoxTerminal, DDSTerminal>::getIceoryxTerminal() const noexcept
{
    return m_ioxTerminal;
}

template <typename IoxTerminal, typename DDSTerminal>
inline std::shared_ptr<DDSTerminal> Channel<IoxTerminal, DDSTerminal>::getDDSTerminal() const noexcept
{
    return m_ddsTerminal;
}

} // namespace dds
} // namespace iox
