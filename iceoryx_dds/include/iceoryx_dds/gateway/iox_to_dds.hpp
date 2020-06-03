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

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include <iceoryx_posh/iceoryx_posh_types.hpp>
#include <iceoryx_posh/popo/subscriber.hpp>
#include <iceoryx_posh/runtime/posh_runtime.hpp>

#include "iceoryx_dds/dds/data_writer.hpp"
#include "iceoryx_dds/dds/dds_configs.hpp"
#include "iceoryx_dds/dds/dds_types.hpp"
#include "iceoryx_dds/gateway/dds_gateway_generic.hpp"
#include "iceoryx_dds/gateway/channel.hpp"

namespace iox
{
namespace dds
{
///
/// @brief A Gateway to support internode communication between iceoryx nodes in a DDS network.
///
/// Forwards data published in a local posh runtime to an attached DDS network.
///
template <typename channel_t = iox::dds::Channel<iox::popo::Subscriber, iox::dds::data_writer_t>>
class Iceoryx2DDSGateway : public iox::dds::DDSGatewayGeneric<channel_t>
{
  public:
    Iceoryx2DDSGateway() noexcept;
    void discover(const iox::capro::CaproMessage& msg) noexcept;
    void forward() noexcept;
};

} // dds
} // iox

#include "iceoryx_dds/internal/gateway/iox_to_dds.inl"
