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

#ifndef IOX_DDS_INTERNAL_GATEWAY_CHANNEL_INL
#define IOX_DDS_INTERNAL_GATEWAY_CHANNEL_INL

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
    : m_service(service)
    , m_iceoryxTerminal(iceoryxTerminal)
    , m_ddsTerminal(ddsTerminal)
    , m_dataSize(0u)
{
}


template <typename IceoryxTerminal, typename DDSTerminal>
inline Channel<IceoryxTerminal, DDSTerminal>::Channel(const iox::capro::ServiceDescription& service,
                                                      const IceoryxTerminalPtr iceoryxTerminal,
                                                      const DDSTerminalPtr ddsTerminal,
                                                      const uint64_t& dataSize) noexcept
    : m_service(service)
    , m_iceoryxTerminal(iceoryxTerminal)
    , m_ddsTerminal(ddsTerminal)
    , m_dataSize(dataSize)
{
}

template <typename IceoryxTerminal, typename DDSTerminal>
constexpr inline bool
Channel<IceoryxTerminal, DDSTerminal>::operator==(const Channel<IceoryxTerminal, DDSTerminal>& rhs) const noexcept
{
    return m_service == rhs.getService();
}

template <typename IceoryxTerminal, typename DDSTerminal>
inline iox::cxx::expected<Channel<IceoryxTerminal, DDSTerminal>, ChannelError>
Channel<IceoryxTerminal, DDSTerminal>::create(const iox::capro::ServiceDescription& service) noexcept
{
    return create(service, 0u);
}

template <typename IceoryxTerminal, typename DDSTerminal>
inline iox::cxx::expected<Channel<IceoryxTerminal, DDSTerminal>, ChannelError>
Channel<IceoryxTerminal, DDSTerminal>::create(const iox::capro::ServiceDescription& service,
                                              const uint64_t& dataSize) noexcept
{
    // Create objects in the pool.
    auto rawIceoryxTerminalPtr = s_iceoryxTerminals.create(std::forward<const iox::capro::ServiceDescription>(service));
    if (rawIceoryxTerminalPtr == nullptr)
    {
        return iox::cxx::error<ChannelError>(ChannelError::OBJECT_POOL_FULL);
    }
    auto rawDDSTerminalPtr =
        s_ddsTerminals.create(service.getServiceIDString(), service.getInstanceIDString(), service.getEventIDString());
    if (rawDDSTerminalPtr == nullptr)
    {
        return iox::cxx::error<ChannelError>(ChannelError::OBJECT_POOL_FULL);
    }

    // Wrap in smart pointer with custom deleter to ensure automatic cleanup.
    auto iceoryxTerminalPtr =
        IceoryxTerminalPtr(rawIceoryxTerminalPtr, [](IceoryxTerminal* const p) { s_iceoryxTerminals.free(p); });
    auto ddsTerminalPtr = DDSTerminalPtr(rawDDSTerminalPtr, [](DDSTerminal* const p) { s_ddsTerminals.free(p); });

    return iox::cxx::success<Channel>(Channel(service, iceoryxTerminalPtr, ddsTerminalPtr, dataSize));
}

template <typename IceoryxTerminal, typename DDSTerminal>
inline iox::capro::ServiceDescription Channel<IceoryxTerminal, DDSTerminal>::getServiceDescription() const noexcept
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

template <typename IceoryxTerminal, typename DDSTerminal>
inline iox::cxx::optional<uint64_t> Channel<IceoryxTerminal, DDSTerminal>::getDataSize() const noexcept
{
    if (m_dataSize > 0)
    {
        return iox::cxx::make_optional<uint64_t>(m_dataSize);
    }
    else
    {
        return iox::cxx::nullopt_t();
    }
}

} // namespace dds
} // namespace iox

#endif
