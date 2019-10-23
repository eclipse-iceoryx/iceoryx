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

#include "iceoryx_posh/internal/popo/application_port_data.hpp"

namespace iox
{
namespace popo
{
ApplicationPortData::ApplicationPortData(const std::string& f_applicationName, Interfaces f_interface)
    : BasePortData(capro::ServiceDescription(),
                   BasePortType::APPLICATION_PORT,
                   f_applicationName,
                   f_interface,
                   nullptr)
{
}
} // namespace popo
} // namespace iox
