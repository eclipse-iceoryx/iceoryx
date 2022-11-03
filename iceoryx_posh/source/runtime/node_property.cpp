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

#include "iceoryx_posh/internal/runtime/node_property.hpp"

#include "iceoryx_dust/cxx/serialization.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"

namespace iox
{
namespace runtime
{
NodeProperty::NodeProperty(const iox::NodeName_t& name, const uint64_t nodeDeviceIdentifier) noexcept
    : m_name(name)
    , m_nodeDeviceIdentifier(nodeDeviceIdentifier)
{
}

NodeProperty::NodeProperty(const cxx::Serialization& serialized) noexcept
{
    if (!serialized.extract(m_name, m_nodeDeviceIdentifier))
    {
        LogError() << "unable to create NodeProperty from serialized string " << serialized.toString();
    }
}

NodeProperty::operator cxx::Serialization() const noexcept
{
    return cxx::Serialization::create(m_name, m_nodeDeviceIdentifier);
}
} // namespace runtime
} // namespace iox
