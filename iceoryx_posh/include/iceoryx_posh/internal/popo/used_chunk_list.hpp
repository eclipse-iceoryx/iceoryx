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
#ifndef IOX_POSH_POPO_USED_CHUNK_LIST_HPP
#define IOX_POSH_POPO_USED_CHUNK_LIST_HPP

#include "iceoryx_posh/internal/mepoo/chunk_management.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_pointer.hpp"

#include <atomic>
#include <cstdint>

namespace iox
{
namespace popo
{
template <uint32_t Size>
class UsedChunkList
{
  public:
    UsedChunkList();

    // only from runtime context
    bool insert(mepoo::SharedChunk chunk);

    // only from runtime context
    bool remove(const mepoo::ChunkHeader* chunkHeader, mepoo::SharedChunk& chunk);

    // only once from runtime context
    void setup();

    // from RouDi context once the applications walked the plank
    void cleanup();

  private:
    void init();

    bool freeSpaceInList();

  private:
    static constexpr uint32_t InvalidIndex{Size};

  private:
    std::atomic_flag m_synchronizer = ATOMIC_FLAG_INIT;
    uint32_t m_usedListHead{InvalidIndex};
    uint32_t m_freeListHead{0u};
    uint32_t m_listNodes[Size];
    rp::RelativePointer<mepoo::ChunkManagement> m_listData[Size];
};

} // namespace popo
} // namespace iox

#include "used_chunk_list.inl"

#endif // IOX_POSH_POPO_USED_CHUNK_LIST_HPP
