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

#ifndef IOX_POSH_POPO_UNTYPED_CLIENT_IMPL_HPP
#define IOX_POSH_POPO_UNTYPED_CLIENT_IMPL_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/base_client.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
/// @brief The UntypedClientImpl class implements the untyped client API
/// @note Not intended for public usage! Use the 'UntypedClient' instead!
template <typename BaseClientT = BaseClient<>>
class UntypedClientImpl : public BaseClientT
{
  public:
    explicit UntypedClientImpl(const capro::ServiceDescription& service,
                               const ClientOptions& clientOptions = {}) noexcept;
    virtual ~UntypedClientImpl() noexcept;

    UntypedClientImpl(const UntypedClientImpl&) = delete;
    UntypedClientImpl(UntypedClientImpl&&) = delete;
    UntypedClientImpl& operator=(const UntypedClientImpl&) = delete;
    UntypedClientImpl& operator=(UntypedClientImpl&&) = delete;

    /// @brief Get a request chunk from loaned shared memory.
    /// @param payloadSize The expected payload size of the chunk.
    /// @param payloadAlignment The expected payload alignment of the chunk.
    /// @return A pointer to the payload of a chunk of memory with the requested size or
    ///         an AllocationError if no chunk could be loaned.
    /// @note An AllocationError occurs if no chunk is available in the shared memory.
    expected<void*, AllocationError> loan(const uint64_t payloadSize, const uint32_t payloadAlignment) noexcept;

    /// @brief Releases the ownership of the request chunk provided by the payload pointer.
    /// @param requestPayload pointer to the payload of the chunk to be released
    /// @details The requestPayload pointer must have been previously provided by 'loan'
    ///          and not have been already released. The chunk must not be accessed afterwards
    ///          as its memory may have been reclaimed.
    void releaseRequest(void* const requestPayload) noexcept;

    /// @brief Sends the provided memory chunk as request to the server.
    /// @param requestPayload Pointer to the payload of the allocated shared memory chunk.
    /// @return Error if sending was not successful
    expected<void, ClientSendError> send(void* const requestPayload) noexcept;

    /// @brief Take the response chunk from the top of the receive queue.
    /// @return The payload pointer of the request chunk taken.
    /// @details No automatic cleanup of the associated chunk is performed
    ///          and must be manually done by calling 'releaseResponse'
    expected<const void*, ChunkReceiveResult> take() noexcept;

    /// @brief Releases the ownership of the response chunk provided by the payload pointer.
    /// @param responsePayload pointer to the payload of the chunk to be released
    /// @details The responsePayload pointer must have been previously provided by 'take'
    ///          and not have been already released. The chunk must not be accessed afterwards
    ///          as its memory may have been reclaimed.
    void releaseResponse(const void* const responsePayload) noexcept;

  protected:
    using BaseClientT::port;
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_client_impl.inl"

#endif // IOX_POSH_POPO_UNTYPED_CLIENT_IMPL_HPP
