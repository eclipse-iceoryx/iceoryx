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
#ifndef IOX_POSH_POPO_DELIVERY_FIFO_HPP
#define IOX_POSH_POPO_DELIVERY_FIFO_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/chunk_management.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/internal/concurrent/sofi.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

namespace iox
{
namespace popo
{
class DeliveryFiFo
{
  public:
    using VisibilityIndexType = std::uint16_t;

    struct ChunkManagementTransport
    {
        ChunkManagementTransport() = default;
        ChunkManagementTransport(iox::relative_ptr<mepoo::ChunkManagement> chunk, VisibilityIndexType visibilityIndex)
            : m_segmentId(chunk.getId())
            , m_chunkOffset(chunk.getOffset())
            , m_visibilityIndex(visibilityIndex)
        {
        }

        RelativePointer::id_t m_segmentId{iox::RelativePointer::NULL_POINTER_ID};
        RelativePointer::offset_t m_chunkOffset{iox::RelativePointer::NULL_POINTER_OFFSET};
        VisibilityIndexType m_visibilityIndex;
    };

    bool pop(mepoo::SharedChunk& chunk);
    bool push(mepoo::SharedChunk&& chunkIn, mepoo::SharedChunk& chunkOut);
    bool pop(ChunkManagementTransport& chunkTransport);
    bool push(ChunkManagementTransport&& chunkTransportIn, ChunkManagementTransport& chunkTransportOut);

    bool empty() const;
    bool resize(const uint32_t f_size);
    uint64_t getCapacity() const;
    uint64_t getSize() const;

  private:
    concurrent::SoFi<ChunkManagementTransport, MAX_SUBSCRIBER_QUEUE_CAPACITY> m_fifo;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_DELIVERY_FIFO_HPP
