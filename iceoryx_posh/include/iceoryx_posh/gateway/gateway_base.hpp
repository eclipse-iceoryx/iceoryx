// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_GW_GATEWAY_BASE_HPP
#define IOX_POSH_GW_GATEWAY_BASE_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"

#include <memory>

namespace iox
{
namespace capro
{
class CaproMessage;
}

namespace popo
{
class InterfacePort;
}

namespace gw
{
/// @brief Generic gateway for communication events
class GatewayBase
{
  public:
    using CaproMessage = capro::CaproMessage;

    /// @brief Constructor for creating generic gateway based on type of interface
    /// @param[in] f_interface Type of interface
    GatewayBase(const capro::Interfaces f_interface) noexcept;

    GatewayBase& operator=(const GatewayBase& other) = delete;
    GatewayBase(const GatewayBase& other) = delete;
    GatewayBase(GatewayBase&& other) noexcept = default;
    GatewayBase& operator=(GatewayBase&&) noexcept = default;

    virtual ~GatewayBase() noexcept;
    /// @brief Get function for type of capro message - service or event or field
    /// @param[in] msg Type of caro message
    bool getCaProMessage(CaproMessage& msg) noexcept;

  protected:
    // Needed for unit testing
    GatewayBase() noexcept = default;
    capro::Interfaces getInterface() const noexcept;

  protected:
    popo::InterfacePort m_interfaceImpl{nullptr};
};

} // namespace gw
} // namespace iox

#endif // IOX_POSH_GW_GATEWAY_BASE_HPP
