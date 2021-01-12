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

#include "iceoryx_posh/internal/runtime/node_data.hpp"

namespace iox
{
namespace runtime
{
NodeData::NodeData(const ProcessName_t& process, const NodeName_t& node, const uint64_t nodeDeviceIdentifier) noexcept
    : m_process(process)
    , m_node(node)
    , m_nodeDeviceIdentifier(nodeDeviceIdentifier)
{
}
} // namespace runtime
} // namespace iox
