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
#ifndef IOX_POSH_RUNTIME_NODE_PROPERTY_HPP
#define IOX_POSH_RUNTIME_NODE_PROPERTY_HPP

#include "iceoryx_dust/cxx/serialization.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

namespace iox
{
namespace runtime
{
/// @brief helper struct which is convertable to string and constructable from a string
///         which is required to send the createNode request over the IPC channel
struct NodeProperty
{
    /// @brief constructor
    /// @param[in] name name of the node
    /// @param[in] nodeDeviceIdentifier identifier of the device on which the node will run
    NodeProperty(const iox::NodeName_t& name, const uint64_t nodeDeviceIdentifier) noexcept;

    /// @brief serialization constructor, used by the IPC channel message to create NodeProperty
    ///         from a received message
    /// @param[in] serialized raw serialized string where all the values are stored
    NodeProperty(const cxx::Serialization& serialized) noexcept;

    /// @brief serialization of the node properties
    operator cxx::Serialization() const noexcept;

    iox::NodeName_t m_name;
    uint64_t m_nodeDeviceIdentifier;
};
} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_NODE_PROPERTY_HPP
