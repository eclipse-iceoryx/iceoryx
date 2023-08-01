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

#ifndef IOX_POSH_GW_GATEWAY_CONFIG_HPP
#define IOX_POSH_GW_GATEWAY_CONFIG_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iox/vector.hpp"

namespace iox
{
namespace config
{
///
/// @brief Generic configuration for gateways.
///
struct GatewayConfig
{
    struct ServiceEntry
    {
        capro::ServiceDescription m_serviceDescription;
    };
    iox::vector<ServiceEntry, MAX_GATEWAY_SERVICES> m_configuredServices;

    void setDefaults() noexcept;
};
} // namespace config
} // namespace iox

#endif // IOX_POSH_GW_GATEWAY_CONFIG_HPP
