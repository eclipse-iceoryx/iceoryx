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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue.hpp"
#include "iceoryx_utils/cxx/algorithm.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
struct ChunkQueueData;

struct ChunkDistributorData
{
    ChunkDistributorData(uint64_t historyCapacity = MAX_SENDER_SAMPLE_HISTORY_CAPACITY) noexcept
        : m_historyCapacity(algorithm::min(historyCapacity, MAX_SENDER_SAMPLE_HISTORY_CAPACITY))
    {
        if (m_historyCapacity != historyCapacity)
        {
            LogWarn() << "Chunk history too large, reducing from " << historyCapacity << " to "
                      << MAX_SENDER_SAMPLE_HISTORY_CAPACITY;
        }
    }

    const uint64_t m_historyCapacity;

    using QueueContainer_t = cxx::vector<ChunkQueue::MemberType_t*, MAX_RECEIVERS_PER_SENDERPORT>;
    QueueContainer_t m_queues;

    /// @todo using ChunkManagement instead of SheredChunk as in UsedChunkList?
    using SampleHistoryContainer_t = cxx::vector<mepoo::SharedChunk, MAX_SENDER_SAMPLE_HISTORY_CAPACITY>;
    SampleHistoryContainer_t m_sampleHistory;
};

} // namespace popo
} // namespace iox
