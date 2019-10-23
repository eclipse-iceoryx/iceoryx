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

#pragma once

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/internal/roudi/roudi_process.hpp"
#include "iceoryx_posh/internal/roudi/shared_memory_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

namespace iox
{
namespace roudi
{
/**
 * @brief This class handles the mempool intropection for RouDi.
 *        It is recommended to use the MemPoolIntrospectionType alias which sets
 *        the intended template parameters required for the actual introspection.
 *
 *        The class sends snapshots of the mempool usage to the introspection
 *        client if subscribed.
 */
template <typename MemoryManager, typename SegmentManager, typename SenderPort>
class MemPoolIntrospection
{
  private:
    enum RunLevel
    {
        RUN,
        WAIT,
        TERMINATE
    };

    using Topic = MemPoolIntrospectionTopic;

  public:
    /**
     * @brief The constructor for the MemPoolIntrospection.
     *        It starts a thread and set it into a wait condition.
     *
     * @param rouDiInternalMemoryManager is the internal RouDi memory manager
     * @param segmentManager contains the shared memory segments and their memory pools which will be intropected
     * @param senderPort is the sender port for transmission of the introspection data
     */
    MemPoolIntrospection(MemoryManager& rouDiInternalMemoryManager,
                         SegmentManager& segmentManager,
                         SenderPort&& senderPort);

    ~MemPoolIntrospection();

    // delete copy constructor and assignment operator
    MemPoolIntrospection(MemPoolIntrospection const&) = delete;
    MemPoolIntrospection& operator=(MemPoolIntrospection const&) = delete;
    // delete move constructor and assignment operator
    MemPoolIntrospection(MemPoolIntrospection&&) = delete;
    MemPoolIntrospection& operator=(MemPoolIntrospection&&) = delete;

    /**
     * @brief This function sets the thread into run mode, which periodically
     *        sends snapshots of the mempool introspecton data to the client.
     *        The send interval can be set by @ref setSnapshotInterval "setSnapshotInterval(...)".
     */
    void start();

    /**
     * @brief This functions sets the thread into a wait state and
     *        the transmission of the introspection data is stopped.
     */
    void wait();

    /**
     * @brief This function stops the thread which sends the introspection data.
     *        It is not possible to start the thread again.
     */
    void terminate();

    /**
     * @brief This function configures the interval for the transmission of the
     *        mempool introspection data.
     *
     * @param snapshotInterval_ms is the interval time in milliseconds
     */
    void setSnapshotInterval(unsigned int snapshotInterval_ms);

  private:
    MemoryManager* m_rouDiInternalMemoryManager{nullptr}; // mempool handler needs to outlive this class (!)
    SegmentManager* m_segmentManager{nullptr};

    std::chrono::milliseconds m_snapShotInterval{1000};

    SenderPort m_senderPort{nullptr};

    std::atomic<RunLevel> m_runLevel{WAIT};
    std::condition_variable m_waitConditionVar;
    std::mutex m_mutex;
    std::thread m_thread;

    void run();
    void waitForRunLevelChange();
    void wakeUp(RunLevel newLevel = RUN);

    // wait until start command, run until wait or terminate, restart from wait
    // is possible  terminate call leads to exit
    void threadMain();

    void prepareIntrospectionSample(Topic* f_sample,
                                    const posix::PosixGroup& f_readerGroup,
                                    const posix::PosixGroup& f_writerGroup,
                                    uint32_t f_id);
    void send();

    // copy data fro internal struct into interface struct
    void copyMemPoolInfo(const MemoryManager& f_memoryManager, MemPoolInfoContainer& f_dest);
};

/**
 * @brief typedef for the templated mempool introspection class that is used by RouDi for the
 * actual mempool introspection functionality.
 */
using MemPoolIntrospectionType = MemPoolIntrospection<mepoo::MemoryManager, mepoo::SegmentManager<>, popo::SenderPort>;


} // namespace roudi
} // namespace iox
#include "mempool_introspection.inl"
