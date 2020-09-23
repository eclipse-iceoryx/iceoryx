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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_SENDER_DATA_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_SENDER_DATA_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_distributor_data.hpp"
#include "iceoryx_posh/internal/popo/used_chunk_list.hpp"
#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

namespace iox
{
namespace popo
{
template <uint32_t MaxChunksAllocatedSimultaneously, typename ChunkDistributorDataType>
struct ChunkSenderData : public ChunkDistributorDataType
{
    explicit ChunkSenderData(cxx::not_null<mepoo::MemoryManager* const> memoryManager,
                             const uint64_t historyCapacity = 0U,
                             const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept;

    using ChunkDistributorData_t = ChunkDistributorDataType;

    const relative_ptr<mepoo::MemoryManager> m_memoryMgr;
    mepoo::MemoryInfo m_memoryInfo;
    UsedChunkList<MaxChunksAllocatedSimultaneously> m_chunksInUse;
    mepoo::SequenceNumberType m_sequenceNumber{0u};
    mepoo::SharedChunk m_lastChunk{nullptr};
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender_data.inl"

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_SENDER_DATA_HPP
