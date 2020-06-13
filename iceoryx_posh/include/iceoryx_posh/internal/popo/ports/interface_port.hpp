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

#ifndef IOX_POPO_INTERFACE_PORT_HPP_
#define IOX_POPO_INTERFACE_PORT_HPP_

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
    InterfacePort(InterfacePort&& other) = default;
    InterfacePort& operator=(InterfacePort&& other) = default;
    ~InterfacePort() = default;

    bool dispatchCaProMessage(const capro::CaproMessage& message) noexcept;

    bool getCaProMessage(capro::CaproMessage& message) noexcept;

  private:
    const InterfacePortData* getMembers() const noexcept;
    InterfacePortData* getMembers() noexcept;
};

} // namespace popo
} // namespace iox

#endif
