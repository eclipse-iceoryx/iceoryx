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
template <typename ChunkDistributorDataType>
inline ChunkDistributor<ChunkDistributorDataType>::ChunkDistributor(
    cxx::not_null<MemberType_t* const> chunkDistrubutorDataPtr) noexcept
    : m_chunkDistrubutorDataPtr(chunkDistrubutorDataPtr)
{
}

template <typename ChunkDistributorDataType>
inline const typename ChunkDistributor<ChunkDistributorDataType>::MemberType_t*
ChunkDistributor<ChunkDistributorDataType>::getMembers() const noexcept
{
    return m_chunkDistrubutorDataPtr;
}

template <typename ChunkDistributorDataType>
inline typename ChunkDistributor<ChunkDistributorDataType>::MemberType_t*
ChunkDistributor<ChunkDistributorDataType>::getMembers() noexcept
{
    return m_chunkDistrubutorDataPtr;
}

template <typename ChunkDistributorDataType>
inline void ChunkDistributor<ChunkDistributorDataType>::addQueue(cxx::not_null<ChunkQueueData_t* const> queueToAdd,
                                                                 uint64_t requestedHistory) noexcept
{
    typename MemberType_t::LockGuard_t lock(*getMembers());

    auto alreadyKnownReceiver = std::find_if(getMembers()->m_queues.begin(),
                                             getMembers()->m_queues.end(),
                                             [&](ChunkQueueData_t* const queue) { return queue == queueToAdd; });

    // check if the queue is not already in the list
    if (alreadyKnownReceiver == getMembers()->m_queues.end())
    {
        if (getMembers()->m_queues.size() < getMembers()->m_queues.capacity())
        {
            getMembers()->m_queues.push_back(queueToAdd);

            uint64_t currChunkHistorySize = getMembers()->m_history.size();

            // if the current history is large enough we send the requested number of chunks, else we send the
            // total history
            auto startIndex = (requestedHistory <= currChunkHistorySize) ? currChunkHistorySize - requestedHistory : 0u;
            for (auto i = startIndex; i < currChunkHistorySize; ++i)
            {
                deliverToQueue(queueToAdd, getMembers()->m_history[i]);
            }
        }
        else
        {
            errorHandler(Error::kPOPO__CHUNK_DISTRIBUTOR_OVERFLOW_OF_QUEUE_CONTAINER, nullptr, ErrorLevel::SEVERE);
        }
    }
}

template <typename ChunkDistributorDataType>
inline void
ChunkDistributor<ChunkDistributorDataType>::removeQueue(cxx::not_null<ChunkQueueData_t* const> queueToRemove) noexcept
{
    typename MemberType_t::LockGuard_t lock(*getMembers());

    auto iter = std::find(getMembers()->m_queues.begin(), getMembers()->m_queues.end(), queueToRemove);
    if (iter != getMembers()->m_queues.end())
    {
        getMembers()->m_queues.erase(iter);
    }
}

template <typename ChunkDistributorDataType>
inline void ChunkDistributor<ChunkDistributorDataType>::removeAllQueues() noexcept
{
    typename MemberType_t::LockGuard_t lock(*getMembers());

    getMembers()->m_queues.clear();
}

template <typename ChunkDistributorDataType>
inline bool ChunkDistributor<ChunkDistributorDataType>::hasStoredQueues() noexcept
{
    typename MemberType_t::LockGuard_t lock(*getMembers());

    return !getMembers()->m_queues.empty();
}

template <typename ChunkDistributorDataType>
inline void ChunkDistributor<ChunkDistributorDataType>::deliverToAllStoredQueues(mepoo::SharedChunk chunk) noexcept
{
    typename MemberType_t::LockGuard_t lock(*getMembers());

    // send to all the queues
    for (auto& queue : getMembers()->m_queues)
    {
        deliverToQueue(queue, chunk);
    }

    // update the history
    addToHistoryWithoutDelivery(chunk);
}

template <typename ChunkDistributorDataType>
inline void ChunkDistributor<ChunkDistributorDataType>::deliverToQueue(cxx::not_null<ChunkQueueData_t* const> queue,
                                                                       mepoo::SharedChunk chunk) noexcept
{
    // We intentionally do not return anything here as from a ChunkDistributor point of view it doesn't matter if the
    // push succeeds or fails
    ChunkQueuePusher_t(queue).push(chunk);
}

template <typename ChunkDistributorDataType>
inline void ChunkDistributor<ChunkDistributorDataType>::addToHistoryWithoutDelivery(mepoo::SharedChunk chunk) noexcept
{
    typename MemberType_t::LockGuard_t lock(*getMembers());

    if (0 < getMembers()->m_historyCapacity)
    {
        if (getMembers()->m_history.size() >= getMembers()->m_historyCapacity)
        {
            getMembers()->m_history.erase(getMembers()->m_history.begin());
        }
        getMembers()->m_history.push_back(chunk);
    }
}

template <typename ChunkDistributorDataType>
inline uint64_t ChunkDistributor<ChunkDistributorDataType>::getHistorySize() noexcept
{
    typename MemberType_t::LockGuard_t lock(*getMembers());

    return getMembers()->m_history.size();
}

template <typename ChunkDistributorDataType>
inline uint64_t ChunkDistributor<ChunkDistributorDataType>::getHistoryCapacity() const noexcept
{
    return getMembers()->m_historyCapacity;
}

template <typename ChunkDistributorDataType>
inline void ChunkDistributor<ChunkDistributorDataType>::clearHistory() noexcept
{
    typename MemberType_t::LockGuard_t lock(*getMembers());

    getMembers()->m_history.clear();
}

template <typename ChunkDistributorDataType>
inline void ChunkDistributor<ChunkDistributorDataType>::cleanup() noexcept
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
