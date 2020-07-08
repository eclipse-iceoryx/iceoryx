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

#ifndef IOX_DDS_GATEWAY_CHANNEL_HPP
#define IOX_DDS_GATEWAY_CHANNEL_HPP

#include "iceoryx_dds/dds/dds_config.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/objectpool/objectpool.hpp"

#include <memory>

namespace iox
{
namespace dds
{
enum class ChannelError : uint8_t
{
    OBJECT_POOL_FULL
};

///
/// @class Channel
/// @brief A data structure representing a channel between Iceoryx and DDS.
///
/// The class couples related iceoryx and dds entities that communicate with eachother to form the communication
/// channel.
/// For example: An Iceoryx subscriber and its corresponding DDS data writer, which communicate eachother to form
///              an outbound communication channel.
/// These entites are conceptualized as channel "Terminals".
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
template <typename IceoryxTerminal, typename DDSTerminal>
class Channel
{
    using IceoryxTerminalPtr = std::shared_ptr<IceoryxTerminal>;
    using IceoryxTerminalPool = iox::cxx::ObjectPool<IceoryxTerminal, MAX_CHANNEL_NUMBER>;
    using DDSTerminalPtr = std::shared_ptr<DDSTerminal>;
    using DDSTerminalPool = iox::cxx::ObjectPool<DDSTerminal, MAX_CHANNEL_NUMBER>;

  public:
    Channel(const iox::capro::ServiceDescription& service,
            const IceoryxTerminalPtr iceoryxTerminal,
            const DDSTerminalPtr ddsTerminal) noexcept;

    constexpr bool operator==(const Channel<IceoryxTerminal, DDSTerminal>& rhs) const noexcept;

    ///
    /// @brief create Creates a channel for the given service whose terminals reside in a static object pool.
    /// @param service The service to create the channel for.
    /// @return A copy of the created channel, if successful.
    ///
    static iox::cxx::expected<Channel, ChannelError> create(const iox::capro::ServiceDescription& service) noexcept;

    iox::capro::ServiceDescription getServiceDescription() const noexcept;
    IceoryxTerminalPtr getIceoryxTerminal() const noexcept;
    DDSTerminalPtr getDDSTerminal() const noexcept;

  private:
    static IceoryxTerminalPool s_iceoryxTerminals;
    static DDSTerminalPool s_ddsTerminals;

    iox::capro::ServiceDescription m_service;
    IceoryxTerminalPtr m_iceoryxTerminal;
    DDSTerminalPtr m_ddsTerminal;
};

} // namespace dds
} // namespace iox

#include "iceoryx_dds/internal/gateway/channel.inl"

#endif // IOX_DDS_DDS_DDS_TYPES_HPP
