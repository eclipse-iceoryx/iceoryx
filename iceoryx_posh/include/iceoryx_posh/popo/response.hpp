// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_RESPONSE_HPP
#define IOX_POSH_POPO_RESPONSE_HPP

#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"
#include "iceoryx_posh/internal/popo/smart_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/popo/rpc_header.hpp"
#include "iox/logging.hpp"
#include "iox/type_traits.hpp"
#include "iox/unique_ptr.hpp"


namespace iox
{
namespace popo
{
template <typename RpcType, typename SendErrorEnum>
class RpcInterface;

/// @brief The Response class is a mutable abstraction over types which are written to loaned shared memory.
/// These responses are sent to the client via the iceoryx system.
template <typename T>
class Response
    : public SmartChunk<RpcInterface<Response<T>, ServerSendError>, T, add_const_conditionally_t<ResponseHeader, T>>
{
    using BaseType =
        SmartChunk<RpcInterface<Response<T>, ServerSendError>, T, add_const_conditionally_t<ResponseHeader, T>>;

    template <typename S, typename TT>
    using ForServerOnly = typename BaseType::template ForProducerOnly<S, TT>;

  public:
    /// @brief Constructor for a Response used by the server/client
    /// @param smartChunkUniquePtr is a 'rvalue' to a 'iox::unique_ptr<T>' with to the data of the encapsulated type
    /// T
    /// @param producer (for server only) is a reference to the server to be able to use server specific methods
    using BaseType::BaseType;

    /// @brief Sends the response via the server from which it was loaned and automatically
    /// release ownership to it.
    /// @details Only available for server (non-const type T)
    template <typename S = T, typename = ForServerOnly<S, T>>
    expected<void, ServerSendError> send() noexcept;

    /// @brief Retrieve the response-header of the underlying memory chunk loaned to the sample.
    /// @return The response-header of the underlying memory chunk.
    add_const_conditionally_t<ResponseHeader, T>& getResponseHeader() noexcept;

    /// @brief Retrieve the response-header of the underlying memory chunk loaned to the sample.
    /// @return The response-header of the underlying memory chunk.
    const ResponseHeader& getResponseHeader() const noexcept;

  private:
    template <typename, typename, typename>
    friend class ClientImpl;
    template <typename, typename, typename>
    friend class ServerImpl;

    using BaseType::release;

    using BaseType::m_members;
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/response.inl"

#endif
