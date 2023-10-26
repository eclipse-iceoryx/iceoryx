// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/runtime/port_config_info.hpp"

namespace iox
{
namespace runtime
{
PortConfigInfo::PortConfigInfo(uint32_t portType, uint32_t deviceId, uint32_t memoryType) noexcept
    : portType(portType)
    , memoryInfo(deviceId, memoryType)
{
}

PortConfigInfo::PortConfigInfo(const Serialization& serialization) noexcept
{
    serialization.extract(portType, memoryInfo.deviceId, memoryInfo.memoryType);
}

PortConfigInfo::operator Serialization() const noexcept
{
    return Serialization::create(portType, memoryInfo.deviceId, memoryInfo.memoryType);
}

bool PortConfigInfo::operator==(const PortConfigInfo& rhs) const noexcept
{
    return portType == rhs.portType && memoryInfo == rhs.memoryInfo;
}
} // namespace runtime
} // namespace iox
