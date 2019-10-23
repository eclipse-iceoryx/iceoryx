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

#include "iceoryx_posh/mepoo/chunk_info.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

#include <cstring>

template <typename Topic>
class ChunkMock
{
  public:
    ChunkMock()
    {
        memset(m_rawMemory, 0xFF, Size);

        m_chunkInfo = new (m_rawMemory) iox::mepoo::ChunkInfo();
        m_topic = static_cast<Topic*>(m_chunkInfo->m_payload);
    }
    ~ChunkMock()
    {
        if (m_chunkInfo != nullptr)
        {
            m_chunkInfo->~ChunkInfo();
        }
    }

    iox::mepoo::ChunkInfo* chunkInfo()
    {
        return m_chunkInfo;
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
    static constexpr size_t Size = sizeof(iox::mepoo::ChunkInfo) + sizeof(Topic);
    alignas(iox::cxx::maxAlignment<iox::mepoo::ChunkInfo, Topic>()) uint8_t m_rawMemory[Size];
    iox::mepoo::ChunkInfo* m_chunkInfo = nullptr;
    Topic* m_topic = nullptr;
};

