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

#include "iceoryx_posh/popo/client_options.hpp"
#include "iox/logging.hpp"

namespace iox
{
namespace popo
{
Serialization ClientOptions::serialize() const noexcept
{
    return Serialization::create(responseQueueCapacity,
                                 nodeName,
                                 connectOnCreate,
                                 static_cast<std::underlying_type_t<QueueFullPolicy>>(responseQueueFullPolicy),
                                 static_cast<std::underlying_type_t<ConsumerTooSlowPolicy>>(serverTooSlowPolicy));
}

expected<ClientOptions, Serialization::Error> ClientOptions::deserialize(const Serialization& serialized) noexcept
{
    using QueueFullPolicyUT = std::underlying_type_t<QueueFullPolicy>;
    using ConsumerTooSlowPolicyUT = std::underlying_type_t<ConsumerTooSlowPolicy>;

    ClientOptions clientOptions{};
    QueueFullPolicyUT responseQueueFullPolicy{};
    ConsumerTooSlowPolicyUT serverTooSlowPolicy{};

    auto deserializationSuccessful = serialized.extract(clientOptions.responseQueueCapacity,
                                                        clientOptions.nodeName,
                                                        clientOptions.connectOnCreate,
                                                        responseQueueFullPolicy,
                                                        serverTooSlowPolicy);

    if (!deserializationSuccessful
        || responseQueueFullPolicy > static_cast<QueueFullPolicyUT>(QueueFullPolicy::DISCARD_OLDEST_DATA)
        || serverTooSlowPolicy > static_cast<ConsumerTooSlowPolicyUT>(ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA))
    {
        return err(Serialization::Error::DESERIALIZATION_FAILED);
    }

    clientOptions.responseQueueFullPolicy = static_cast<QueueFullPolicy>(responseQueueFullPolicy);
    clientOptions.serverTooSlowPolicy = static_cast<ConsumerTooSlowPolicy>(serverTooSlowPolicy);
    return ok(clientOptions);
}

bool ClientOptions::operator==(const ClientOptions& rhs) const noexcept
{
    return responseQueueCapacity == rhs.responseQueueCapacity && nodeName == rhs.nodeName
           && connectOnCreate == rhs.connectOnCreate && responseQueueFullPolicy == rhs.responseQueueFullPolicy
           && serverTooSlowPolicy == rhs.serverTooSlowPolicy;
}
} // namespace popo
} // namespace iox
