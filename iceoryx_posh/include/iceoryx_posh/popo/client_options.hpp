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

#ifndef IOX_POSH_POPO_CLIENT_OPTIONS_HPP
#define IOX_POSH_POPO_CLIENT_OPTIONS_HPP

#include "iceoryx_posh/internal/popo/ports/client_server_port_types.hpp"
#include "iceoryx_posh/popo/port_queue_policies.hpp"

#include "iceoryx_dust/cxx/serialization.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
/// @brief This struct is used to configure the client
struct ClientOptions
{
    /// @brief The size of the response queue where chunks are stored before they are passed to the user
    /// @attention Depending on the underlying queue there can be a different overflow behavior
    uint64_t responseQueueCapacity{ClientChunkQueueData_t::MAX_CAPACITY};

    /// @brief The name of the node where the client should belong to
    iox::NodeName_t nodeName{""};

    /// @brief The option whether the client shall try to connect when creating it
    bool connectOnCreate{true};

    /// @brief The option whether the server should block when the response queue is full
    /// @note Corresponds with ServerOptions::clientTooSlowPolicy
    QueueFullPolicy responseQueueFullPolicy{QueueFullPolicy::DISCARD_OLDEST_DATA};

    /// @brief The option whether the client should block when the request queue is full
    /// @note Corresponds with ServerOptions::requestQueueFullPolicy
    ConsumerTooSlowPolicy serverTooSlowPolicy{ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA};

    /// @brief serialization of the ClientOptions
    cxx::Serialization serialize() const noexcept;
    /// @brief deserialization of the ClientOptions
    static expected<ClientOptions, cxx::Serialization::Error>
    deserialize(const cxx::Serialization& serialized) noexcept;

    /// @brief comparison operator
    /// @param[in] rhs the right hand side of the comparison
    bool operator==(const ClientOptions& rhs) const noexcept;
};

} // namespace popo
} // namespace iox
#endif // IOX_POSH_POPO_CLIENT_OPTIONS_HPP
