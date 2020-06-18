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

#include "iceoryx_dds/gateway/channel.hpp"
#include "iceoryx_dds/gateway/dds_gateway_generic.hpp"

#include "mocks/google_mocks.hpp"

namespace iox
{
namespace dds
{
template <typename channel_t>
using TestDDSGatewayGeneric = iox::dds::DDSGatewayGeneric<channel_t, MockGenericGateway>;

///
/// @brief The StubbedDDSGatewayGeneric class stubs out the pure virtual methods and exposes the protected methods
/// to allow them to be tested.
/// Only to be used in testing.
///
template <typename channel_t>
class StubbedDDSGatewayGeneric : public TestDDSGatewayGeneric<channel_t>
{
  public:
    void loadConfiguration(const GatewayConfig& config) noexcept
    {
        // Stubbed.
    }

    void discover(const iox::capro::CaproMessage& msg) noexcept
    {
        // Stubbed.
    }

    void forward(const channel_t& channel) noexcept
    {
        // Stubbed.
    }

    iox::cxx::expected<channel_t, uint8_t> addChannel(const iox::capro::ServiceDescription& service) noexcept
    {
        return TestDDSGatewayGeneric<channel_t>::addChannel(service);
    }

    iox::cxx::optional<channel_t> findChannel(const iox::capro::ServiceDescription& service) noexcept
    {
        return TestDDSGatewayGeneric<channel_t>::findChannel(service);
    }

    void forEachChannel(const iox::cxx::function_ref<void(channel_t&)> f) noexcept
    {
        TestDDSGatewayGeneric<channel_t>::forEachChannel(f);
    }

    void discardChannel(const iox::capro::ServiceDescription& service) noexcept
    {
        TestDDSGatewayGeneric<channel_t>::discardChannel(service);
    }
};
}
}
