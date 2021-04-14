// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/internal/mepoo/shm_safe_unmanaged_chunk.hpp"


namespace iox
{
namespace mepoo
{
static_assert(sizeof(ShmSafeUnmanagedChunk) <= 8U,
              "The ShmSafeUnmanagedChunk size must not exceed 64 bit to prevent torn writes!");
static_assert(std::is_trivially_copyable<ShmSafeUnmanagedChunk>::value,
              "The ShmSafeUnmanagedChunk must be trivially copyable to prevent Frankenstein objects when the copy ctor "
              "works on half dead objects!");

ShmSafeUnmanagedChunk::ShmSafeUnmanagedChunk(mepoo::SharedChunk chunk) noexcept
{
    // this is only necessary if it's not an empty chunk
    if (chunk)
    {
        rp::RelativePointer<mepoo::ChunkManagement> ptr = chunk.release();
        auto id = ptr.getId();
        auto offset = ptr.getOffset();
        cxx::Ensures(id <= rp::RelativePointerData::ID_RANGE && "RelativePointer id must fit into id type!");
        cxx::Ensures(offset <= rp::RelativePointerData::OFFSET_RANGE
                     && "RelativePointer offset must fit into offset type!");
        m_chunkManagement = rp::RelativePointerData(static_cast<rp::RelativePointerData::id_t>(id), offset);
    }
}

SharedChunk ShmSafeUnmanagedChunk::releaseToSharedChunk() noexcept
{
    if (m_chunkManagement.isLogicalNullptr())
    {
        return SharedChunk();
    }
    auto chunkMgmt = rp::RelativePointer<mepoo::ChunkManagement>(m_chunkManagement.offset(), m_chunkManagement.id());
    m_chunkManagement.reset();
    return SharedChunk(chunkMgmt);
}

SharedChunk ShmSafeUnmanagedChunk::duplicateToSharedChunk() noexcept
{
    if (m_chunkManagement.isLogicalNullptr())
    {
        return SharedChunk();
    }
    auto chunkMgmt = rp::RelativePointer<mepoo::ChunkManagement>(m_chunkManagement.offset(), m_chunkManagement.id());
    chunkMgmt->m_referenceCounter.fetch_add(1U, std::memory_order_relaxed);
    return SharedChunk(chunkMgmt);
}

bool ShmSafeUnmanagedChunk::isLogicalNullptr() const noexcept
{
    return m_chunkManagement.isLogicalNullptr();
}

ChunkHeader* ShmSafeUnmanagedChunk::getChunkHeader() noexcept
{
    if (m_chunkManagement.isLogicalNullptr())
    {
        return nullptr;
    }
    auto chunkMgmt = rp::RelativePointer<mepoo::ChunkManagement>(m_chunkManagement.offset(), m_chunkManagement.id());
    return chunkMgmt->m_chunkHeader;
}

const ChunkHeader* ShmSafeUnmanagedChunk::getChunkHeader() const noexcept
{
    return const_cast<ShmSafeUnmanagedChunk*>(this)->getChunkHeader();
}

bool ShmSafeUnmanagedChunk::isNotLogicalNullptrAndHasNoOtherOwners() const noexcept
{
    if (m_chunkManagement.isLogicalNullptr())
    {
        return false;
    }

    auto chunkMgmt = rp::RelativePointer<mepoo::ChunkManagement>(m_chunkManagement.offset(), m_chunkManagement.id());
    return chunkMgmt->m_referenceCounter.load(std::memory_order_relaxed) == 1U;
}

} // namespace mepoo
} // namespace iox
