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

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

#include <cstring>

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
#include <malloc.h>
#endif

template <typename Topic>
class ChunkMock
{
  public:
    ChunkMock()
    {
#if defined(QNX) || defined(QNX__) || defined(__QNX__)
        m_rawMemory = static_cast<uint8_t*>(memalign(Alignment, Size));
#else
        m_rawMemory = static_cast<uint8_t*>(aligned_alloc(Alignment, Size));
#endif
        assert(m_rawMemory != nullptr && "Could not get aligned memory");

        memset(m_rawMemory, 0xFF, Size);

        m_chunkHeader = new (m_rawMemory) iox::mepoo::ChunkHeader();
        m_topic = static_cast<Topic*>(m_chunkHeader->payload());
    }
    ~ChunkMock()
    {
        if (m_chunkHeader != nullptr)
        {
            m_chunkHeader->~ChunkHeader();
        }
        if (m_rawMemory != nullptr)
        {
            free(m_rawMemory);
            m_rawMemory = nullptr;
        }
    }

    iox::mepoo::ChunkHeader* chunkHeader()
    {
        return m_chunkHeader;
    }

    Topic* sample()
    {
        return m_topic;
    }

    ChunkMock(const ChunkMock&) = delete;
    ChunkMock(ChunkMock&&) = delete;
    ChunkMock& operator=(const ChunkMock&) = delete;
    ChunkMock& operator=(ChunkMock&&) = delete;

  private:
    static constexpr size_t Size = sizeof(iox::mepoo::ChunkHeader) + sizeof(Topic);
    static constexpr size_t Alignment = iox::cxx::maxAlignment<iox::mepoo::ChunkHeader, Topic>();
    uint8_t* m_rawMemory{nullptr};
    iox::mepoo::ChunkHeader* m_chunkHeader = nullptr;
    Topic* m_topic = nullptr;
};
