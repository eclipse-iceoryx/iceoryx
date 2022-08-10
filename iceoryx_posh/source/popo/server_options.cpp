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

#include "iceoryx_posh/popo/server_options.hpp"
#include "iceoryx_hoofs/log/logging.hpp"

namespace iox
{
namespace popo
{
cxx::Serialization ServerOptions::serialize() const noexcept
{
    return cxx::Serialization::create(requestQueueCapacity,
                                      nodeName,
                                      offerOnCreate,
                                      static_cast<std::underlying_type_t<QueueFullPolicy>>(requestQueueFullPolicy),
                                      static_cast<std::underlying_type_t<ConsumerTooSlowPolicy>>(clientTooSlowPolicy));
}

cxx::expected<ServerOptions, cxx::Serialization::Error>
ServerOptions::deserialize(const cxx::Serialization& serialized) noexcept
{
    using QueueFullPolicyUT = std::underlying_type_t<QueueFullPolicy>;
    using ClientTooSlowPolicyUT = std::underlying_type_t<ConsumerTooSlowPolicy>;

    ServerOptions serverOptions;
    QueueFullPolicyUT requestQueueFullPolicy;
    ClientTooSlowPolicyUT clientTooSlowPolicy;

    auto deserializationSuccessful = serialized.extract(serverOptions.requestQueueCapacity,
                                                        serverOptions.nodeName,
                                                        serverOptions.offerOnCreate,
                                                        requestQueueFullPolicy,
                                                        clientTooSlowPolicy);

    if (!deserializationSuccessful
        || requestQueueFullPolicy > static_cast<QueueFullPolicyUT>(QueueFullPolicy::DISCARD_OLDEST_DATA)
        || clientTooSlowPolicy > static_cast<ClientTooSlowPolicyUT>(ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA))
    {
        return cxx::error<cxx::Serialization::Error>(cxx::Serialization::Error::DESERIALIZATION_FAILED);
    }

    serverOptions.requestQueueFullPolicy = static_cast<QueueFullPolicy>(requestQueueFullPolicy);
    serverOptions.clientTooSlowPolicy = static_cast<ConsumerTooSlowPolicy>(clientTooSlowPolicy);

    return cxx::success<ServerOptions>(serverOptions);
}

bool ServerOptions::operator==(const ServerOptions& rhs) const noexcept
{
    return requestQueueCapacity == rhs.requestQueueCapacity && nodeName == rhs.nodeName
           && offerOnCreate == rhs.offerOnCreate && requestQueueFullPolicy == rhs.requestQueueFullPolicy
           && clientTooSlowPolicy == rhs.clientTooSlowPolicy;
}
} // namespace popo
} // namespace iox
