// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_MEMORY_ICEORYX_ROUDI_MEMORY_MANAGER_HPP
#define IOX_POSH_ROUDI_MEMORY_ICEORYX_ROUDI_MEMORY_MANAGER_HPP

#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"

#include "iceoryx_posh/roudi/memory/default_roudi_memory.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_manager.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"

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
    cxx::expected<RouDiMemoryManagerError> createAndAnnounceMemory() noexcept override;

    /// @brief The RouDiMemoryManager calls the the MemoryProvider to destroy the memory, which in turn prompts the
    /// MemoryBlocks to destroy their data
    cxx::expected<RouDiMemoryManagerError> destroyMemory() noexcept override;

    PosixShmMemoryProvider* mgmtMemoryProvider() noexcept override;
    const PosixShmMemoryProvider* mgmtMemoryProvider() const noexcept override;
    cxx::optional<PortPool*> portPool() noexcept override;
    cxx::optional<mepoo::MemoryManager*> introspectionMemoryManager() const noexcept override;
    cxx::optional<mepoo::SegmentManager<>*> segmentManager() const noexcept override;

  private:
    PortPoolMemoryBlock m_portPoolBlock;
    cxx::optional<PortPool> m_portPool;
    DefaultRouDiMemory m_defaultMemory;
    RouDiMemoryManager m_memoryManager;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_ICEORYX_ROUDI_MEMORY_MANAGER_HPP
