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

#ifndef IOX_DDS_GATEWAY_MOCKS_CHUNK_MOCK_HPP
#define IOX_DDS_GATEWAY_MOCKS_CHUNK_MOCK_HPP

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

#include <cstdlib>
#include <cstring>

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
#include <malloc.h>
#endif

///
/// This mock is an adapted copy from iceoryx_posh.
/// Shared mocks across modules should be exported to a library for usage across all modules to avoid copying.
/// When this is done, this (and all other) copies should be deleted, and mocks from the shaared library should be used.
///
template <typename T>
class ChunkMockDDS
{
  public:
    ChunkMockDDS(T val)
    {
#if defined(QNX) || defined(QNX__) || defined(__QNX__)
        m_rawMemory = static_cast<uint8_t*>(memalign(Alignment, Size));
#elif defined(_WIN32)
        m_rawMemory = static_cast<uint8_t*>(_aligned_malloc(Alignment, Size));
#else
        m_rawMemory = static_cast<uint8_t*>(aligned_alloc(Alignment, Size));
#endif
        assert(m_rawMemory != nullptr && "Could not get aligned memory");

        memset(m_rawMemory, 0xFF, Size);

        auto chunkSettingsResult = iox::mepoo::ChunkSettings::create(sizeof(T), alignof(T));
        iox::cxx::Ensures(!chunkSettingsResult.has_error() && "Invalid parameter for ChunkMock");
        auto& chunkSettings = chunkSettingsResult.value();

        m_chunkHeader = new (m_rawMemory) iox::mepoo::ChunkHeader(Size, chunkSettings);

        // Set the value
        auto userPayloadPtr = reinterpret_cast<T*>(m_rawMemory + sizeof(iox::mepoo::ChunkHeader));
        *userPayloadPtr = val;

        m_value = static_cast<T*>(m_chunkHeader->userPayload());
    }

    ~ChunkMockDDS()
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

    T* sample()
    {
        return m_value;
    }

    ChunkMockDDS(const ChunkMockDDS&) = delete;
    ChunkMockDDS(ChunkMockDDS&&) = delete;
    ChunkMockDDS& operator=(const ChunkMockDDS&) = delete;
    ChunkMockDDS& operator=(ChunkMockDDS&&) = delete;

  private:
    static constexpr size_t Size = sizeof(iox::mepoo::ChunkHeader) + sizeof(T);
    static constexpr size_t Alignment = iox::cxx::maxAlignment<iox::mepoo::ChunkHeader, T>();
    uint8_t* m_rawMemory{nullptr};
    iox::mepoo::ChunkHeader* m_chunkHeader = nullptr;
    T* m_value = nullptr;
};

#endif // IOX_DDS_GATEWAY_MOCKS_CHUNK_MOCK_HPP
