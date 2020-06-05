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

#pragma once

#include <iceoryx_posh/popo/subscriber.hpp>

#include "iceoryx_dds/dds/dds_types.hpp"
#include "iceoryx_dds/gateway/dds_gateway_generic.hpp"
#include "iceoryx_dds/gateway/channel.hpp"

namespace iox {
namespace dds {

template <typename channel_t = iox::dds::Channel<iox::popo::Subscriber, iox::dds::data_writer_t>,
          typename gateway_t = iox::dds::DDSGatewayGeneric<channel_t>>
class Iceoryx2DDSGateway : public gateway_t
{
    using ChannelFactory = std::function<channel_t(const iox::capro::ServiceDescription)>;
public:
    Iceoryx2DDSGateway() noexcept;
    void loadConfiguration(GatewayConfig config) noexcept;
    void discover(const iox::capro::CaproMessage& msg) noexcept;
    void forward() noexcept;
};

} // namespace dds
} // namespace iox

#include "iceoryx_dds/internal/gateway/iox_to_dds.inl"
