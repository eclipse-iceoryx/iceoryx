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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/base_port.hpp"
#include "iceoryx_posh/internal/popo/interface_port_data.hpp"

namespace iox
{
namespace popo
{
class InterfacePort : public BasePort
{
  public:
    InterfacePort(InterfacePortData* const f_member);
    InterfacePort(const InterfacePort& other) = delete;
    InterfacePort& operator=(const InterfacePort& other) = delete;
    InterfacePort(InterfacePort&& other) = default;
    InterfacePort& operator=(InterfacePort&& other) = default;

    bool dispatchCaProMessage(const capro::CaproMessage& f_message);

    bool getCaProMessage(capro::CaproMessage& f_message);

  private:
    const InterfacePortData* getMembers() const;
    InterfacePortData* getMembers();
};

} // namespace popo
} // namespace iox

