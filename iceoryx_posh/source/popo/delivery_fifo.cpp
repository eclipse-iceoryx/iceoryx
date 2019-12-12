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

#include "iceoryx_posh/internal/popo/delivery_fifo.hpp"

namespace iox
{
namespace popo
{
bool DeliveryFiFo::pop(mepoo::SharedChunk& chunk)
{
    DeliveryFiFo::ChunkManagementTransport chunkTransport;
    bool retVal = m_fifo.pop(chunkTransport);
    if (retVal == true)
    {
        auto chunkManagement =
            iox::relative_ptr<mepoo::ChunkManagement>(chunkTransport.m_chunkOffset, chunkTransport.m_segmentId);
        chunk = mepoo::SharedChunk(chunkManagement);
    }
    return retVal;
}

bool DeliveryFiFo::push(mepoo::SharedChunk&& chunkIn, mepoo::SharedChunk& chunkOut)
{
    DeliveryFiFo::ChunkManagementTransport chunkTransportIn(chunkIn.releaseWithRelativePtr());
    DeliveryFiFo::ChunkManagementTransport chunkTransportOut;

    bool retVal = m_fifo.push(std::move(chunkTransportIn), chunkTransportOut);

    if (retVal == false)
    {
        auto chunkManagement =
            iox::relative_ptr<mepoo::ChunkManagement>(chunkTransportOut.m_chunkOffset, chunkTransportOut.m_segmentId);
        chunkOut = mepoo::SharedChunk(chunkManagement);
    }
    return retVal;
}

bool DeliveryFiFo::empty() const
{
    return m_fifo.empty();
}

bool DeliveryFiFo::resize(const uint32_t f_size)
{
    return m_fifo.resize(f_size);
}

uint64_t DeliveryFiFo::getCapacity() const
{
    return m_fifo.capacity();
}

uint64_t DeliveryFiFo::getSize() const
{
    return m_fifo.size();
}

} // namespace popo
} // namespace iox
