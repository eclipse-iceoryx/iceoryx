// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_MEMORY_ICEORYX_ROUDI_MEMORY_MANAGER_HPP
#define IOX_POSH_ROUDI_MEMORY_ICEORYX_ROUDI_MEMORY_MANAGER_HPP

#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"

#include "iceoryx_hoofs/posix_wrapper/file_lock.hpp"
#include "iceoryx_posh/roudi/memory/default_roudi_memory.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_manager.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "iox/filesystem.hpp"
#include "iox/logging.hpp"

namespace iox
{
namespace roudi
{
class IceOryxRouDiMemoryManager : public RouDiMemoryInterface
{
  public:
    IceOryxRouDiMemoryManager(const RouDiConfig_t& roudiConfig) noexcept;
    /// @brief The Destructor of the IceOryxRouDiMemoryManager also calls destroy on the registered MemoryProvider
    virtual ~IceOryxRouDiMemoryManager() noexcept = default;

    IceOryxRouDiMemoryManager(IceOryxRouDiMemoryManager&&) = delete;
    IceOryxRouDiMemoryManager& operator=(IceOryxRouDiMemoryManager&&) = delete;

    IceOryxRouDiMemoryManager(const IceOryxRouDiMemoryManager&) = delete;
    IceOryxRouDiMemoryManager& operator=(const IceOryxRouDiMemoryManager&) = delete;

    /// @brief The RouDiMemoryManager calls the the MemoryProvider to create the memory and announce the availability
    /// to its MemoryBlocks
    /// @return an RouDiMemoryManagerError if the MemoryProvider cannot create the memory, otherwise success
    expected<void, RouDiMemoryManagerError> createAndAnnounceMemory() noexcept override;

    /// @brief The RouDiMemoryManager calls the the MemoryProvider to destroy the memory, which in turn prompts the
    /// MemoryBlocks to destroy their data
    expected<void, RouDiMemoryManagerError> destroyMemory() noexcept override;

    const PosixShmMemoryProvider* mgmtMemoryProvider() const noexcept override;
    optional<PortPool*> portPool() noexcept override;
    optional<mepoo::MemoryManager*> introspectionMemoryManager() const noexcept override;
    optional<mepoo::SegmentManager<>*> segmentManager() const noexcept override;

  private:
    // in order to prevent a second RouDi to cleanup the memory resources of a running RouDi, this resources are
    // protected by a file lock
    posix::FileLock fileLock = std::move(
        posix::FileLockBuilder()
            .name(ROUDI_LOCK_NAME)
            .permission(iox::perms::owner_read | iox::perms::owner_write)
            .create()
            .or_else([](auto& error) {
                if (error == posix::FileLockError::LOCKED_BY_OTHER_PROCESS)
                {
                    IOX_LOG(FATAL) << "Could not acquire lock, is RouDi still running?";
                    errorHandler(PoshError::ICEORYX_ROUDI_MEMORY_MANAGER__ROUDI_STILL_RUNNING, iox::ErrorLevel::FATAL);
                }
                else
                {
                    IOX_LOG(FATAL) << "Error occurred while acquiring file lock named " << ROUDI_LOCK_NAME;
                    errorHandler(PoshError::ICEORYX_ROUDI_MEMORY_MANAGER__COULD_NOT_ACQUIRE_FILE_LOCK,
                                 iox::ErrorLevel::FATAL);
                }
            })
            .value());

    PortPoolMemoryBlock m_portPoolBlock;
    optional<PortPool> m_portPool;
    DefaultRouDiMemory m_defaultMemory;
    RouDiMemoryManager m_memoryManager;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_ICEORYX_ROUDI_MEMORY_MANAGER_HPP
