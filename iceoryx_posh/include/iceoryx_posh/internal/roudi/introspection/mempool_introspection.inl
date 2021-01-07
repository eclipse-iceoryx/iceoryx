// Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_INTROSPECTION_MEMPOOL_INTROSPECTION_INL
#define IOX_POSH_ROUDI_INTROSPECTION_MEMPOOL_INTROSPECTION_INL

#include "iceoryx_utils/posix_wrapper/thread.hpp"
#include "mempool_introspection.hpp"

namespace iox
{
namespace roudi
{
template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::MemPoolIntrospection(
    MemoryManager& rouDiInternalMemoryManager, SegmentManager& segmentManager, PublisherPort&& publisherPort)
    : m_rouDiInternalMemoryManager(&rouDiInternalMemoryManager)
    , m_segmentManager(&segmentManager)
    , m_publisherPort(std::move(publisherPort))
    , m_thread(&MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::threadMain, this)
{
    m_publisherPort.offer();

    posix::setThreadName(m_thread.native_handle(), "MemPoolIntr");
}

template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::~MemPoolIntrospection()
{
    m_publisherPort.stopOffer();
    terminate();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::wakeUp(RunLevel newLevel) noexcept
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_runLevel.store(newLevel, std::memory_order_seq_cst);
    m_waitConditionVar.notify_one();
}

template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::start() noexcept
{
    wakeUp(RUN);
}

template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::wait() noexcept
{
    wakeUp(WAIT);
}

template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::terminate() noexcept
{
    wakeUp(TERMINATE);
}

template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::setSnapshotInterval(
    unsigned int snapshotInterval_ms) noexcept
{
    m_snapShotInterval = std::chrono::milliseconds(snapshotInterval_ms);
}

template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::run() noexcept
{
    while (m_runLevel.load(std::memory_order_seq_cst) == RUN)
    {
        send();
        // TODO: could use sleep_until to avoid drift but a small drift is
        // not critical here
        std::this_thread::sleep_for(m_snapShotInterval);
    }
}

template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::waitForRunLevelChange() noexcept
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_runLevel.load(std::memory_order_seq_cst) == WAIT)
    {
        m_waitConditionVar.wait(lock);
    }
}

// wait until start command, run until wait or terminate, restart from wait
// is possible  terminate call leads to exit
template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::threadMain() noexcept
{
    while (m_runLevel.load(std::memory_order_seq_cst) != TERMINATE)
    {
        waitForRunLevelChange();
        run();
    }
}

template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::prepareIntrospectionSample(
    MemPoolIntrospectionInfo& sample,
    const posix::PosixGroup& readerGroup,
    const posix::PosixGroup& writerGroup,
    uint32_t id) noexcept
{
    sample.m_readerGroupName.assign("");
    sample.m_readerGroupName.append(cxx::TruncateToCapacity, readerGroup.getName());
    sample.m_writerGroupName.assign("");
    sample.m_writerGroupName.append(cxx::TruncateToCapacity, writerGroup.getName());
    sample.m_id = id;
}


template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::send() noexcept
{
    if (m_publisherPort.hasSubscribers())
    {
        uint32_t id = 0U;
        auto maybeChunkHeader = m_publisherPort.tryAllocateChunk(sizeof(MemPoolIntrospectionInfoContainer));
        if (maybeChunkHeader.has_error())
        {
            LogWarn() << "Cannot allocate chunk for mempool introspection!";
            errorHandler(Error::kMEPOO__CANNOT_ALLOCATE_CHUNK, nullptr, ErrorLevel::MODERATE);
        }

        auto sample = static_cast<MemPoolIntrospectionInfoContainer*>(maybeChunkHeader.value()->payload());
        new (sample) MemPoolIntrospectionInfoContainer;

        if (sample->emplace_back())
        {
            // RouDi's shm segment
            auto& memPoolIntrospectionInfo = sample->back();
            prepareIntrospectionSample(memPoolIntrospectionInfo,
                                       posix::PosixGroup::getGroupOfCurrentProcess(),
                                       posix::PosixGroup::getGroupOfCurrentProcess(),
                                       id);
            copyMemPoolInfo(*m_rouDiInternalMemoryManager, memPoolIntrospectionInfo.m_mempoolInfo);
            ++id;

            // User shm segments
            for (auto& segment : m_segmentManager->m_segmentContainer)
            {
                if (sample->emplace_back())
                {
                    auto& memPoolIntrospectionInfo = sample->back();
                    prepareIntrospectionSample(
                        memPoolIntrospectionInfo, segment.getReaderGroup(), segment.getWriterGroup(), id);
                    copyMemPoolInfo(segment.getMemoryManager(), memPoolIntrospectionInfo.m_mempoolInfo);
                }
                else
                {
                    LogWarn() << "Mempool Introspection Container full, Mempool Introspection Data not fully updated! "
                              << (id + 1U) << " of " << m_segmentManager->m_segmentContainer.size()
                              << " memory segments sent.";
                    errorHandler(Error::kMEPOO__INTROSPECTION_CONTAINER_FULL, nullptr, ErrorLevel::MODERATE);
                    break;
                }
                ++id;
            }
        }
        else
        {
            LogWarn() << "Mempool Introspection Container full, Mempool Introspection Data not fully updated! "
                      << (id + 1U) << " of " << m_segmentManager->m_segmentContainer.size() << " memory segments sent.";
            errorHandler(Error::kMEPOO__INTROSPECTION_CONTAINER_FULL, nullptr, ErrorLevel::MODERATE);
        }

        m_publisherPort.sendChunk(maybeChunkHeader.value());
    }
}

// copy data fro internal struct into interface struct
template <typename MemoryManager, typename SegmentManager, typename PublisherPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, PublisherPort>::copyMemPoolInfo(
    const MemoryManager& memoryManager, MemPoolInfoContainer& dest) noexcept
{
    auto numOfMemPools = memoryManager.getNumberOfMemPools();
    dest = MemPoolInfoContainer(numOfMemPools, MemPoolInfo());
    for (uint32_t i = 0U; i < numOfMemPools; ++i)
    {
        auto src = memoryManager.getMemPoolInfo(i);
        auto& dst = dest[i];
        dst.m_usedChunks = src.m_usedChunks;
        dst.m_minFreeChunks = src.m_minFreeChunks;
        dst.m_numChunks = src.m_numChunks;
        dst.m_chunkSize = src.m_chunkSize;
        dst.m_payloadSize = src.m_chunkSize - static_cast<uint32_t>(sizeof(mepoo::ChunkHeader));
    }
}

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_INTROSPECTION_MEMPOOL_INTROSPECTION_INL
