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

#ifndef IOX_POSH_EXPERIMENTAL_SERVER_HPP
#define IOX_POSH_EXPERIMENTAL_SERVER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"
#include "iceoryx_posh/popo/server.hpp"
#include <iceoryx_posh/popo/untyped_server.hpp>
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/unique_ptr.hpp"

namespace iox::posh::experimental
{

using iox::popo::ServerChunkQueueData_t;
using iox::popo::ConsumerTooSlowPolicy;
using iox::popo::QueueFullPolicy;
using iox::popo::Server;
using iox::popo::UntypedServer;

enum class ServerBuilderError : uint8_t
{
    OUT_OF_RESOURCES,
};

/// @brief A builder for the server
class ServerBuilder
{
  public:
    ~ServerBuilder() = default;

    ServerBuilder(const ServerBuilder& other) = delete;
    ServerBuilder& operator=(const ServerBuilder&) = delete;
    ServerBuilder(ServerBuilder&& rhs) noexcept = delete;
    ServerBuilder& operator=(ServerBuilder&& rhs) noexcept = delete;

    /// @brief The size of the request queue where chunks are stored before they are passed to the user
    IOX_BUILDER_PARAMETER(uint64_t, request_queue_capacity, ServerChunkQueueData_t::MAX_CAPACITY)

    /// @brief The option whether the server should already be offered when creating it
    IOX_BUILDER_PARAMETER(bool, offer_on_create, true)

    /// @brief The option whether the client should block when the request queue is full
    /// @note Corresponds with ClientOptions::serverTooSlowPolicy
    IOX_BUILDER_PARAMETER(QueueFullPolicy, request_queue_full_policy, QueueFullPolicy::DISCARD_OLDEST_DATA)

    /// @brief The option whether the server should block when the response queue is full
    /// @note Corresponds with ClientOptions::responseQueueFullPolicy
    IOX_BUILDER_PARAMETER(ConsumerTooSlowPolicy, client_too_slow_policy, ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA)

  public:
    /// @brief Creates a typed server instance for the server-client messaging pattern
    /// @tparam Req type of request data
    /// @tparam Res type of response data
    template <typename Req, typename Res>
    expected<unique_ptr<Server<Req, Res>>, ServerBuilderError> create() noexcept;

    /// @brief Creates an untyped server instance for the server-client messaging pattern
    expected<unique_ptr<UntypedServer>, ServerBuilderError> create() noexcept;

  private:
    friend class Node;
    explicit ServerBuilder(runtime::PoshRuntime& runtime,
                               const capro::ServiceDescription& service_description) noexcept;

  private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members) Intentionally used since the ServerBuilder is not intended to be moved
    runtime::PoshRuntime& m_runtime;
    capro::ServiceDescription m_service_description;
};

} // namespace iox::posh::experimental

#include "iox/posh/experimental/detail/server.inl"

#endif // IOX_POSH_EXPERIMENTAL_SERVER_HPP
