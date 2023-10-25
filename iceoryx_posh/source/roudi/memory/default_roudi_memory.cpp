// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/roudi/memory/default_roudi_memory.hpp"
#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/internal/roudi/service_registry.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iox/memory.hpp"

namespace iox
{
namespace roudi
{
DefaultRouDiMemory::DefaultRouDiMemory(const RouDiConfig_t& roudiConfig) noexcept
    : m_introspectionMemPoolBlock(introspectionMemPoolConfig(roudiConfig.introspectionChunkCount))
    , m_discoveryMemPoolBlock(discoveryMemPoolConfig(roudiConfig.discoveryChunkCount))
    , m_segmentManagerBlock(roudiConfig)
    , m_managementShm(SHM_NAME, posix::AccessMode::READ_WRITE, posix::OpenMode::PURGE_AND_CREATE)
{
    m_managementShm.addMemoryBlock(&m_introspectionMemPoolBlock).or_else([](auto) {
        errorHandler(PoshError::ROUDI__DEFAULT_ROUDI_MEMORY_FAILED_TO_ADD_INTROSPECTION_MEMORY_BLOCK,
                     ErrorLevel::FATAL);
    });
    m_managementShm.addMemoryBlock(&m_discoveryMemPoolBlock).or_else([](auto) {
        errorHandler(PoshError::ROUDI__DEFAULT_ROUDI_MEMORY_FAILED_TO_ADD_DISCOVERY_MEMORY_BLOCK, ErrorLevel::FATAL);
    });
    m_managementShm.addMemoryBlock(&heartbeatPoolBlock).or_else([](auto) {
        errorHandler(PoshError::ROUDI__DEFAULT_ROUDI_MEMORY_FAILED_TO_ADD_HEARTBEAT_MEMORY_BLOCK, ErrorLevel::FATAL);
    });
    m_managementShm.addMemoryBlock(&m_segmentManagerBlock).or_else([](auto) {
        errorHandler(PoshError::ROUDI__DEFAULT_ROUDI_MEMORY_FAILED_TO_ADD_SEGMENT_MANAGER_MEMORY_BLOCK,
                     ErrorLevel::FATAL);
    });
}

mepoo::MePooConfig DefaultRouDiMemory::introspectionMemPoolConfig(const uint32_t chunkCount) const noexcept
{
    constexpr uint32_t ALIGNMENT{mepoo::MemPool::CHUNK_MEMORY_ALIGNMENT};
    mepoo::MePooConfig mempoolConfig;
    mempoolConfig.m_mempoolConfig.push_back(
        {align(static_cast<uint32_t>(sizeof(roudi::MemPoolIntrospectionInfoContainer)), ALIGNMENT), chunkCount});
    mempoolConfig.m_mempoolConfig.push_back(
        {align(static_cast<uint32_t>(sizeof(roudi::ProcessIntrospectionFieldTopic)), ALIGNMENT), chunkCount});
    mempoolConfig.m_mempoolConfig.push_back(
        {align(static_cast<uint32_t>(sizeof(roudi::PortIntrospectionFieldTopic)), ALIGNMENT), chunkCount});
    mempoolConfig.m_mempoolConfig.push_back(
        {align(static_cast<uint32_t>(sizeof(roudi::PortThroughputIntrospectionFieldTopic)), ALIGNMENT), chunkCount});
    mempoolConfig.m_mempoolConfig.push_back(
        {align(static_cast<uint32_t>(sizeof(roudi::SubscriberPortChangingIntrospectionFieldTopic)), ALIGNMENT),
         chunkCount});

    mempoolConfig.optimize();
    return mempoolConfig;
}

mepoo::MePooConfig DefaultRouDiMemory::discoveryMemPoolConfig(const uint32_t chunkCount) const noexcept
{
    constexpr uint32_t ALIGNMENT{mepoo::MemPool::CHUNK_MEMORY_ALIGNMENT};
    mepoo::MePooConfig mempoolConfig;
    mempoolConfig.m_mempoolConfig.push_back(
        {align(static_cast<uint32_t>(sizeof(roudi::ServiceRegistry)), ALIGNMENT), chunkCount});

    mempoolConfig.optimize();
    return mempoolConfig;
}

} // namespace roudi
} // namespace iox
