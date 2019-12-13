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

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"

#include <memory>

namespace iox
{
namespace popo
{

class Publisher
{
  public:
    /// @brief Constructor
    /// @param[in] service Information on service , service, instance, event Id
    /// @param[in] runnableName optional name of the runnable the publisher belongs to
    Publisher(const capro::ServiceDescription& service, const cxx::CString100& runnableName = "") noexcept;

    Publisher& operator=(const Publisher& other) = delete;
    Publisher(const Publisher& other) = delete;

    Publisher& operator=(Publisher&&) = default;
    Publisher(Publisher&& other) = default;

    virtual ~Publisher() noexcept = default;

    /// @brief Allocate memory for the chunk to be sent
    /// @param[in] payloadSize size of shared memory to be allocated
    /// @param[in] useDynamicPayloadSizes bool value of using dynamic payload size
    /// @return Information about the chunk reserved
    virtual mepoo::ChunkHeader* allocateChunkWithHeader(uint32_t payloadSize, bool useDynamicPayloadSizes = false) noexcept;

    /// @brief Allocate memory for chunk to be sent
    /// @param[in] payloadSize size of shared memory to be allocated
    /// @param[in] useDynamicPayloadSizes bool value of using dynamic payload size
    /// @return Payload of the chunk reserved
    virtual void* allocateChunk(uint32_t payloadSize, bool useDynamicPayloadSizes = false) noexcept;

    /// @brief Send the chunk and deliver it on subscription
    /// @param[in] chunkHeader Information about the chunk to be sent
    virtual void sendChunk(mepoo::ChunkHeader* const chunkHeader) noexcept;

    /// @brief Converts payload to ChunkHeader and send the chunk, deliver it on subscription
    /// @param[in] payload payload of the chunk
    virtual void sendChunk(const void* const payload) noexcept;

    /// @brief Function for deleting particular chunk from chunkcontainer
    /// @param[in] chunkHeader Information of the chunk to be removed.
    virtual void freeChunk(mepoo::ChunkHeader* const chunkHeader) noexcept;

    /// @brief Function for converting payload information to ChunkHeader , deleting particular chunk from chunkcontainer
    /// @param[in] payload payload of the chunk to be removed.
    virtual void freeChunk(void* const payload) noexcept;

    /// @brief return void pointer to the last chunk
    /// @return void pointer to last chunk
    virtual const void* getLastChunk() const noexcept;

    /// @brief Function for offering event
    void offer() noexcept;

    /// @brief Function to stop offering event
    void stopOffer() noexcept;

    /// @brief Function to check subsrcibers are available to offer event
    /// @return true if there are subscribers otherwise false
    bool hasSubscribers() noexcept;

    /// @brief Enable the functionality to send the last chunk to new subscribers
    void enableDoDeliverOnSubscription() noexcept;

  protected:
    // needed for unit testing
    Publisher() noexcept;

  protected:
    SenderPortType m_sender{nullptr};
    void* m_lastSample{nullptr};
};

} // namespace popo
} // namespace iox
