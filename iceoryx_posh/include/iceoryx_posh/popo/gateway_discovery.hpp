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

#include "gateway_generic.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

namespace iox
{
namespace capro
{
struct CaproMessage;
}
namespace popo
{
class InterfacePort;

/// @brief Discover the gateway
template <typename Impl_T = GatewayGeneric>
class GatewayDiscovery
{
  public:
    using CaproMessage = capro::CaproMessage;

    /// @brief Constructor for discovering gateway based on type of interface
    /// @param[in] f_interface Type of interface
    GatewayDiscovery(const Interfaces f_interface) noexcept
        : m_impl(f_interface)
    {
    }

    /// @brief Get function for type of capro message - service or event or field
    /// @param[in] msg Type of capro message
    bool getCaproMessage(CaproMessage& msg) noexcept
    {
        return m_impl.getCaProMessage(msg);
    }

  protected:
    // needed for unit testing
    GatewayDiscovery(Impl_T interfacePortImpl) noexcept
        : m_impl(interfacePortImpl)
    {
    }

  private:
    Impl_T m_impl;
};
} // namespace popo
} // namespace iox
