// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/interface_port.hpp"

#include <memory>

namespace iox
{
namespace capro
{
struct CaproMessage;
}
namespace popo
{
class InterfacePort;

/// @brief Generic gateway for communication events
class GatewayGeneric
{
  public:
    using CaproMessage = capro::CaproMessage;

    /// @brief Constructor for creating generic gateway based on type of interface
    /// @param[in] f_interface Type of interface
    GatewayGeneric(const Interfaces f_interface) noexcept;

    GatewayGeneric& operator=(const GatewayGeneric& other) = delete;
    GatewayGeneric(const GatewayGeneric& other) = delete;

    /// @brief Get function for type of capro message - service or event or field
    /// @param[in] msg Type of caro message
    bool getCaProMessage(CaproMessage& msg) noexcept;

  protected:
    // needed for unit testing
    GatewayGeneric() noexcept
    {
    }

  protected:
    InterfacePort m_interfaceImpl{nullptr};
};

} // namespace popo
} // namespace iox
