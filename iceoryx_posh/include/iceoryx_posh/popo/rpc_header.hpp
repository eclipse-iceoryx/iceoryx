// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_POSH_POPO_RPC_HEADER_HPP
#define IOX_POSH_POPO_RPC_HEADER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
class RpcBaseHeader
{
  public:
    /// @brief Constructs and initializes a RpcBaseHeader
    /// @param[in] clientQueueUniquePortId is the UniquePortId of the port owning the client queue where the response
    /// shall be delivered
    /// @param[in] lastKnownClientQueueIndex is the last know index of the client queue in the ChunkDistributor for fast
    /// lookup
    /// @param[in] sequenceId is a custom ID to map a response to a request
    /// @param[in] rpcHeaderVersion is set by RequestHeader/ResponseHeader and should be RPC_HEADER_VERSION
    explicit RpcBaseHeader(const UniquePortId& clientQueueUniquePortId,
                           const uint32_t lastKnownClientQueueIndex,
                           const int64_t sequenceId,
                           const uint8_t rpcHeaderVersion) noexcept;

    RpcBaseHeader(const RpcBaseHeader& other) = delete;
    RpcBaseHeader& operator=(const RpcBaseHeader&) = delete;
    RpcBaseHeader(RpcBaseHeader&& rhs) = default;
    RpcBaseHeader& operator=(RpcBaseHeader&& rhs) = default;
    ~RpcBaseHeader() = default;

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
    UniquePortId m_clientQueueUniquePortId;
    int64_t m_sequenceId{0};
};

class RequestHeader : public RpcBaseHeader
{
  public:
    /// @brief Constructs and initializes a RpcBaseHeader
    /// @param[in] clientQueueUniquePortId is the UniquePortId of the port owning the client queue where the response
    /// shall be delivered
    /// @param[in] lastKnownClientQueueIndex is the last know index of the client queue in the ChunkDistributor for fast
    /// lookup
    explicit RequestHeader(const UniquePortId& clientQueueUniquePortId,
                           const uint32_t lastKnownClientQueueIndex) noexcept;

    RequestHeader(const RequestHeader& other) = delete;
    RequestHeader& operator=(const RequestHeader&) = delete;
    RequestHeader(RequestHeader&& rhs) = default;
    RequestHeader& operator=(RequestHeader&& rhs) = default;
    ~RequestHeader() = default;

    /// @brief Sets the sequence ID which is used to match a response to a request
    /// @param[in] sequenceId is a consecutive number set by the user
    /// @note The user has to set this manually if multiple requests are sent before a response is read since a server
    /// might drop a requests or process the requests out of order and therefore the responses might also be out of
    /// order
    void setSequenceId(const int64_t sequenceId) noexcept;

    /// @brief Sets the fire and forget flag which indicates that no response is expected
    void setFireAndForget() noexcept;

    /// @brief Obtains the fire and forget flag
    /// @return true if set and no response is expected, false otherwise
    bool isFireAndForget() const noexcept;

  private:
    bool m_isFireAndForget{false};
};

class ResponseHeader : public RpcBaseHeader
{
  public:
    /// @brief Constructs and initializes a RpcBaseHeader
    /// @param[in] clientQueueUniquePortId is the UniquePortId of the port owning the client queue where the response
    /// shall be delivered
    /// @param[in] lastKnownClientQueueIndex is the last know index of the client queue in the ChunkDistributor for fast
    /// lookup
    /// @param[in] sequenceId is a custom ID to map a response to a request
    explicit ResponseHeader(const UniquePortId& clientQueueUniquePortId,
                            const uint32_t lastKnownClientQueueIndex,
                            const int64_t sequenceId) noexcept;

    ResponseHeader(const ResponseHeader& other) = delete;
    ResponseHeader& operator=(const ResponseHeader&) = delete;
    ResponseHeader(ResponseHeader&& rhs) = default;
    ResponseHeader& operator=(ResponseHeader&& rhs) = default;
    ~ResponseHeader() = default;

    /// @brief Sets the server error flag
    void setServerError() noexcept;

    /// @brief Obtains the server error flag
    /// @return true if there is an error, false otherwise
    bool hasServerError() const noexcept;

  private:
    bool m_hasServerError{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_RPC_HEADER_HPP
