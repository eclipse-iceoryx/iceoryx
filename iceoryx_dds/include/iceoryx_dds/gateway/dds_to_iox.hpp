// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_DDS_DDS_TO_IOX_HPP
#define IOX_DDS_DDS_TO_IOX_HPP

#include "iceoryx_dds/dds/dds_types.hpp"
#include "iceoryx_posh/gateway/channel.hpp"
#include "iceoryx_posh/gateway/gateway_config.hpp"
#include "iceoryx_posh/gateway/gateway_generic.hpp"
#include "iceoryx_posh/popo/untyped_publisher.hpp"

namespace iox
{
namespace dds
{
/// @brief DDS Gateway implementation for the DDS to iceoryx direction.
template <typename channel_t = gw::Channel<popo::UntypedPublisher, dds::data_reader_t>,
          typename gateway_t = gw::GatewayGeneric<channel_t>>
class DDS2IceoryxGateway : public gateway_t
{
  public:
    /// @brief Creates a gateway with DDS set as interface
    DDS2IceoryxGateway() noexcept;

    void loadConfiguration(const config::GatewayConfig& config) noexcept;
    void discover(const capro::CaproMessage& msg) noexcept;
    void forward(const channel_t& channel) noexcept;

  private:
    void* m_reservedChunk = nullptr;

    cxx::expected<channel_t, gw::GatewayError> setupChannel(const capro::ServiceDescription& service,
                                                            const popo::PublisherOptions& publisherOptions) noexcept;
};

} // namespace dds
} // namespace iox

#include "iceoryx_dds/internal/gateway/dds_to_iox.inl"

#endif // IOX_DDS_DDS_TO_IOX_HPP
