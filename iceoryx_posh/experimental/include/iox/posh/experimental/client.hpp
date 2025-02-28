// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#ifndef IOX_POSH_EXPERIMENTAL_CLIENT_HPP
#define IOX_POSH_EXPERIMENTAL_CLIENT_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"
#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/unique_ptr.hpp"

namespace iox::posh::experimental
{

using iox::popo::Client;
using iox::popo::ClientChunkQueueData_t;
using iox::popo::ConsumerTooSlowPolicy;
using iox::popo::QueueFullPolicy;
using iox::popo::UntypedClient;

enum class ClientBuilderError : uint8_t
{
    OUT_OF_RESOURCES,
};

/// @brief A builder for the client
class ClientBuilder
{
  public:
    ~ClientBuilder() = default;

    ClientBuilder(const ClientBuilder& other) = delete;
    ClientBuilder& operator=(const ClientBuilder&) = delete;
    ClientBuilder(ClientBuilder&& rhs) noexcept = delete;
    ClientBuilder& operator=(ClientBuilder&& rhs) noexcept = delete;

    /// @brief The size of the response queue where chunks are stored before they are passed to the user
    /// @attention Depending on the underlying queue there can be a different overflow behavior
    IOX_BUILDER_PARAMETER(uint64_t, response_queue_capacity, ClientChunkQueueData_t::MAX_CAPACITY)

    /// @brief The option whether the client shall try to connect when creating it
    IOX_BUILDER_PARAMETER(bool, connect_on_create, true)

    /// @brief The option whether the server should block when the response queue is full
    /// @note Corresponds with ServerOptions::clientTooSlowPolicy
    IOX_BUILDER_PARAMETER(QueueFullPolicy, response_queue_full_policy, QueueFullPolicy::DISCARD_OLDEST_DATA)

    /// @brief The option whether the client should block when the request queue is full
    /// @note Corresponds with ServerOptions::requestQueueFullPolicy
    IOX_BUILDER_PARAMETER(ConsumerTooSlowPolicy, server_too_slow_policy, ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA)

  public:
    /// @brief Creates a typed client instance for the server-client messaging pattern
    /// @tparam Req type of request data
    /// @tparam Res type of response data
    template <typename Req, typename Res>
    expected<unique_ptr<Client<Req, Res>>, ClientBuilderError> create() noexcept;

    /// @brief Creates an untyped client instance for the server-client messaging pattern
    expected<unique_ptr<UntypedClient>, ClientBuilderError> create() noexcept;

  private:
    friend class Node;
    explicit ClientBuilder(runtime::PoshRuntime& runtime,
                           const capro::ServiceDescription& service_description) noexcept;

  private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members) Intentionally used since the ClientBuilder is not intended to be moved
    runtime::PoshRuntime& m_runtime;
    capro::ServiceDescription m_service_description;
};

} // namespace iox::posh::experimental

#include "iox/posh/experimental/detail/client.inl"

#endif // IOX_POSH_EXPERIMENTAL_CLIENT_HPP
