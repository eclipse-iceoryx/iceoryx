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

#ifndef IOX_POSH_POPO_UNTYPED_SERVER_IMPL_HPP
#define IOX_POSH_POPO_UNTYPED_SERVER_IMPL_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/base_server.hpp"
#include "iceoryx_posh/popo/server_options.hpp"

namespace iox
{
namespace popo
{
/// @brief The UntypedServerImpl class implements the untyped server API
/// @note Not intended for public usage! Use the 'UntypedServer' instead!
template <typename BaseServerT = BaseServer<>>
class UntypedServerImpl : public BaseServerT
{
  public:
    explicit UntypedServerImpl(const capro::ServiceDescription& service,
                               const ServerOptions& serverOptions = {}) noexcept;
    virtual ~UntypedServerImpl() noexcept;

    UntypedServerImpl(const UntypedServerImpl&) = delete;
    UntypedServerImpl(UntypedServerImpl&&) = delete;
    UntypedServerImpl& operator=(const UntypedServerImpl&) = delete;
    UntypedServerImpl& operator=(UntypedServerImpl&&) = delete;

    /// @brief Take the request chunk from the top of the receive queue.
    /// @return The payload pointer of the request chunk taken.
    /// @details No automatic cleanup of the associated chunk is performed
    ///          and must be manually done by calling 'releaseRequest'
    expected<const void*, ServerRequestResult> take() noexcept;

    /// @brief Releases the ownership of the request chunk provided by the payload pointer.
    /// @param requestPayload pointer to the payload of the chunk to be released
    /// @details The requestPayload pointer must have been previously provided by 'take'
    ///          and not have been already released. The chunk must not be accessed afterwards
    ///          as its memory may have been reclaimed.
    void releaseRequest(const void* const requestPayload) noexcept;

    /// @brief Get a response chunk from loaned shared memory.
    /// @param[in] requestHeader The requestHeader to which the response belongs to, to determine where to send the
    /// response
    /// @param payloadSize The expected payload size of the chunk.
    /// @param payloadAlignment The expected payload alignment of the chunk.
    /// @return A pointer to the payload of a chunk of memory with the requested size or
    ///         an AllocationError if no chunk could be loaned.
    /// @note An AllocationError occurs if no chunk is available in the shared memory.
    expected<void*, AllocationError> loan(const RequestHeader* const requestHeader,
                                          const uint64_t payloadSize,
                                          const uint32_t payloadAlignment) noexcept;

    /// @brief Sends the provided memory chunk as response to the client.
    /// @param responsePayload Pointer to the payload of the allocated shared memory chunk.
    /// @return Error if sending was not successful
    expected<void, ServerSendError> send(void* const responsePayload) noexcept;

    /// @brief Releases the ownership of the response chunk provided by the payload pointer.
    /// @param responsePayload pointer to the payload of the chunk to be released
    /// @details The responsePayload pointer must have been previously provided by 'loan'
    ///          and not have been already released. The chunk must not be accessed afterwards
    ///          as its memory may have been reclaimed.
    void releaseResponse(void* const responsePayload) noexcept;

  protected:
    using BaseServerT::port;
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_server_impl.inl"

#endif // IOX_POSH_POPO_UNTYPED_SERVER_IMPL_HPP
