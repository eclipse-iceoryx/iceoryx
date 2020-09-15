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

#ifndef IOX_eth_GATEWAY_eth_TO_IOX_HPP
#define IOX_eth_GATEWAY_eth_TO_IOX_HPP

#include "iceoryx_eth/eth/eth_types.hpp"
#include "iceoryx_posh/gateway/channel.hpp"
#include "iceoryx_posh/gateway/gateway_generic.hpp"
#include "iceoryx_posh/gateway/gateway_config.hpp"
#include "iceoryx_posh/popo/publisher.hpp"

namespace iox
{
namespace eth
{
///
/// @brief eth Gateway implementation for the eth to iceoryx direction.
///
template <typename channel_t = iox::gw::Channel<iox::popo::Publisher, iox::eth::data_reader_t>,
          typename gateway_t = iox::gw::GatewayGeneric<channel_t>>
class eth2IceoryxGateway : public gateway_t
{
    using ChannelFactory = std::function<channel_t(const iox::capro::ServiceDescription)>;

  public:
    eth2IceoryxGateway() noexcept;
    eth2IceoryxGateway(ChannelFactory channelFactory) noexcept;
    void loadConfiguration(const iox::config::GatewayConfig& config) noexcept;
    void discover(const iox::capro::CaproMessage& msg) noexcept;
    void forward(const channel_t& channel) noexcept;
    void loadConfigurationEth(const std::vector<iox::popo::EthGatewayConf>& config) noexcept;
    void forwardLocal() noexcept;
    ~eth2IceoryxGateway() noexcept;
  private:
    void* m_reservedChunk = nullptr;
    iox::cxx::expected<channel_t, iox::gw::GatewayError>
    setupChannel(const iox::capro::ServiceDescription& service) noexcept;
};

} // namespace eth
} // namespace iox

#include "iceoryx_eth/internal/gateway/eth_to_iox.inl"

#endif // IOX_eth_GATEWAY_eth_TO_IOX_HPP
