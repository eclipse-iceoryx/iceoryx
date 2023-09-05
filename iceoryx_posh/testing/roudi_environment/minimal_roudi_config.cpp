// Copyright (c) 2023 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#include "iceoryx_posh/testing/roudi_environment/minimal_roudi_config.hpp"

namespace iox
{
namespace testing
{
RouDiConfig_t MinimalRouDiConfigBuilder::create() const noexcept
{
    RouDiConfig_t roudiConfig;
    mepoo::MePooConfig mepooConfig;
    mepooConfig.addMemPool({m_payloadChunkSize, m_payloadChunkCount});
    auto currentGroup = iox::posix::PosixGroup::getGroupOfCurrentProcess();
    roudiConfig.m_sharedMemorySegments.push_back({currentGroup.getName(), currentGroup.getName(), mepooConfig});
    roudiConfig.introspectionChunkCount = m_introspectionChunkCount;
    return roudiConfig;
}
} // namespace testing
} // namespace iox
