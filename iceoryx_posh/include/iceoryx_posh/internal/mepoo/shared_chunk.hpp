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

#include "iceoryx_posh/internal/mepoo/chunk_management.hpp"
#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

namespace iox
{
namespace mepoo
{
template <typename>
class SharedPointer;

/// @brief WARNING: SharedChunk is not thread safe! Don't share SharedChunk objects between threads! Use for each thread
/// a separate copy
class SharedChunk
{
  public:
    SharedChunk() = default;
    SharedChunk(ChunkManagement* const f_resource);
    SharedChunk(const relative_ptr<ChunkManagement>& f_resource);
    ~SharedChunk();

    SharedChunk(const SharedChunk& rhs);
    SharedChunk(SharedChunk&& rhs);

    SharedChunk& operator=(const SharedChunk& rhs);
    SharedChunk& operator=(SharedChunk&& rhs);

    ChunkHeader* getChunkHeader() const;
    void* getPayload() const;

    ChunkManagement* release();
    iox::relative_ptr<ChunkManagement> releaseWithRelativePtr();

    bool operator==(const SharedChunk& rhs) const;
    /// @todo use the newtype pattern to avoid the void pointer
    bool operator==(const void* const rhs) const;

    bool operator!=(const SharedChunk& rhs) const;
    bool operator!=(const void* const rhs) const;

    operator bool() const;

    bool hasNoOtherOwners() const;

    template <typename>
    friend class SharedPointer;

  private:
    void decrementReferenceCounter();
    void incrementReferenceCounter();
    void freeChunk();

  private:
    iox::relative_ptr<ChunkManagement> m_chunkManagement;
};
} // namespace mepoo
} // namespace iox
