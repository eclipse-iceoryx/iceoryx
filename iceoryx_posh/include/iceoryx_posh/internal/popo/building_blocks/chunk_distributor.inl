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


namespace iox
{
namespace popo
{
template <uint32_t MaxQueues, typename LockingPolicy>
inline ChunkDistributor<MaxQueues, LockingPolicy>::ChunkDistributor(
    MemberType_t* const chunkDistrubutorDataPtr) noexcept
    : m_chunkDistrubutorDataPtr(chunkDistrubutorDataPtr)
{
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline const typename ChunkDistributor<MaxQueues, LockingPolicy>::MemberType_t*
ChunkDistributor<MaxQueues, LockingPolicy>::getMembers() const noexcept
{
    return m_chunkDistrubutorDataPtr;
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline typename ChunkDistributor<MaxQueues, LockingPolicy>::MemberType_t*
ChunkDistributor<MaxQueues, LockingPolicy>::getMembers() noexcept
{
    return m_chunkDistrubutorDataPtr;
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline bool ChunkDistributor<MaxQueues, LockingPolicy>::addQueue(ChunkQueue::MemberType_t* const queueToAdd,
                                                                 uint64_t requestedHistory) noexcept
{
    typename MemberType_t::lockGuard_t lock(*getMembers());

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
            errorHandler(Error::kPOPO__CHUNK_DISTRIBUTOR_OVERFLOW_OF_QUEUE_CONTAINER, nullptr, ErrorLevel::SEVERE);
            return false;
        }
    }

    return true;
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline void
ChunkDistributor<MaxQueues, LockingPolicy>::removeQueue(ChunkQueue::MemberType_t* const queueToRemove) noexcept
{
    typename MemberType_t::lockGuard_t lock(*getMembers());

    auto l_iter = std::find(getMembers()->m_queues.begin(), getMembers()->m_queues.end(), queueToRemove);
    if (l_iter != getMembers()->m_queues.end())
    {
        getMembers()->m_queues.erase(l_iter);
    }
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline void ChunkDistributor<MaxQueues, LockingPolicy>::removeAllQueues() noexcept
{
    typename MemberType_t::lockGuard_t lock(*getMembers());

    getMembers()->m_queues.clear();
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline bool ChunkDistributor<MaxQueues, LockingPolicy>::hasStoredQueues() noexcept
{
    typename MemberType_t::lockGuard_t lock(*getMembers());

    return !getMembers()->m_queues.empty();
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline void ChunkDistributor<MaxQueues, LockingPolicy>::deliverToAllStoredQueues(mepoo::SharedChunk chunk) noexcept
{
    typename MemberType_t::lockGuard_t lock(*getMembers());

    // send to all the queues
    for (auto& queue : getMembers()->m_queues)
    {
        deliverToQueue(queue, chunk);
    }

    // update the history
    addToHistoryWithoutDelivery(chunk);
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline void ChunkDistributor<MaxQueues, LockingPolicy>::deliverToQueue(ChunkQueue::MemberType_t* const queue,
                                                                       mepoo::SharedChunk chunk) noexcept
{
    ChunkQueue(queue).push(chunk);
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline void ChunkDistributor<MaxQueues, LockingPolicy>::addToHistoryWithoutDelivery(mepoo::SharedChunk chunk) noexcept
{
    typename MemberType_t::lockGuard_t lock(*getMembers());

    if (0 < getMembers()->m_historyCapacity)
    {
        if (getMembers()->m_sampleHistory.size() >= getMembers()->m_historyCapacity)
        {
            getMembers()->m_sampleHistory.erase(getMembers()->m_sampleHistory.begin());
        }
        getMembers()->m_sampleHistory.push_back(chunk);
    }
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline uint64_t ChunkDistributor<MaxQueues, LockingPolicy>::getHistorySize() noexcept
{
    typename MemberType_t::lockGuard_t lock(*getMembers());

    return getMembers()->m_sampleHistory.size();
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline uint64_t ChunkDistributor<MaxQueues, LockingPolicy>::getHistoryCapacity() const noexcept
{
    return getMembers()->m_historyCapacity;
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline void ChunkDistributor<MaxQueues, LockingPolicy>::clearHistory() noexcept
{
    typename MemberType_t::lockGuard_t lock(*getMembers());

    getMembers()->m_sampleHistory.clear();
}

template <uint32_t MaxQueues, typename LockingPolicy>
inline void ChunkDistributor<MaxQueues, LockingPolicy>::cleanup() noexcept
{
    if (getMembers()->tryLock())
    {
        clearHistory();
        getMembers()->unlock();
    }
    else
    {
        /// @todo currently we have a deadlock / mutex destroy vulnerability if the ThreadSafePolicy is used
        /// and a sending application dies when having the lock for sending. If the RouDi daemon wants to
        /// cleanup or does discovery changes we have a deadlock or an exception when destroying the mutex
        /// As long as we don't have a multi-threaded lock-free ChunkDistributor or another concept we die here
        errorHandler(Error::kPOPO__CHUNK_DISTRIBUTOR_CLEANUP_DEADLOCK_BECAUSE_BAD_APPLICATION_TERMINATION,
                     nullptr,
                     ErrorLevel::FATAL);
    }
}

} // namespace popo
} // namespace iox
