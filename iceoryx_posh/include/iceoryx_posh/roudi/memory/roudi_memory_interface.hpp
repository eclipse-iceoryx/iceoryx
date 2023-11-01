// Copyright (c) 2020, 2021 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_MEMORY_ROUDI_MEMORY_INTERFACE_HPP
#define IOX_POSH_ROUDI_MEMORY_ROUDI_MEMORY_INTERFACE_HPP

#include "iceoryx_posh/internal/roudi/memory/mempool_collection_memory_block.hpp"
#include "iceoryx_posh/internal/roudi/memory/mempool_segment_manager_memory_block.hpp"
#include "iceoryx_posh/internal/roudi/memory/port_pool_memory_block.hpp"
#include "iceoryx_posh/roudi/heartbeat_pool.hpp"
#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_manager.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "iox/optional.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
class MemoryProvider;

class RouDiMemoryInterface
{
  public:
    RouDiMemoryInterface() noexcept = default;
    /// @brief The Destructor of the RouDiMemoryInterface also calls destroy on the registered MemoryProvider
    virtual ~RouDiMemoryInterface() noexcept = default;

    RouDiMemoryInterface(RouDiMemoryInterface&&) = delete;
    RouDiMemoryInterface& operator=(RouDiMemoryInterface&&) = delete;

    RouDiMemoryInterface(const RouDiMemoryInterface&) = delete;
    RouDiMemoryInterface& operator=(const RouDiMemoryInterface&) = delete;

    /// @brief The RouDiMemoryManager calls the the MemoryProvider to create the memory and announce the availability
    /// to its MemoryBlocks
    /// @return an RouDiMemoryManagerError if the MemoryProvider cannot create the memory, otherwise success
    virtual expected<void, RouDiMemoryManagerError> createAndAnnounceMemory() noexcept = 0;

    /// @brief The RouDiMemoryManager calls the the MemoryProvider to destroy the memory, which in turn prompts the
    /// MemoryBlocks to destroy their data
    virtual expected<void, RouDiMemoryManagerError> destroyMemory() noexcept = 0;

    virtual const PosixShmMemoryProvider* mgmtMemoryProvider() const noexcept = 0;
    virtual optional<PortPool*> portPool() noexcept = 0;
    virtual optional<mepoo::MemoryManager*> introspectionMemoryManager() const noexcept = 0;
    virtual optional<mepoo::MemoryManager*> discoveryMemoryManager() const noexcept = 0;
    virtual optional<HeartbeatPool*> heartbeatPool() const noexcept = 0;
    virtual optional<mepoo::SegmentManager<>*> segmentManager() const noexcept = 0;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_ROUDI_MEMORY_INTERFACE_HPP
