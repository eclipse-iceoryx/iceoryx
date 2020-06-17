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

namespace iox
{
namespace dds
{
// Typedefs
template <typename IceoryxTerminal>
using IceoryxTerminalPool = iox::cxx::ObjectPool<IceoryxTerminal, MAX_CHANNEL_NUMBER>;
template <typename DDSTerminal>
using DDSTerminalPool = iox::cxx::ObjectPool<DDSTerminal, MAX_CHANNEL_NUMBER>;

// Statics
template <typename IceoryxTerminal, typename DDSTerminal>
IceoryxTerminalPool<IceoryxTerminal> Channel<IceoryxTerminal, DDSTerminal>::s_iceoryxTerminals = IceoryxTerminalPool();
template <typename IceoryxTerminal, typename DDSTerminal>
DDSTerminalPool<DDSTerminal> Channel<IceoryxTerminal, DDSTerminal>::s_ddsTerminals = DDSTerminalPool();

template <typename IceoryxTerminal, typename DDSTerminal>
inline Channel<IceoryxTerminal, DDSTerminal>::Channel(const iox::capro::ServiceDescription& service,
                                                      const IceoryxTerminalPtr iceoryxTerminal,
                                                      const DDSTerminalPtr ddsTerminal) noexcept
    : m_service(service),
      m_iceoryxTerminal(iceoryxTerminal),
      m_ddsTerminal(ddsTerminal)
{
}

template <typename IceoryxTerminal, typename DDSTerminal>
constexpr inline bool Channel<IceoryxTerminal, DDSTerminal>::operator==(const Channel<IceoryxTerminal, DDSTerminal>& rhs) const noexcept
{
    return m_service == rhs.getService();
}

template <typename IceoryxTerminal, typename DDSTerminal>
inline Channel<IceoryxTerminal, DDSTerminal>
Channel<IceoryxTerminal, DDSTerminal>::create(const iox::capro::ServiceDescription& service) noexcept
{
    // Create objects in the pool.
    auto rawIceoryxTerminalPtr = s_iceoryxTerminals.create(std::forward<const iox::capro::ServiceDescription>(service));
    auto rawDDSTerminalPtr =
        s_ddsTerminals.create(service.getServiceIDString(), service.getInstanceIDString(), service.getEventIDString());

    // Wrap in smart pointer with custom deleter to ensure automatic cleanup.
    auto iceoryxTerminalPtr =
        IceoryxTerminalPtr(rawIceoryxTerminalPtr, [](IceoryxTerminal* const p) { s_iceoryxTerminals.free(p); });
    auto ddsTerminalPtr = DDSTerminalPtr(rawDDSTerminalPtr, [](DDSTerminal* const p) { s_ddsTerminals.free(p); });

    return Channel(service, iceoryxTerminalPtr, ddsTerminalPtr);
}

template <typename IceoryxTerminal, typename DDSTerminal>
inline iox::capro::ServiceDescription Channel<IceoryxTerminal, DDSTerminal>::getService() const noexcept
{
    return m_service;
}

template <typename IceoryxTerminal, typename DDSTerminal>
inline std::shared_ptr<IceoryxTerminal> Channel<IceoryxTerminal, DDSTerminal>::getIceoryxTerminal() const noexcept
{
    return m_iceoryxTerminal;
}

template <typename IceoryxTerminal, typename DDSTerminal>
inline std::shared_ptr<DDSTerminal> Channel<IceoryxTerminal, DDSTerminal>::getDDSTerminal() const noexcept
{
    return m_ddsTerminal;
}

} // namespace dds
} // namespace iox
