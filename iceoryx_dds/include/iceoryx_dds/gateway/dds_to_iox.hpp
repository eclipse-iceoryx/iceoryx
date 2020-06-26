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

#ifndef IOX_DDS_GATEWAY_DDS_TO_IOX_HPP
#define IOX_DDS_GATEWAY_DDS_TO_IOX_HPP

#include "iceoryx_dds/dds/dds_types.hpp"
#include "iceoryx_dds/gateway/channel.hpp"
#include "iceoryx_dds/gateway/dds_gateway_generic.hpp"
#include "iceoryx_posh/popo/publisher.hpp"

namespace iox
{
namespace dds
{
///
/// @brief DDS Gateway implementation for the DDS to iceoryx direction.
///
template <typename channel_t = iox::dds::Channel<iox::popo::Publisher, iox::dds::data_reader_t>,
          typename gateway_t = iox::dds::DDSGatewayGeneric<channel_t>>
class DDS2IceoryxGateway : public iox::dds::DDSGatewayGeneric<channel_t>
{
    using ChannelFactory = std::function<channel_t(const iox::capro::ServiceDescription)>;

  public:
    DDS2IceoryxGateway() noexcept;
    DDS2IceoryxGateway(ChannelFactory channelFactory) noexcept;
    void loadConfiguration(const GatewayConfig& config) noexcept;
    void discover(const iox::capro::CaproMessage& msg) noexcept;
    void forward(const channel_t& channel) noexcept;

private:
    void* m_reservedChunk = nullptr;

    void setupChannel(const iox::capro::ServiceDescription& service) noexcept;
};

} // namespace dds
} // namespace iox

#include "iceoryx_dds/internal/gateway/dds_to_iox.inl"

#endif // IOX_DDS_GATEWAY_DDS_TO_IOX_HPP
