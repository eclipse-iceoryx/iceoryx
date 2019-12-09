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

namespace iox
{
namespace mepoo
{
template <typename T>
inline TypedMemPool<T>::TypedMemPool(const cxx::greater_or_equal<uint32_t, 1> f_numberOfChunks,
                                     posix::Allocator* f_managementAllocator,
                                     posix::Allocator* f_payloadAllocator)
    : m_memPool(getAdjustedPayloadSize(), f_numberOfChunks, f_managementAllocator, f_payloadAllocator)
    , m_chunkManagementPool(sizeof(ChunkManagement), f_numberOfChunks, f_managementAllocator, f_managementAllocator)
{
}

template <typename T>
template <typename... Targs>
inline cxx::expected<SharedPointer<T>, TypedMemPoolError> TypedMemPool<T>::createObject(Targs&&... args)
{
    ChunkHeader* chunkHeader = static_cast<ChunkHeader*>(m_memPool.getChunk());
    if (chunkHeader == nullptr)
    {
        return cxx::error<TypedMemPoolError>(TypedMemPoolError::OutOfChunks);
    }

    ChunkManagement* chunkManagement = static_cast<ChunkManagement*>(m_chunkManagementPool.getChunk());
    if (chunkManagement == nullptr)
    {
        errorHandler(Error::kMEPOO__TYPED_MEMPOOL_HAS_INCONSISTENT_STATE);
        return cxx::error<TypedMemPoolError>(TypedMemPoolError::FatalErrorReachedInconsistentState);
    }

    new (chunkHeader) ChunkHeader();
    chunkHeader->m_info.m_payloadSize = sizeof(T);
    chunkHeader->m_info.m_usedSizeOfChunk = MemoryManager::sizeWithChunkHeaderStruct(sizeof(T));

    new (chunkManagement) ChunkManagement(chunkHeader, &m_memPool, &m_chunkManagementPool);
    auto newObject = SharedPointer<T>::create(SharedChunk(chunkManagement), std::forward<Targs>(args)...);

    if (newObject.has_error())
    {
        errorHandler(Error::kMEPOO__TYPED_MEMPOOL_MANAGEMENT_SEGMENT_IS_BROKEN);
        return cxx::error<TypedMemPoolError>(TypedMemPoolError::FatalErrorReachedInconsistentState);
    }

    return cxx::success<SharedPointer<T>>(newObject.get_value());
}

template <typename T>
uint32_t TypedMemPool<T>::getChunkCount() const
{
    return m_memPool.getChunkCount();
}

template <typename T>
uint32_t TypedMemPool<T>::getUsedChunks() const
{
    return m_memPool.getUsedChunks();
}

template <typename T>
uint64_t TypedMemPool<T>::getAdjustedPayloadSize()
{
    return cxx::align(std::max(static_cast<uint64_t>(MemoryManager::sizeWithChunkHeaderStruct(sizeof(T))),
                               posix::Allocator::MEMORY_ALIGNMENT),
                      MemPool::MEMORY_ALIGNMENT);
}

template <typename T>
uint64_t TypedMemPool<T>::requiredManagementMemorySize(const uint64_t f_numberOfChunks)
{
    return f_numberOfChunks * sizeof(ChunkManagement)
           + 2 * cxx::align(MemPool::freeList_t::requiredMemorySize(f_numberOfChunks), 32ul);
}

template <typename T>
uint64_t TypedMemPool<T>::requiredChunkMemorySize(const uint64_t f_numberOfChunks)
{
    return f_numberOfChunks * getAdjustedPayloadSize();
}

template <typename T>
uint64_t TypedMemPool<T>::requiredFullMemorySize(const uint64_t f_numberOfChunks)
{
    return requiredManagementMemorySize(f_numberOfChunks) + requiredChunkMemorySize(f_numberOfChunks);
}

} // namespace mepoo
} // namespace iox
