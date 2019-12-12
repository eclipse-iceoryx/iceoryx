// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "mempool_introspection.hpp"

namespace iox
{
namespace roudi
{
template <typename MemoryManager, typename SegmentManager, typename SenderPort>
MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::MemPoolIntrospection(
    MemoryManager& rouDiInternalMemoryManager, SegmentManager& segmentManager, SenderPort&& senderPort)
    : m_rouDiInternalMemoryManager(&rouDiInternalMemoryManager)
    , m_segmentManager(&segmentManager)
    , m_senderPort(std::move(senderPort))
    , m_thread(&MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::threadMain, this)
{
    m_senderPort.activate(); // corresponds to offer

    /// @todo create a wrapper function which takes care of the 16 character limitation of the thread name
    // set thread name
    pthread_setname_np(m_thread.native_handle(), "MemPoolIntr");
}

template <typename MemoryManager, typename SegmentManager, typename SenderPort>
MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::~MemPoolIntrospection()
{
    m_senderPort.deactivate(); // stop offer
    terminate();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::wakeUp(RunLevel newLevel)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_runLevel.store(newLevel, std::memory_order_seq_cst);
    m_waitConditionVar.notify_one();
}

template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::start()
{
    wakeUp(RUN);
}

template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::wait()
{
    wakeUp(WAIT);
}

template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::terminate()
{
    wakeUp(TERMINATE);
}

template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::setSnapshotInterval(
    unsigned int snapshotInterval_ms)
{
    m_snapShotInterval = std::chrono::milliseconds(snapshotInterval_ms);
}

template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::run()
{
    while (m_runLevel.load(std::memory_order_seq_cst) == RUN)
    {
        send();
        // TODO: could use sleep_until to avoid drift but a small drift is
        // not critical here
        std::this_thread::sleep_for(m_snapShotInterval);
    }
}

template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::waitForRunLevelChange()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_runLevel.load(std::memory_order_seq_cst) == WAIT)
    {
        m_waitConditionVar.wait(lock);
    }
}

// wait until start command, run until wait or terminate, restart from wait
// is possible  terminate call leads to exit
template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::threadMain()
{
    while (m_runLevel.load(std::memory_order_seq_cst) != TERMINATE)
    {
        waitForRunLevelChange();
        run();
    }
}

template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::prepareIntrospectionSample(
    Topic* f_sample, const posix::PosixGroup& f_readerGroup, const posix::PosixGroup& f_writerGroup, uint32_t f_id)
{
    strncpy(f_sample->m_readerGroupName, f_readerGroup.getName().c_str(), MAX_GROUP_NAME_LENGTH - 1);
    f_sample->m_readerGroupName[MAX_GROUP_NAME_LENGTH - 1] = 0;
    strncpy(f_sample->m_writerGroupName, f_writerGroup.getName().c_str(), MAX_GROUP_NAME_LENGTH - 1);
    f_sample->m_writerGroupName[MAX_GROUP_NAME_LENGTH - 1] = 0;
    f_sample->m_id = f_id;
}


template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::send()
{
    if (m_senderPort.hasSubscribers())
    {
        uint32_t id = 0;

        auto chunkHeader = m_senderPort.reserveChunk(sizeof(Topic));
        auto sample = static_cast<Topic*>(chunkHeader->payload());
        new (sample) Topic;

        prepareIntrospectionSample(
            sample, posix::PosixGroup::getGroupOfCurrentProcess(), posix::PosixGroup::getGroupOfCurrentProcess(), id);
        copyMemPoolInfo(*m_rouDiInternalMemoryManager, sample->m_mempoolInfo);

        m_senderPort.deliverChunk(chunkHeader);

        for (auto& segment : m_segmentManager->m_segmentContainer)
        {
            ++id;
            auto chunkHeader = m_senderPort.reserveChunk(sizeof(Topic));
            auto sample = static_cast<Topic*>(chunkHeader->payload());
            new (sample) Topic;

            prepareIntrospectionSample(sample, segment.getReaderGroup(), segment.getWriterGroup(), id);
            copyMemPoolInfo(segment.getMemoryManager(), sample->m_mempoolInfo);

            m_senderPort.deliverChunk(chunkHeader);
        }
    }
}

// copy data fro internal struct into interface struct
template <typename MemoryManager, typename SegmentManager, typename SenderPort>
void MemPoolIntrospection<MemoryManager, SegmentManager, SenderPort>::copyMemPoolInfo(
    const MemoryManager& f_memoryManager, MemPoolInfoContainer& f_dest)
{
    auto numOfMemPools = f_memoryManager.getNumberOfMemPools();
    f_dest = MemPoolInfoContainer(numOfMemPools, MemPoolInfo());
    for (uint32_t i = 0; i < numOfMemPools; ++i)
    {
        auto src = f_memoryManager.getMemPoolInfo(i);
        auto& dst = f_dest[static_cast<int>(i)];
        dst.m_usedChunks = src.m_usedChunks;
        dst.m_minFreeChunks = src.m_minFreeChunks;
        dst.m_numChunks = src.m_numChunks;
        dst.m_chunkSize = src.m_chunkSize;
        dst.m_payloadSize = src.m_chunkSize - static_cast<uint32_t>(sizeof(mepoo::ChunkHeader));
    }
}

} // namespace roudi
} // namespace iox
