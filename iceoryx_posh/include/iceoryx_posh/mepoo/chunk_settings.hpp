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

#ifndef IOX_POSH_MEPOO_CHUNK_SETTINGS_HPP
#define IOX_POSH_MEPOO_CHUNK_SETTINGS_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/cxx/expected.hpp"

#include <cstdint>

namespace iox
{
namespace mepoo
{
class ChunkSettings
{
  public:
    enum class Error
    {
        ALIGNMENT_NOT_POWER_OF_TWO,
        CUSTOM_HEADER_ALIGNMENT_EXCEEDS_CHUNK_HEADER_ALIGNMENT,
        CUSTOM_HEADER_SIZE_NOT_MULTIPLE_OF_ITS_ALIGNMENT,
        REQUIRED_CHUNK_SIZE_EXCEEDS_MAX_CHUNK_SIZE,
        INVALID_STATE
    };

    /// @brief constructs and initializes a ChunkSettings
    /// @param[in] chunkSize is the size of the chunk fulfilling the payload and custom header requirements
    /// @param[in] payloadSize is the size of the user payload
    /// @param[in] payloadAlignment is the alignment of the user payload
    /// @param[in] customHeaderSize is the size of the custom header custom header
    /// @param[in] customHeaderAlignment is the alignment for the custom header
    static cxx::expected<ChunkSettings, ChunkSettings::Error>
    create(const uint32_t payloadSize,
           const uint32_t payloadAlignment = iox::CHUNK_DEFAULT_PAYLOAD_ALIGNMENT,
           const uint32_t customHeaderSize = iox::CHUNK_NO_CUSTOM_HEADER_SIZE,
           const uint32_t customHeaderAlignment = iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT) noexcept;

    /// @brief getter method for the chunk size fulfilling the payload and custom header requirements
    /// @return the chunk size
    uint32_t requiredChunkSize() const noexcept;

    /// @brief getter method for the user payload size
    /// @return the user payload size
    uint32_t payloadSize() const noexcept;

    /// @brief getter method for the user payload alignment
    /// @return the user payload alignment
    uint32_t payloadAlignment() const noexcept;

    /// @brief getter method for the custom header size
    /// @return the custom header size
    uint32_t customHeaderSize() const noexcept;

    /// @brief getter method for the custom header alignment
    /// @return the custom header alignment
    uint32_t customHeaderAlignment() const noexcept;

  private:
    ChunkSettings(const uint32_t payloadSize,
                  const uint32_t payloadAlignment,
                  const uint32_t customHeaderSize,
                  const uint32_t customHeaderAlignment,
                  const uint32_t requiredChunkSize) noexcept;

    static uint64_t calculateRequiredChunkSize(const uint32_t payloadSize,
                                               const uint32_t payloadAlignment,
                                               const uint32_t customHeaderSize) noexcept;

  private:
    uint32_t m_payloadSize{0U};
    uint32_t m_payloadAlignment{0U};
    uint32_t m_customHeaderSize{0U};
    uint32_t m_customHeaderAlignment{0U};
    uint32_t m_requiredChunkSize{0U};
};

} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_CHUNK_SETTINGS_HPP
