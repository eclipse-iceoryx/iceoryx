// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"

namespace iox
{
namespace roudi_env
{
IceoryxConfig MinimalIceoryxConfigBuilder::create() const noexcept
{
    IceoryxConfig config;
    mepoo::MePooConfig mepooConfig;
    mepooConfig.addMemPool({m_payloadChunkSize, m_payloadChunkCount});
    auto currentGroup = PosixGroup::getGroupOfCurrentProcess();
    config.m_sharedMemorySegments.push_back({currentGroup.getName(), currentGroup.getName(), mepooConfig});

    config.introspectionChunkCount = m_introspectionChunkCount;
    config.discoveryChunkCount = m_discoveryChunkCount;

    return config;
}
} // namespace roudi_env
} // namespace iox
