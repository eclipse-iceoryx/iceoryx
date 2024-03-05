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

#ifndef IOX_POSH_POPO_SERVER_OPTIONS_HPP
#define IOX_POSH_POPO_SERVER_OPTIONS_HPP

#include "iceoryx_posh/internal/popo/ports/client_server_port_types.hpp"
#include "iceoryx_posh/popo/port_queue_policies.hpp"

#include "iox/detail/serialization.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
/// @brief This struct is used to configure the server
struct ServerOptions
{
    /// @brief The size of the request queue where chunks are stored before they are passed to the user
    /// @attention Depending on the underlying queue there can be a different overflow behavior
    uint64_t requestQueueCapacity{ServerChunkQueueData_t::MAX_CAPACITY};

    /// @brief The name of the node where the server should belong to
    /// @deprecated the 'nodeName' is not used with the current stable API
    iox::NodeName_t nodeName{""};

    /// @brief The option whether the server should already be offered when creating it
    bool offerOnCreate{true};

    /// @brief The option whether the client should block when the request queue is full
    /// @note Corresponds with ClientOptions::serverTooSlowPolicy
    QueueFullPolicy requestQueueFullPolicy{QueueFullPolicy::DISCARD_OLDEST_DATA};

    /// @brief The option whether the server should block when the response queue is full
    /// @note Corresponds with ClientOptions::responseQueueFullPolicy
    ConsumerTooSlowPolicy clientTooSlowPolicy{ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA};

    /// @brief serialization of the ServerOptions
    Serialization serialize() const noexcept;
    /// @brief deserialization of the ServerOptions
    static expected<ServerOptions, Serialization::Error> deserialize(const Serialization& serialized) noexcept;

    /// @brief comparison operator
    /// @param[in] rhs the right hand side of the comparison
    bool operator==(const ServerOptions& rhs) const noexcept;
};

} // namespace popo
} // namespace iox
#endif // IOX_POSH_POPO_CLIENT_OPTIONS_HPP
