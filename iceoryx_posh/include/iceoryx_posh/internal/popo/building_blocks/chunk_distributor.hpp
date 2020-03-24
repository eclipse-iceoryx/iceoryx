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

#pragma once

#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor_data.hpp"

namespace iox
{
namespace popo
{
/// @brief The ChunkDistributor is the low layer building block to send SharedChunks to a dynamic number of ChunkQueus.
/// Together with the ChunkQueue, the ChunkDistributor builds the infrastructure to exchange memory chunks between
/// different data producers and consumers that could be located in different processes. Besides a modifiable container
/// of ChunkQueues to which a SharedChunk can be deliverd, it holds a configurable history of last sent chunks. This
/// allows to provide a newly added queue a number of last chunks to start from. This is needed for functionality
/// known as latched topic in ROS or field in ara::com. A ChunkDistributor is used to build elements of higher
/// abstraction layers that also do memory managemet and provide an API towards the real user
class ChunkDistributor
{
  public:
    using MemberType_t = ChunkDistributorData;

    ChunkDistributor(MemberType_t* const chunkDistrubutorDataPtr) noexcept;

    ChunkDistributor(const ChunkDistributor& other) = delete;
    ChunkDistributor& operator=(const ChunkDistributor&) = delete;
    ChunkDistributor(ChunkDistributor&& rhs) = default;
    ChunkDistributor& operator=(ChunkDistributor&& rhs) = default;
    ~ChunkDistributor() = default;

    /// @brief Add a queue to the internal list of chunk queues to which chunks are delivered when calling
    /// deliverToAllStoredQueues
    /// @param[in] queueToAdd chunk queue to add to the list
    /// @param[in] requestedHistory number of last chunks from history to send if available. If history size is smaller
    /// then the available history size chunks are provided
    /// @return true on success otherwise false
    bool addQueue(ChunkQueue::MemberType_t* const queueToAdd, uint64_t requestedHistory = 0) noexcept;

    /// @brief Remove a queue from the internal list of chunk queues
    /// @param[in] chunk queue to remove from the list
    void removeQueue(ChunkQueue::MemberType_t* const queueToRemove) noexcept;

    /// @brief Delete all the stored chunk queues
    void removeAllQueues() noexcept;

    /// @brief Get the information whether there are any stored chunk queues
    /// @return true if there are stored chunk queues, false if not
    bool hasStoredQueues() noexcept;

    /// @brief Deliver the provided shared chunk to all the stored chunk queues. The chunk will be added to the chunk
    /// history
    /// @param[in] shared chunk to be delivered
    void deliverToAllStoredQueues(mepoo::SharedChunk chunk) noexcept;

    /// @brief Deliver the provided shared chunk to the provided chunk queue. The chunk will NOT be added to the chunk
    /// history
    /// @param[in] chunk queue to which this chunk shall be delivered
    /// @param[in] shared chunk to be delivered
    void deliverToQueue(ChunkQueue::MemberType_t* const queue, mepoo::SharedChunk chunk) noexcept;

    /// @brief Update the chunk history but do not deliver the chunk to any chunk queue. E.g. use case is to to update a
    /// non offered field in ara
    /// @param[in] shared chunk add to the chunk history
    void addToHistoryWithoutDelivery(mepoo::SharedChunk chunk) noexcept;

    /// @brief Get the current size of the chunk history
    /// @return chunk history size
    uint64_t getHistorySize() noexcept;

    /// @brief Get the capacity of the chunk history
    /// @return chunk history capacity
    uint64_t getHistoryCapacity() noexcept;

    /// @brief Clears the chunk history
    void clearHistory() noexcept;

  protected:
    const MemberType_t* getMembers() const noexcept;
    MemberType_t* getMembers() noexcept;

  private:
    MemberType_t* m_chunkDistrubutorDataPtr{nullptr};
};

} // namespace popo
} // namespace iox
