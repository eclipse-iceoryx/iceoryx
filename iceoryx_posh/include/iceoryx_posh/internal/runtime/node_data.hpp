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
#ifndef IOX_POSH_RUNTIME_NODE_DATA_HPP
#define IOX_POSH_RUNTIME_NODE_DATA_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"

#include <atomic>

namespace iox
{
namespace runtime
{
/// @brief struct which contains all the members of an object of type Node
class NodeData
{
  public:
    /// @brief constructor
    /// @param[in] name name of the node
    /// @param[in] nodeDeviceIdentifier identifier of the device on which the node will run
    NodeData(const ProcessName_t& process, const NodeName_t& node, const uint64_t nodeDeviceIdentifier) noexcept;

    NodeData(const NodeData&) = delete;
    NodeData(NodeData&&) = delete;
    NodeData& operator=(const NodeData&) = delete;
    NodeData& operator=(NodeData&&) = delete;


    ProcessName_t m_process;
    NodeName_t m_node;
    uint64_t m_nodeDeviceIdentifier;
    std::atomic_bool m_toBeDestroyed{false};
};
} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_NODE_DATA_HPP
