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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_POPO_PORTS_INTERFACE_PORT_HPP
#define IOX_POSH_POPO_PORTS_INTERFACE_PORT_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port_data.hpp"

namespace iox
{
namespace popo
{
class InterfacePort : public BasePort
{
  public:
    explicit InterfacePort(InterfacePortData* const interfacePortDataPtr) noexcept;

    InterfacePort(const InterfacePort& other) = delete;
    InterfacePort& operator=(const InterfacePort& other) = delete;
    InterfacePort(InterfacePort&& other) noexcept = default;
    InterfacePort& operator=(InterfacePort&& other) noexcept = default;
    ~InterfacePort() = default;

    /// @brief get an optional CaPro message for the interface port to process
    /// @return CaPro message, empty optional if no new messages
    optional<capro::CaproMessage> tryGetCaProMessage() noexcept;

    /// @brief dispatch a CaPro message to this interface port
    /// @param[in] caProMessage
    void dispatchCaProMessage(const capro::CaproMessage& caProMessage) noexcept;

  private:
    const InterfacePortData* getMembers() const noexcept;
    InterfacePortData* getMembers() noexcept;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_INTERFACE_PORT_HPP
