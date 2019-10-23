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

#include "iceoryx_posh/internal/popo/base_port.hpp"
#include "iceoryx_posh/internal/popo/application_port_data.hpp"

namespace iox
{
namespace popo
{
class ApplicationPort : public BasePort
{
  public:
    using MemberType_t = ApplicationPortData;

    ApplicationPort(ApplicationPortData* const f_memberPtr);
    ApplicationPort(const ApplicationPort& other) = delete;
    ApplicationPort& operator=(const ApplicationPort& other) = delete;

    ApplicationPort(ApplicationPort&& other) = default;
    ApplicationPort& operator=(ApplicationPort&& other) = default;

    bool dispatchCaProMessage(const capro::CaproMessage& f_message);
    bool getCaProMessage(capro::CaproMessage& f_message);

  private:
    const MemberType_t* getMembers() const;
    MemberType_t* getMembers();
};

} // namespace popo
} // namespace iox


