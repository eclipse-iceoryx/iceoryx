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

#ifndef IOX_eth_GATEWAY_IOX_TO_DDS_HPP
#define IOX_eth_GATEWAY_IOX_TO_DDS_HPP

#include "eth/eth_types.hpp"
#include "iceoryx_posh/gateway/channel.hpp"
#include "iceoryx_posh/gateway/gateway_generic.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"

namespace iox
{
namespace eth
{
///
/// @brief  Gateway implementation for the iceoryx to eth direction.
///
template <typename channel_t = iox::gw::Channel<iox::popo::Subscriber, iox::eth::data_writer_t>,
          typename gateway_t = iox::gw::GatewayGeneric<channel_t>>
class Iceoryx2ethGateway : public gateway_t
{
  public:
    Iceoryx2ethGateway() noexcept;
    void loadConfiguration(const iox::config::GatewayConfig& config) noexcept;
    void discover(const iox::capro::CaproMessage& msg) noexcept;
    void forward(const channel_t& channel) noexcept;
   ~Iceoryx2ethGateway() noexcept;
  private:
    iox::cxx::expected<channel_t, iox::gw::GatewayError>
    setupChannel(const iox::capro::ServiceDescription& service) noexcept;
};

} // namespace eth
} // namespace iox

#include "internal/gateway/iox_to_eth.inl"

#endif // IOX_eth_GATEWAY_IOX_TO_DDS_HPP
