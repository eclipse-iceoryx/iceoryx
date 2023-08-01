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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_POSH_STUBS_GATEWAY_GENERIC_HPP
#define IOX_POSH_STUBS_GATEWAY_GENERIC_HPP

#include "iceoryx_posh/gateway/channel.hpp"
#include "iceoryx_posh/gateway/gateway_generic.hpp"

#include "mocks/gateway_base_mock.hpp"

namespace iox
{
namespace gw
{
template <typename channel_t>
using TestGatewayGeneric = iox::gw::GatewayGeneric<channel_t, MockGatewayBase>;

///
/// @brief The StubbedGatewayGeneric class stubs out the pure virtual methods and exposes the protected methods
/// to allow them to be tested.
/// Only to be used in testing.
///
template <typename channel_t>
class StubbedGatewayGeneric : public TestGatewayGeneric<channel_t>
{
  public:
    StubbedGatewayGeneric()
        : TestGatewayGeneric<channel_t>(iox::capro::Interfaces::INTERNAL){};

    void loadConfiguration(const config::GatewayConfig&) noexcept
    {
        // Stubbed.
    }

    void discover(const iox::capro::CaproMessage&) noexcept
    {
        // Stubbed.
    }

    void forward(const channel_t&) noexcept
    {
        // Stubbed.
    }

    template <typename IceoryxPubSubOptions>
    iox::expected<channel_t, iox::gw::GatewayError> addChannel(const iox::capro::ServiceDescription& service,
                                                               const IceoryxPubSubOptions& options) noexcept
    {
        return TestGatewayGeneric<channel_t>::addChannel(service, options);
    }

    iox::optional<channel_t> findChannel(const iox::capro::ServiceDescription& service) noexcept
    {
        return TestGatewayGeneric<channel_t>::findChannel(service);
    }

    void forEachChannel(const iox::function_ref<void(channel_t&)> f) noexcept
    {
        TestGatewayGeneric<channel_t>::forEachChannel(f);
    }

    iox::expected<void, iox::gw::GatewayError> discardChannel(const iox::capro::ServiceDescription& service) noexcept
    {
        return TestGatewayGeneric<channel_t>::discardChannel(service);
    }
};
} // namespace gw
} // namespace iox

#endif // IOX_POSH_STUBS_GATEWAY_GENERIC_HPP
