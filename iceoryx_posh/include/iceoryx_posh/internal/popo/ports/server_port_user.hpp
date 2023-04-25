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
#ifndef IOX_POSH_POPO_PORTS_SERVER_PORT_USER_HPP
#define IOX_POSH_POPO_PORTS_SERVER_PORT_USER_HPP

#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_receiver.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_sender.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_data.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/popo/rpc_header.hpp"
#include "iox/expected.hpp"
#include "iox/into.hpp"
#include "iox/optional.hpp"

namespace iox
{
namespace popo
{
enum class ServerRequestResult
{
    TOO_MANY_REQUESTS_HELD_IN_PARALLEL,
    NO_PENDING_REQUESTS,
    UNDEFINED_CHUNK_RECEIVE_ERROR,
    NO_PENDING_REQUESTS_AND_SERVER_DOES_NOT_OFFER,
};

/// @brief Converts the ServerRequestResult to a string literal
/// @param[in] value to convert to a string literal
/// @return pointer to a string literal
inline constexpr const char* asStringLiteral(const ServerRequestResult value) noexcept;

/// @brief Convenience stream operator to easily use the 'asStringLiteral' function with std::ostream
/// @param[in] stream sink to write the message to
/// @param[in] value to convert to a string literal
/// @return the reference to 'stream' which was provided as input parameter
inline std::ostream& operator<<(std::ostream& stream, ServerRequestResult value) noexcept;

/// @brief Convenience stream operator to easily use the 'asStringLiteral' function with iox::log::LogStream
/// @param[in] stream sink to write the message to
/// @param[in] value to convert to a string literal
/// @return the reference to 'stream' which was provided as input parameter
inline log::LogStream& operator<<(log::LogStream& stream, ServerRequestResult value) noexcept;
} // namespace popo

template <>
constexpr popo::ServerRequestResult
from<popo::ChunkReceiveResult, popo::ServerRequestResult>(const popo::ChunkReceiveResult value) noexcept;

namespace popo
{
enum class ServerSendError
{
    NOT_OFFERED,
    CLIENT_NOT_AVAILABLE,
    INVALID_RESPONSE,
};

/// @brief Converts the ServerSendError to a string literal
/// @param[in] value to convert to a string literal
/// @return pointer to a string literal
inline constexpr const char* asStringLiteral(const ServerSendError value) noexcept;

/// @brief Convenience stream operator to easily use the 'asStringLiteral' function with std::ostream
/// @param[in] stream sink to write the message to
/// @param[in] value to convert to a string literal
/// @return the reference to 'stream' which was provided as input parameter
inline std::ostream& operator<<(std::ostream& stream, ServerSendError value) noexcept;

/// @brief Convenience stream operator to easily use the 'asStringLiteral' function with iox::log::LogStream
/// @param[in] stream sink to write the message to
/// @param[in] value to convert to a string literal
/// @return the reference to 'stream' which was provided as input parameter
inline log::LogStream& operator<<(log::LogStream& stream, ServerSendError value) noexcept;

/// @brief The ServerPortUser provides the API for accessing a server port from the user side. The server port
/// is divided in the three parts ServerPortData, ServerPortRouDi and ServerPortUser. The ServerPortUser
/// uses the functionality of a ChunkSender and ChunReceiver for receiving requests and sending responses.
/// Additionally it provides the offer / stopOffer API which controls whether the server is discoverable
/// for client ports
class ServerPortUser : public BasePort
{
  public:
    using MemberType_t = ServerPortData;

    explicit ServerPortUser(MemberType_t& serverPortData) noexcept;

    ServerPortUser(const ServerPortUser& other) = delete;
    ServerPortUser& operator=(const ServerPortUser&) = delete;
    ServerPortUser(ServerPortUser&& rhs) noexcept = default;
    ServerPortUser& operator=(ServerPortUser&& rhs) noexcept = default;
    ~ServerPortUser() = default;

    /// @brief Tries to get the next request from the queue. If there is a new one, the ChunkHeader of the oldest
    /// request in the queue is returned (FiFo queue)
    /// @return expected that has a new RequestHeader if there are new requests in the underlying queue,
    /// ServerRequestResult on error
    expected<const RequestHeader*, ServerRequestResult> getRequest() noexcept;

    /// @brief Release a request that was obtained with getRequest
    /// @param[in] chunkHeader, pointer to the ChunkHeader to release
    void releaseRequest(const RequestHeader* const requestHeader) noexcept;

    /// @brief Release all the requests that are currently queued up.
    void releaseQueuedRequests() noexcept;

    /// @brief check if there are requests in the queue
    /// @return if there are requests in the queue return true, otherwise false
    bool hasNewRequests() const noexcept;

    /// @brief check if there was a queue overflow since the last call of hasLostRequestsSinceLastCall
    /// @return true if the underlying queue overflowed since last call of this method, otherwise false
    bool hasLostRequestsSinceLastCall() noexcept;

    /// @brief Allocate a response, the ownerhip of the SharedChunk remains in the ServerPortUser for being able to
    /// cleanup if the user process disappears
    /// @param[in] requestHeader, the request header for the corresponding response
    /// @param[in] userPayloadSize, size of the user user-paylaod without additional headers
    /// @param[in] userPayloadAlignment, alignment of the user user-paylaod without additional headers
    /// @return on success pointer to a ChunkHeader which can be used to access the chunk-header, user-header and
    /// user-payload fields, error if not
    expected<ResponseHeader*, AllocationError> allocateResponse(const RequestHeader* const requestHeader,
                                                                const uint32_t userPayloadSize,
                                                                const uint32_t userPayloadAlignment) noexcept;

    /// @brief Releases an allocated response without sending it
    /// @param[in] chunkHeader, pointer to the ChunkHeader to free
    void releaseResponse(const ResponseHeader* const responseHeader) noexcept;

    /// @brief Send an allocated request chunk to the server port
    /// @param[in] chunkHeader, pointer to the ChunkHeader to send
    /// @return ServerSendError if sending was not successful
    expected<void, ServerSendError> sendResponse(ResponseHeader* const responseHeader) noexcept;

    /// @brief offer this server port in the system
    void offer() noexcept;

    /// @brief stop offering this server port, all clients will be disconnected from this server
    void stopOffer() noexcept;

    /// @brief Checks whether the server port is currently offered
    /// @return true if currently offered otherwise false
    bool isOffered() const noexcept;

    /// @brief Checks whether there are currently clients connected to this server
    /// @return true if there are clients otherwise false
    bool hasClients() const noexcept;

    /// @brief set a condition variable (via its pointer) to the client
    /// @return true if attachment worked, otherwise false
    void setConditionVariable(ConditionVariableData& conditionVariableData, const uint64_t notificationIndex) noexcept;

    /// @brief unset a condition variable from the client
    void unsetConditionVariable() noexcept;

    /// @brief check if there's a condition variable set
    /// @return true if a condition variable attached, otherwise false
    bool isConditionVariableSet() const noexcept;

  private:
    const MemberType_t* getMembers() const noexcept;
    MemberType_t* getMembers() noexcept;

    ChunkSender<ServerChunkSenderData_t> m_chunkSender;
    ChunkReceiver<ServerChunkReceiverData_t> m_chunkReceiver;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/ports/server_port_user.inl"

#endif // IOX_POSH_POPO_PORTS_PUBLISHER_PORT_USER_HPP
