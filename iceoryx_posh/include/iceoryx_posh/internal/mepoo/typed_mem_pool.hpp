// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_MEPOO_TYPED_MEM_POOL_HPP
#define IOX_POSH_MEPOO_TYPED_MEM_POOL_HPP

#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/chunk_management.hpp"
#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/shared_pointer.hpp"
#include "iox/algorithm.hpp"
#include "iox/bump_allocator.hpp"
#include "iox/expected.hpp"
#include "iox/optional.hpp"
#include "iox/variant.hpp"

#include <algorithm>

namespace iox
{
namespace mepoo
{
enum class TypedMemPoolError
{
    OutOfChunks,
    FatalErrorReachedInconsistentState
};

template <typename T>
class TypedMemPool
{
  public:
    TypedMemPool(const greater_or_equal<uint32_t, 1> numberOfChunks,
                 BumpAllocator& managementAllocator,
                 BumpAllocator& chunkMemoryAllocator) noexcept;

    TypedMemPool(const TypedMemPool&) = delete;
    TypedMemPool(TypedMemPool&&) = delete;
    TypedMemPool& operator=(const TypedMemPool&) = delete;
    TypedMemPool& operator=(TypedMemPool&&) = delete;

    template <typename... Targs>
    expected<SharedPointer<T>, TypedMemPoolError> createObject(Targs&&... args) noexcept;
    template <typename ErrorType, typename... Targs>
    expected<SharedPointer<T>, variant<TypedMemPoolError, ErrorType>>
    createObjectWithCreationPattern(Targs&&... args) noexcept;
    uint32_t getChunkCount() const noexcept;
    uint32_t getUsedChunks() const noexcept;

    static uint64_t requiredManagementMemorySize(const uint64_t f_numberOfChunks) noexcept;
    static uint64_t requiredChunkMemorySize(const uint64_t f_numberOfChunks) noexcept;
    static uint64_t requiredFullMemorySize(const uint64_t f_numberOfChunks) noexcept;

  private:
    static uint64_t requiredChunkSize() noexcept;
    expected<ChunkManagement*, TypedMemPoolError> acquireChunkManagementPointer() noexcept;

  private:
    MemPool m_memPool;
    MemPool m_chunkManagementPool;
};
} // namespace mepoo
} // namespace iox


#include "typed_mem_pool.inl"

#endif // IOX_POSH_MEPOO_TYPED_MEM_POOL_HPP
