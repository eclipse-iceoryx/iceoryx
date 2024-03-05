// Copyright (c) 2020, 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"

namespace iox
{
namespace roudi
{
IceOryxRouDiMemoryManager::IceOryxRouDiMemoryManager(const IceoryxConfig& config) noexcept
    : m_fileLock(std::move(
        FileLockBuilder()
            .name(concatenate(iceoryxResourcePrefix(config.domainId, ResourceType::ICEORYX_DEFINED), ROUDI_LOCK_NAME))
            .permission(iox::perms::owner_read | iox::perms::owner_write)
            .create()
            .or_else([](auto& error) {
                if (error == FileLockError::LOCKED_BY_OTHER_PROCESS)
                {
                    IOX_LOG(FATAL, "Could not acquire lock, is RouDi still running?");
                    IOX_REPORT_FATAL(PoshError::ICEORYX_ROUDI_MEMORY_MANAGER__ROUDI_STILL_RUNNING);
                }
                else
                {
                    IOX_LOG(FATAL, "Error occurred while acquiring file lock named " << ROUDI_LOCK_NAME);
                    IOX_REPORT_FATAL(PoshError::ICEORYX_ROUDI_MEMORY_MANAGER__COULD_NOT_ACQUIRE_FILE_LOCK);
                }
            })
            .value()))
    , m_portPoolBlock(config.uniqueRouDiId)
    , m_defaultMemory(config)
{
    m_defaultMemory.m_managementShm.addMemoryBlock(&m_portPoolBlock).or_else([](auto) {
        IOX_REPORT_FATAL(PoshError::ICEORYX_ROUDI_MEMORY_MANAGER__FAILED_TO_ADD_PORTPOOL_MEMORY_BLOCK);
    });
    m_memoryManager.addMemoryProvider(&m_defaultMemory.m_managementShm).or_else([](auto) {
        IOX_REPORT_FATAL(PoshError::ICEORYX_ROUDI_MEMORY_MANAGER__FAILED_TO_ADD_MANAGEMENT_MEMORY_BLOCK);
    });
}

expected<void, RouDiMemoryManagerError> IceOryxRouDiMemoryManager::createAndAnnounceMemory() noexcept
{
    auto result = m_memoryManager.createAndAnnounceMemory();
    m_defaultMemory.heartbeatPoolBlock.emplace();
    auto portPool = m_portPoolBlock.portPool();
    if (result.has_value() && portPool.has_value())
    {
        m_portPool.emplace(*portPool.value());
    }
    return result;
}

expected<void, RouDiMemoryManagerError> IceOryxRouDiMemoryManager::destroyMemory() noexcept
{
    return m_memoryManager.destroyMemory();
}

const PosixShmMemoryProvider* IceOryxRouDiMemoryManager::mgmtMemoryProvider() const noexcept
{
    return &m_defaultMemory.m_managementShm;
}

optional<PortPool*> IceOryxRouDiMemoryManager::portPool() noexcept
{
    return (m_portPool.has_value()) ? make_optional<PortPool*>(&*m_portPool) : nullopt_t();
}

optional<mepoo::MemoryManager*> IceOryxRouDiMemoryManager::introspectionMemoryManager() const noexcept
{
    return m_defaultMemory.m_introspectionMemPoolBlock.memoryManager();
}

optional<mepoo::MemoryManager*> IceOryxRouDiMemoryManager::discoveryMemoryManager() const noexcept
{
    return m_defaultMemory.m_discoveryMemPoolBlock.memoryManager();
}

optional<HeartbeatPool*> IceOryxRouDiMemoryManager::heartbeatPool() const noexcept
{
    return m_defaultMemory.heartbeatPoolBlock.value();
}

optional<mepoo::SegmentManager<>*> IceOryxRouDiMemoryManager::segmentManager() const noexcept
{
    return m_defaultMemory.m_segmentManagerBlock.segmentManager();
}

} // namespace roudi
} // namespace iox
