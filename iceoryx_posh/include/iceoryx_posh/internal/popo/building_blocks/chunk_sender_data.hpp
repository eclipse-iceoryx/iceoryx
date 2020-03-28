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

#ifndef IOX_POSH_POPO_CHUNK_SENDER_DATA_HPP_
#define IOX_POSH_POPO_CHUNK_SENDER_DATA_HPP_

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor_data.hpp"
#include "iceoryx_posh/internal/popo/used_chunk_list.hpp"
#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

namespace iox
{
namespace popo
{
struct ChunkSenderData
{
    ChunkSenderData(mepoo::MemoryManager* const memMgr,
                    uint64_t historyCapacity = MAX_SENDER_SAMPLE_HISTORY_CAPACITY,
                    const MemoryInfo& memoryInfo = MemoryInfo()) noexcept
        : m_memoryMgr(memMgr)
        , m_memoryInfo(memoryInfo)
        , m_chunkDistributor(historyCapacity)
    {
    }

    iox::relative_ptr<mepoo::MemoryManager> m_memoryMgr;
    MemoryInfo m_memoryInfo;
    ChunkDistributorData m_chunkDistributorData;
    UsedChunkList<MAX_SAMPLE_ALLOCATE_PER_SENDER> m_chunksInUse;
    mepoo::SequenceNumberType m_sequenceNumber{0u};
    mepoo::SharedChunk m_lastChunk{nullptr};

    // bool m_isUnique{false};
    // throughput related members
    // std::atomic<uint32_t> m_activePayloadSize{0u};
    // Throughput m_throughput{};
    // mutable Throughput m_throughputReadCache{};
    // enum class ThreadContext : uint32_t
    // {
    //     Application,
    //     RouDi,
    //     END_OF_LIST
    // };
    // mutable concurrent::TACO<Throughput, ThreadContext> m_throughputExchange{
    //     concurrent::TACOMode::DenyDataFromSameContext};
};

} // namespace popo
} // namespace iox

#endif
