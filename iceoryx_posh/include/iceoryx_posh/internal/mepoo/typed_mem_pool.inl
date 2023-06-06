// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_MEPOO_TYPED_MEM_POOL_INL
#define IOX_POSH_MEPOO_TYPED_MEM_POOL_INL

#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/chunk_management.hpp"
#include "iceoryx_posh/internal/mepoo/typed_mem_pool.hpp"

namespace iox
{
namespace mepoo
{
template <typename T>
inline TypedMemPool<T>::TypedMemPool(const greater_or_equal<uint32_t, 1> numberOfChunks,
                                     BumpAllocator& managementAllocator,
                                     BumpAllocator& chunkMemoryAllocator) noexcept
    : m_memPool(static_cast<uint32_t>(requiredChunkSize()), numberOfChunks, managementAllocator, chunkMemoryAllocator)
    , m_chunkManagementPool(sizeof(ChunkManagement), numberOfChunks, managementAllocator, managementAllocator)
{
}

template <typename T>
inline expected<ChunkManagement*, TypedMemPoolError> TypedMemPool<T>::acquireChunkManagementPointer() noexcept
{
    ChunkHeader* chunkHeader = static_cast<ChunkHeader*>(m_memPool.getChunk());
    if (chunkHeader == nullptr)
    {
        return err(TypedMemPoolError::OutOfChunks);
    }

    ChunkManagement* chunkManagement = static_cast<ChunkManagement*>(m_chunkManagementPool.getChunk());
    if (chunkManagement == nullptr)
    {
        errorHandler(PoshError::MEPOO__TYPED_MEMPOOL_HAS_INCONSISTENT_STATE);
        return err(TypedMemPoolError::FatalErrorReachedInconsistentState);
    }

    auto chunkSettingsResult = mepoo::ChunkSettings::create(sizeof(T), alignof(T));
    // this is safe since we use correct values for size and alignment
    auto& chunkSettings = chunkSettingsResult.value();

    new (chunkHeader) ChunkHeader(m_memPool.getChunkSize(), chunkSettings);
    new (chunkManagement) ChunkManagement(chunkHeader, &m_memPool, &m_chunkManagementPool);

    return ok(chunkManagement);
}

template <typename T>
template <typename... Targs>
inline expected<SharedPointer<T>, TypedMemPoolError> TypedMemPool<T>::createObject(Targs&&... args) noexcept
{
    auto chunkManagement = acquireChunkManagementPointer();
    if (chunkManagement.has_error())
    {
        return err(chunkManagement.error());
    }

    auto newObject = SharedPointer<T>::create(SharedChunk(*chunkManagement), std::forward<Targs>(args)...);

    if (newObject.has_error())
    {
        errorHandler(PoshError::MEPOO__TYPED_MEMPOOL_MANAGEMENT_SEGMENT_IS_BROKEN);
        return err(TypedMemPoolError::FatalErrorReachedInconsistentState);
    }

    return ok(newObject.value());
}

template <typename T>
template <typename ErrorType, typename... Targs>
inline expected<SharedPointer<T>, variant<TypedMemPoolError, ErrorType>>
TypedMemPool<T>::createObjectWithCreationPattern(Targs&&... args) noexcept
{
    using errorType_t = variant<TypedMemPoolError, ErrorType>;
    auto chunkManagement = acquireChunkManagementPointer();
    if (chunkManagement.has_error())
    {
        return err<errorType_t>(in_place_index<0>(), chunkManagement.error());
    }

    auto newObject = T::create(std::forward<Targs>(args)...);
    if (newObject.has_error())
    {
        return err<errorType_t>(in_place_index<1>(), newObject.error());
    }

    auto sharedPointer = SharedPointer<T>::create(SharedChunk(*chunkManagement), std::move(*newObject));

    if (sharedPointer.has_error())
    {
        errorHandler(PoshError::MEPOO__TYPED_MEMPOOL_MANAGEMENT_SEGMENT_IS_BROKEN);
        return err<errorType_t>(in_place_index<0>(), TypedMemPoolError::FatalErrorReachedInconsistentState);
    }

    return ok(sharedPointer.value());
}

template <typename T>
inline uint32_t TypedMemPool<T>::getChunkCount() const noexcept
{
    return m_memPool.getChunkCount();
}

template <typename T>
inline uint32_t TypedMemPool<T>::getUsedChunks() const noexcept
{
    return m_memPool.getUsedChunks();
}

template <typename T>
inline uint64_t TypedMemPool<T>::requiredChunkSize() noexcept
{
    auto chunkSettingsResult = mepoo::ChunkSettings::create(sizeof(T), alignof(T));
    // this is safe since we use correct values for size and alignment
    auto& chunkSettings = chunkSettingsResult.value();

    return align(static_cast<uint64_t>(chunkSettings.requiredChunkSize()), MemPool::CHUNK_MEMORY_ALIGNMENT);
}

template <typename T>
inline uint64_t TypedMemPool<T>::requiredManagementMemorySize(const uint64_t f_numberOfChunks) noexcept
{
    uint64_t memorySizeForManagementPoolChunks =
        align(f_numberOfChunks * sizeof(ChunkManagement), MemPool::CHUNK_MEMORY_ALIGNMENT);
    uint64_t memorySizeForIndices = MemPool::freeList_t::requiredIndexMemorySize(f_numberOfChunks);
    uint64_t memorySizeForIndicesOfManangementAndDataMemPools =
        2 * align(memorySizeForIndices, MemPool::CHUNK_MEMORY_ALIGNMENT);
    return memorySizeForManagementPoolChunks + memorySizeForIndicesOfManangementAndDataMemPools;
}

template <typename T>
inline uint64_t TypedMemPool<T>::requiredChunkMemorySize(const uint64_t f_numberOfChunks) noexcept
{
    return align(f_numberOfChunks * requiredChunkSize(), MemPool::CHUNK_MEMORY_ALIGNMENT);
}

template <typename T>
inline uint64_t TypedMemPool<T>::requiredFullMemorySize(const uint64_t f_numberOfChunks) noexcept
{
    return requiredManagementMemorySize(f_numberOfChunks) + requiredChunkMemorySize(f_numberOfChunks);
}

} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_TYPED_MEM_POOL_INL
