// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_RPC_HEADER_HPP
#define IOX_POSH_POPO_RPC_HEADER_HPP

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iox/detail/unique_id.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
class RpcBaseHeader
{
  public:
    /// @brief Constructs and initializes a RpcBaseHeader
    /// @param[in] uniqueClientQueueId is the iox::UniqueId of the client queue where the response shall be delivered
    /// @param[in] lastKnownClientQueueIndex is the last know index of the client queue in the ChunkDistributor for fast
    /// lookup
    /// @param[in] sequenceId is a custom ID to map a response to a request
    /// @param[in] rpcHeaderVersion is set by RequestHeader/ResponseHeader and should be RPC_HEADER_VERSION
    explicit RpcBaseHeader(const UniqueId& uniqueClientQueueId,
                           const uint32_t lastKnownClientQueueIndex,
                           const int64_t sequenceId,
                           const uint8_t rpcHeaderVersion) noexcept;

    RpcBaseHeader(const RpcBaseHeader& other) = delete;
    RpcBaseHeader& operator=(const RpcBaseHeader&) = delete;
    RpcBaseHeader(RpcBaseHeader&& rhs) noexcept = default;
    RpcBaseHeader& operator=(RpcBaseHeader&& rhs) noexcept = default;
    ~RpcBaseHeader() noexcept = default;

    /// @brief From the 2.0 release onward, this must be incremented for each incompatible change, e.g.
    ///            - data width of members changes
    ///            - members are rearranged
    ///            - semantic meaning of a member changes
    ///        in any of RpcBaseHeader, RequestHeader or ResponseHeader!
    static constexpr uint8_t RPC_HEADER_VERSION{1U};

    static constexpr uint32_t UNKNOWN_CLIENT_QUEUE_INDEX{std::numeric_limits<uint32_t>::max()};
    static constexpr int64_t START_SEQUENCE_ID{0};

    /// @brief The RpcBaseHeader version is used to detect incompatibilities for record&replay functionality
    /// @return the RpcBaseHeader version
    uint8_t getRpcHeaderVersion() const noexcept;

    /// @briet Obtains the sequence ID of the RPC message
    /// @return the sequenceId of the RPC message
    int64_t getSequenceId() const noexcept;

    /// @brief Get the pointer to the ChunkHeader
    /// @return the pointer to the ChunkHeader
    mepoo::ChunkHeader* getChunkHeader() noexcept;

    /// @brief Get the const pointer to the ChunkHeader
    /// @return the const pointer to the ChunkHeader
    const mepoo::ChunkHeader* getChunkHeader() const noexcept;

    /// @brief Get the pointer to the user-payload
    /// @return the pointer to the user-payload
    void* getUserPayload() noexcept;

    /// @brief Get the const pointer to the user-payload
    /// @return the const pointer to the user-payload
    const void* getUserPayload() const noexcept;

    friend class ServerPortUser;

  protected:
    uint8_t m_rpcHeaderVersion{RPC_HEADER_VERSION};
    uint32_t m_lastKnownClientQueueIndex{UNKNOWN_CLIENT_QUEUE_INDEX};
    UniqueId m_uniqueClientQueueId;
    int64_t m_sequenceId{0};
};

class RequestHeader : public RpcBaseHeader
{
  public:
    /// @brief Constructs and initializes a RpcBaseHeader
    /// @param[in] uniqueClientQueueId is the iox::UniqueId of the client queue to which the response shall be delivered
    /// @param[in] lastKnownClientQueueIndex is the last know index of the client queue in the ChunkDistributor for fast
    /// lookup
    explicit RequestHeader(const UniqueId& uniqueClientQueueId, const uint32_t lastKnownClientQueueIndex) noexcept;

    RequestHeader(const RequestHeader& other) = delete;
    RequestHeader& operator=(const RequestHeader&) = delete;
    RequestHeader(RequestHeader&& rhs) noexcept = default;
    RequestHeader& operator=(RequestHeader&& rhs) noexcept = default;
    ~RequestHeader() noexcept = default;

    /// @brief Sets the sequence ID which is used to match a response to a request
    /// @param[in] sequenceId is a consecutive number set by the user
    /// @note The user has to set this manually if multiple requests are sent before a response is read since a server
    /// might drop a requests or process the requests out of order and therefore the responses might also be out of
    /// order
    void setSequenceId(const int64_t sequenceId) noexcept;

    static RequestHeader* fromPayload(void* const payload) noexcept;
    static const RequestHeader* fromPayload(const void* const payload) noexcept;
};

class ResponseHeader : public RpcBaseHeader
{
  public:
    /// @brief Constructs and initializes a RpcBaseHeader
    /// @param[in] uniqueClientQueueId is the iox::UniqueId of the client queue to which the response shall be delivered
    /// @param[in] lastKnownClientQueueIndex is the last know index of the client queue in the ChunkDistributor for fast
    /// lookup
    /// @param[in] sequenceId is a custom ID to map a response to a request
    explicit ResponseHeader(const UniqueId& uniqueClientQueueId,
                            const uint32_t lastKnownClientQueueIndex,
                            const int64_t sequenceId) noexcept;

    ResponseHeader(const ResponseHeader& other) = delete;
    ResponseHeader& operator=(const ResponseHeader&) = delete;
    ResponseHeader(ResponseHeader&& rhs) noexcept = default;
    ResponseHeader& operator=(ResponseHeader&& rhs) noexcept = default;
    ~ResponseHeader() noexcept = default;

    /// @brief Sets the server error flag
    void setServerError() noexcept;

    /// @brief Obtains the server error flag
    /// @return true if there is an error, false otherwise
    bool hasServerError() const noexcept;

    static ResponseHeader* fromPayload(void* const payload) noexcept;
    static const ResponseHeader* fromPayload(const void* const payload) noexcept;

  private:
    bool m_hasServerError{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_RPC_HEADER_HPP
