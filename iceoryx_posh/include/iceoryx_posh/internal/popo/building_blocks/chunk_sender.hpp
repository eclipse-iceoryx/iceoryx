// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_POPO_CHUNK_SENDER_HPP_
#define IOX_POPO_CHUNK_SENDER_HPP_

#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender_data.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

namespace iox
{
namespace popo
{
enum class ChunkSenderError
{
    RUNNING_OUT_OF_CHUNKS,
    TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL
};

/// @brief The ChunkSender is a building block of the shared memory communication infrastructure. It extends
/// the functionality of a ChunkDistributor with the abililty to allocate and free memory chunks.
/// For getting chunks of memory the MemoryManger is used. Together with the ChunkReceiver, they are the next
/// abstraction layer on top of ChunkDistributor and ChunkQueuePopper. The ChunkSender holds the ownership of the
/// SharedChunks and does a bookkeeping which chunks are currently passed to the user side.
template <typename ChunkDistributorType>
class ChunkSender : public ChunkDistributorType
{
  public:
    using MemberType_t = ChunkSenderData<typename ChunkDistributorType::MemberType_t>;

    ChunkSender(cxx::not_null<MemberType_t* const> chunkDistributorDataPtr) noexcept;

    ChunkSender(const ChunkSender& other) = delete;
    ChunkSender& operator=(const ChunkSender&) = delete;
    ChunkSender(ChunkSender&& rhs) = default;
    ChunkSender& operator=(ChunkSender&& rhs) = default;
    ~ChunkSender() = default;

    /// @brief Allocate a chunk, the ownerhip of the SharedChunk remains in the ChunkSender for being able to cleanup if
    /// the user process disappears
    /// @param[in] payloadSize, size of the user paylaod without additional headers
    /// @return on success pointer to a ChunkHeader which can be used to access the payload and header fields, error if
    /// not
    cxx::expected<mepoo::ChunkHeader*, ChunkSenderError> allocate(const uint32_t payloadSize) noexcept;

    /// @brief Free an allocated chunk without sending it
    /// @param[in] chunkHeader, pointer to the ChunkHeader to free
    void free(mepoo::ChunkHeader* const chunkHeader) noexcept;

    /// @brief Send an allocated chunk to all connected ChunkQueuePopper
    /// @param[in] chunkHeader, pointer to the ChunkHeader to send
    void send(mepoo::ChunkHeader* const chunkHeader) noexcept;

    /// @brief Push an allocated chunk to the history without sending it
    /// @param[in] chunkHeader, pointer to the ChunkHeader to push to the history
    void pushToHistory(mepoo::ChunkHeader* const chunkHeader) noexcept;

    /// @brief Returns the last sent chunk if there is one
    /// @return pointer to the ChunkHeader of the last sent Chunk if there is one, empty optional if not
    cxx::optional<const mepoo::ChunkHeader*> getLast() const noexcept;

    /// @brief Release all the chunks that are currently held. Caution: Only call this if the user process is no more
    /// running E.g. This cleans up chunks that were held by a user process that died unexpectetly, for avoiding lost
    /// chunks in the system
    void releaseAll() noexcept;

  private:
    /// @brief Get the SharedChunk from the provided ChunkHeader and do all that is required to send the chunk
    /// @param[in] chunkHeader of the chunk that shall be send
    /// @param[in][out] chunk that corresponds to the chunk header
    /// @return true if there was a matching chunk with this header, false if not
    bool getChunkReadyForSend(mepoo::ChunkHeader* chunkHeader, mepoo::SharedChunk& chunk) noexcept;

    const MemberType_t* getMembers() const noexcept;
    MemberType_t* getMembers() noexcept;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.inl"

#endif
