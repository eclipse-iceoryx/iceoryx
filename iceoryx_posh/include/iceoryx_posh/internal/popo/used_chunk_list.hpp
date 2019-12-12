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
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

#include <array>
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
    UsedChunkList()
    {
        init();
    }

    // only from runtime context
    bool insert(mepoo::SharedChunk f_chunk)
    {
        if (freeSpaceInList())
        {
            // get next free entry after freelistHead
            auto nextFree = m_list[m_freeListHead];

            // freeListHead is getting new usedListHead, next of this entry is updated to next in usedList
            m_list[m_freeListHead] = m_usedListHead;
            m_usedListHead = m_freeListHead;

            // store chunk mgmt ptr
            m_data[m_usedListHead] = f_chunk.release();

            // set freeListHead to the next free entry
            m_freeListHead = nextFree;

            //@todo can we do this cheaper with a global fence in cleanup?
            m_synchronizer.clear(std::memory_order_release);
            return true;
        }
        else
        {
            return false;
        }
    }

    // only from runtime context
    bool remove(const mepoo::ChunkHeader* f_chunkHeader, mepoo::SharedChunk& f_chunk)
    {
        uint32_t previous = InvalidIndex;

        // go through usedList with stored chunks
        for (uint32_t current = m_usedListHead; current != InvalidIndex; current = m_list[current])
        {
            // does the entry match the one we want to remove?
            if (m_data[current] != nullptr && m_data[current]->m_chunkHeader == f_chunkHeader)
            {
                // return the chunk mgmt entry as SharedChunk object
                f_chunk = mepoo::SharedChunk(m_data[current]);
                m_data[current] = nullptr;

                // remove index from used list
                if (current == m_usedListHead)
                {
                    m_usedListHead = m_list[current];
                }
                else
                {
                    m_list[previous] = m_list[current];
                }

                // insert index to free list
                m_list[current] = m_freeListHead;
                m_freeListHead = current;

                //@todo can we do this cheaper with a global fence in cleanup?
                m_synchronizer.clear(std::memory_order_release);
                return true;
            }
            previous = current;
        }
        return false;
    }

    // only once from runtime context
    void setup()
    {
        m_synchronizer.test_and_set(std::memory_order_acquire);
        /// @todo maybe also call init here?
    }

    // from RouDi context once the applications walked the plank
    void cleanup()
    {
        m_synchronizer.test_and_set(std::memory_order_acquire);

        for (auto& data : m_data)
        {
            if (data != nullptr)
            {
                mepoo::SharedChunk{data};
            }
        }

        init(); // just to save us from the future self
    }

  private:
    void init()
    {
        // build list
        for (uint32_t i = 0; i < Size; ++i)
        {
            m_list[i] = i + 1;
        }
        m_list[Size - 1] = InvalidIndex; // just to save us from the future self

        m_usedListHead = InvalidIndex;
        m_freeListHead = 0u;

        // clear data
        for (auto& data : m_data)
        {
            data = nullptr;
        }

        m_synchronizer.clear(std::memory_order_release);
    }

    bool freeSpaceInList()
    {
        return (m_freeListHead != InvalidIndex);
    }


  private:
    static constexpr uint32_t InvalidIndex{Size};

  private:
    std::atomic_flag m_synchronizer = ATOMIC_FLAG_INIT;
    uint32_t m_usedListHead{InvalidIndex};
    uint32_t m_freeListHead{0u};
    std::array<uint32_t, Size> m_list;
    std::array<relative_ptr<mepoo::ChunkManagement>, Size> m_data;
};

} // namespace popo
} // namespace iox
