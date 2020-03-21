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


#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor.hpp"

#include <mutex>

namespace iox
{
namespace popo
{
ChunkDistributor::ChunkDistributor(MemberType_t* const chunkDistrubutorDataPtr) noexcept
    : m_chunkDistrubutorDataPtr(chunkDistrubutorDataPtr)
{
}

const ChunkDistributor::MemberType_t* ChunkDistributor::getMembers() const noexcept
{
    return m_chunkDistrubutorDataPtr;
}

ChunkDistributor::MemberType_t* ChunkDistributor::getMembers() noexcept
{
    return m_chunkDistrubutorDataPtr;
}

bool ChunkDistributor::addQueue(ChunkQueue::MemberType_t* const queueToAdd, uint64_t requestedHistory) noexcept
{
    if (nullptr == queueToAdd)
    {
        return false;
    }

    auto l_alreadyKnownReceiver =
        std::find_if(getMembers()->m_queues.begin(),
                     getMembers()->m_queues.end(),
                     [&](ChunkQueue::MemberType_t* const queue) { return queue == queueToAdd; });

    // check if the queue is not already in the list
    if (l_alreadyKnownReceiver == getMembers()->m_queues.end())
    {
        if (getMembers()->m_queues.size() < getMembers()->m_queues.capacity())
        {
            getMembers()->m_queues.push_back(queueToAdd);

            uint64_t currChunkHistorySize = getMembers()->m_sampleHistory.size();

            // if the current history is large enough we send the requested number of chunks, else we send the
            // total history
            auto startIndex = (requestedHistory <= currChunkHistorySize) ? currChunkHistorySize - requestedHistory : 0u;
            for (auto i = startIndex; i < currChunkHistorySize; ++i)
            {
                deliverToQueue(queueToAdd, getMembers()->m_sampleHistory[i]);
            }
        }
        else
        {
            errorHandler(Error::kPOPO__CHUNK_DISTRIBUTOR_OVERFLOW_OF_QUEUE_CONTAINER);
            return false;
        }
    }

    return true;
}

void ChunkDistributor::removeQueue(ChunkQueue::MemberType_t* const queueToRemove) noexcept
{
    auto l_iter = std::find(getMembers()->m_queues.begin(), getMembers()->m_queues.end(), queueToRemove);
    if (l_iter != getMembers()->m_queues.end())
    {
        getMembers()->m_queues.erase(l_iter);
    }
}

void ChunkDistributor::removeAllQueues() noexcept
{
    getMembers()->m_queues.clear();
}

bool ChunkDistributor::hasStoredQueues() noexcept
{
    return !getMembers()->m_queues.empty();
}

void ChunkDistributor::deliverToAllStoredQueues(mepoo::SharedChunk chunk) noexcept
{
    // send to all the queues
    for (auto& queue : getMembers()->m_queues)
    {
        deliverToQueue(queue, chunk);
    }

    // update the history
    addToHistoryWithoutDelivery(chunk);
}

void ChunkDistributor::deliverToQueue(ChunkQueue::MemberType_t* const queue, mepoo::SharedChunk chunk) noexcept
{
    ChunkQueue(queue).push(chunk);
}

void ChunkDistributor::addToHistoryWithoutDelivery(mepoo::SharedChunk chunk) noexcept
{
    if (getMembers()->m_sampleHistory.size() >= getMembers()->m_historyCapacity)
    {
        getMembers()->m_sampleHistory.erase(getMembers()->m_sampleHistory.begin());
    }
    getMembers()->m_sampleHistory.push_back(chunk);
}

uint64_t ChunkDistributor::getHistorySize() noexcept
{
    return getMembers()->m_sampleHistory.size();
}

uint64_t ChunkDistributor::getHistoryCapacity() noexcept
{
    return getMembers()->m_historyCapacity;
}

void ChunkDistributor::clearHistory() noexcept
{
    getMembers()->m_sampleHistory.clear();
}


} // namespace popo
} // namespace iox
