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

#ifndef IOX_POSH_POPO_USED_CHUNK_LIST_INL
#define IOX_POSH_POPO_USED_CHUNK_LIST_INL

namespace iox
{
namespace popo
{
template <uint32_t Size>
UsedChunkList<Size>::UsedChunkList()
{
    init();
}

template <uint32_t Size>
bool UsedChunkList<Size>::insert(mepoo::SharedChunk chunk)
{
    if (freeSpaceInList())
    {
        // get next free entry after freelistHead
        auto nextFree = m_listNodes[m_freeListHead];

        // freeListHead is getting new usedListHead, next of this entry is updated to next in usedList
        m_listNodes[m_freeListHead] = m_usedListHead;
        m_usedListHead = m_freeListHead;

        // store chunk mgmt ptr
        m_listData[m_usedListHead] = chunk.release();

        // set freeListHead to the next free entry
        m_freeListHead = nextFree;

        /// @todo can we do this cheaper with a global fence in cleanup?
        m_synchronizer.clear(std::memory_order_release);
        return true;
    }
    else
    {
        return false;
    }
}

template <uint32_t Size>
bool UsedChunkList<Size>::remove(const mepoo::ChunkHeader* chunkHeader, mepoo::SharedChunk& chunk)
{
    auto previous = InvalidIndex;

    // go through usedList with stored chunks
    for (auto current = m_usedListHead; current != InvalidIndex; current = m_listNodes[current])
    {
        // does the entry match the one we want to remove?
        if (m_listData[current] != nullptr && m_listData[current]->m_chunkHeader == chunkHeader)
        {
            // return the chunk mgmt entry as SharedChunk object
            chunk = mepoo::SharedChunk(m_listData[current]);
            m_listData[current] = nullptr;

            // remove index from used list
            if (current == m_usedListHead)
            {
                m_usedListHead = m_listNodes[current];
            }
            else
            {
                m_listNodes[previous] = m_listNodes[current];
            }

            // insert index to free list
            m_listNodes[current] = m_freeListHead;
            m_freeListHead = current;

            /// @todo can we do this cheaper with a global fence in cleanup?
            m_synchronizer.clear(std::memory_order_release);
            return true;
        }
        previous = current;
    }
    return false;
}

template <uint32_t Size>
void UsedChunkList<Size>::cleanup()
{
    m_synchronizer.test_and_set(std::memory_order_acquire);

    for (auto& data : m_listData)
    {
        if (data != nullptr)
        {
            mepoo::SharedChunk{data};
        }
    }

    init(); // just to save us from the future self
}

template <uint32_t Size>
void UsedChunkList<Size>::init()
{
    // build list
    for (uint32_t i = 0U; i < Size; ++i)
    {
        m_listNodes[i] = i + 1u;
    }

    if (Size > 0U)
    {
        m_listNodes[Size - 1U] = InvalidIndex; // just to save us from the future self
    }
    else
    {
        m_listNodes[0U] = InvalidIndex;
    }


    m_usedListHead = InvalidIndex;
    m_freeListHead = 0U;

    // clear data
    for (auto& data : m_listData)
    {
        data = nullptr;
    }

    m_synchronizer.clear(std::memory_order_release);
}

template <uint32_t Size>
bool UsedChunkList<Size>::freeSpaceInList()
{
    return (m_freeListHead != InvalidIndex);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_USED_CHUNK_LIST_INL
