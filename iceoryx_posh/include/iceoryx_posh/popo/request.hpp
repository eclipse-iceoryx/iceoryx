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

#ifndef IOX_POSH_POPO_REQUEST_HPP
#define IOX_POSH_POPO_REQUEST_HPP

#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"
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

/// @brief The Request class is a mutable abstraction over types which are written to loaned shared memory.
/// These requests are sent to the server via the iceoryx system.
template <typename T>
class Request
    : public SmartChunk<RpcInterface<Request<T>, ClientSendError>, T, add_const_conditionally_t<RequestHeader, T>>
{
    using BaseType =
        SmartChunk<RpcInterface<Request<T>, ClientSendError>, T, add_const_conditionally_t<RequestHeader, T>>;

    template <typename S, typename TT>
    using ForClientOnly = typename BaseType::template ForProducerOnly<S, TT>;

  public:
    /// @brief Constructor for a Request used by the server/client
    /// @param smartChunkUniquePtr is a 'rvalue' to a 'iox::unique_ptr<T>' with to the data of the encapsulated type
    /// T
    /// @param producer (for client only) is a reference to the client to be able to use client specific methods
    using BaseType::BaseType;

    /// @brief Sends the request via the client from which it was loaned and automatically
    /// release ownership to it.
    /// @details Only available for client (non-const type T)
    template <typename S = T, typename = ForClientOnly<S, T>>
    expected<void, ClientSendError> send() noexcept;

    /// @brief Retrieve the request-header of the underlying memory chunk loaned to the sample.
    /// @return The request-header of the underlying memory chunk.
    add_const_conditionally_t<RequestHeader, T>& getRequestHeader() noexcept;

    /// @brief Retrieve the request-header of the underlying memory chunk loaned to the sample.
    /// @return The request-header of the underlying memory chunk.
    const RequestHeader& getRequestHeader() const noexcept;

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

#include "iceoryx_posh/internal/popo/request.inl"

#endif
