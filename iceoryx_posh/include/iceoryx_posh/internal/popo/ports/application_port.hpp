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
#ifndef IOX_POSH_POPO_PORTS_APPLICATION_PORT_HPP
#define IOX_POSH_POPO_PORTS_APPLICATION_PORT_HPP

#include "iceoryx_posh/internal/popo/ports/application_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port.hpp"

namespace iox
{
namespace popo
{
class ApplicationPort : public BasePort
{
  public:
    using MemberType_t = ApplicationPortData;

    explicit ApplicationPort(ApplicationPortData* const applicationPortDataPtr) noexcept;

    ApplicationPort(const ApplicationPort& other) = delete;
    ApplicationPort& operator=(const ApplicationPort& other) = delete;
    ApplicationPort(ApplicationPort&& other) = default;
    ApplicationPort& operator=(ApplicationPort&& other) = default;
    ~ApplicationPort() = default;


    /// @brief get an optional CaPro message that was provided by the application
    /// @return CaPro message, empty optional if no new messages
    cxx::optional<capro::CaproMessage> tryGetCaProMessage() noexcept;

    /// @brief dispatch a CaPro message from the application side to the daemon
    /// @param[in] caProMessage
    void dispatchCaProMessage(const capro::CaproMessage& caProMessage) noexcept;

  private:
    const MemberType_t* getMembers() const noexcept;
    MemberType_t* getMembers() noexcept;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_APPLICATION_PORT_HPP
