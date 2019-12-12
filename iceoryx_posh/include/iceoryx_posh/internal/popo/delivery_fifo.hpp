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
    struct ChunkManagementTransport
    {
        ChunkManagementTransport() = default;
        ChunkManagementTransport(iox::relative_ptr<mepoo::ChunkManagement> f_chunk)
            : m_segmentId(f_chunk.getId())
            , m_chunkOffset(f_chunk.getOffset())
        {
        }

        RelativePointer::id_t m_segmentId{iox::RelativePointer::NULL_POINTER_ID};
        RelativePointer::offset_t m_chunkOffset{iox::RelativePointer::NULL_POINTER_OFFSET};
    };

    bool pop(mepoo::SharedChunk& chunk);
    bool push(mepoo::SharedChunk&& chunkIn, mepoo::SharedChunk& chunkOut);

    bool empty() const;
    bool resize(const uint32_t f_size);
    uint64_t getCapacity() const;
    uint64_t getSize() const;

  private:
    concurrent::SoFi<ChunkManagementTransport, MAX_RECEIVER_QUEUE_SIZE> m_fifo;
};

} // namespace popo
} // namespace iox
