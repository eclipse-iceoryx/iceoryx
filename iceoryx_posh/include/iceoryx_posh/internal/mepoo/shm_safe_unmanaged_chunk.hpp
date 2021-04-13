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

#ifndef IOX_POSH_MEPOO_SHM_SAFE_UNMANAGED_CHUNK_HPP
#define IOX_POSH_MEPOO_SHM_SAFE_UNMANAGED_CHUNK_HPP

#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_pointer_data.hpp"

namespace iox
{
namespace mepoo
{
class ShmSafeUnmanagedChunk
{
  public:
    ShmSafeUnmanagedChunk() noexcept = default;

    /// @brief takes a SharedChunk without decrementing the chunk reference counter
    ShmSafeUnmanagedChunk(SharedChunk chunk) noexcept;

    /// @brief Creates a SharedChunk without incrementing the chunk reference counter and invalidates itself
    SharedChunk releaseToSharedChunk() noexcept;

    /// @brief Creates a SharedChunk with incrementing the chunk reference counter and does not invalidate itself
    SharedChunk duplicateToSharedChunk() noexcept;

    /// @brief Checks if the underlying RelativePointerData is logically a nullptr
    /// @return true if logically a nullptr otherwise false
    bool isLogicalNullptr() const noexcept;

    /// @brief Access to the ChunkHeader of the underlying chunk
    /// @return the pointer to the ChunkHeader of the underlying chunk or nullptr if isLogicalNullptr would return true
    const ChunkHeader* getChunkHeader() const noexcept;

  private:
    rp::RelativePointerData m_chunkManagement;
};

} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_SHM_SAFE_UNMANAGED_CHUNK_HPP
