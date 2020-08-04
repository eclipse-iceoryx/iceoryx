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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_DISTRIBUTOR_DATA_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_DISTRIBUTOR_DATA_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_pusher.hpp"
#include "iceoryx_utils/cxx/algorithm.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/posix_wrapper/mutex.hpp"

#include <cstdint>
#include <mutex>

namespace iox
{
namespace popo
{
class ThreadSafePolicy
{
  public:
    // needs to be public since we want to use std::lock_guard
    void lock() const noexcept;
    void unlock() const noexcept;
    bool tryLock() const noexcept;

  private:
    mutable posix::mutex m_mutex{true}; // recursive lock
};

class SingleThreadedPolicy
{
  public:
    // needs to be public since we want to use std::lock_guard
    void lock() const noexcept;
    void unlock() const noexcept;
    bool tryLock() const noexcept;
};

template <typename ChunkDistributorDataProperties, typename LockingPolicy, typename ChunkQueuePusherType>
struct ChunkDistributorData : public LockingPolicy
{
    using LockGuard_t = std::lock_guard<
        const ChunkDistributorData<ChunkDistributorDataProperties, LockingPolicy, ChunkQueuePusherType>>;
    using ChunkQueuePusher_t = ChunkQueuePusherType;
    using ChunkQueueData_t = typename ChunkQueuePusherType::MemberType_t;
    using ChunkDistributorDataProperties_t = ChunkDistributorDataProperties;

    explicit ChunkDistributorData(const uint64_t historyCapacity = 0u) noexcept;

    const uint64_t m_historyCapacity;

    using QueueContainer_t = cxx::vector<ChunkQueueData_t*, ChunkDistributorDataProperties_t::MAX_QUEUES>;
    QueueContainer_t m_queues;

    /// @todo using ChunkManagement instead of SharedChunk as in UsedChunkList?
    /// When to store a SharedChunk and when the included ChunkManagement must be used?
    /// If we would make the ChunkDistributor lock-free, can we than extend the UsedChunkList to
    /// be like a ring buffer and use this for the history? This would be needed to be able to safely cleanup
    using HistoryContainer_t = cxx::vector<mepoo::SharedChunk, ChunkDistributorDataProperties_t::MAX_HISTORY_CAPACITY>;
    HistoryContainer_t m_history;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor_data.inl"

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_DISTRIBUTOR_DATA_HPP
