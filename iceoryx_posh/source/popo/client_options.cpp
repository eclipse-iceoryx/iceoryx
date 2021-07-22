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
#include "iceoryx_posh/internal/log/posh_logging.hpp"

namespace iox
{
namespace popo
{
cxx::Serialization ClientOptions::serialize() const noexcept
{
    return cxx::Serialization::create(responseQueueCapacity,
                                      nodeName,
                                      connectOnCreate,
                                      static_cast<std::underlying_type_t<QueueFullPolicy2>>(responseQueueFullPolicy),
                                      static_cast<std::underlying_type_t<ConsumerTooSlowPolicy>>(serverTooSlowPolicy));
}

cxx::expected<ClientOptions, cxx::Serialization::Error>
ClientOptions::deserialize(const cxx::Serialization& serialized) noexcept
{
    using QueueFullPolicyUT = std::underlying_type_t<QueueFullPolicy2>;
    using ConsumerTooSlowPolicyUT = std::underlying_type_t<ConsumerTooSlowPolicy>;

    ClientOptions clientOptions;
    QueueFullPolicyUT responseQueueFullPolicy;
    ConsumerTooSlowPolicyUT serverTooSlowPolicy;

    auto deserializationSuccessful = serialized.extract(clientOptions.responseQueueCapacity,
                                                        clientOptions.nodeName,
                                                        clientOptions.connectOnCreate,
                                                        responseQueueFullPolicy,
                                                        serverTooSlowPolicy);

    if (!deserializationSuccessful)
    {
        return cxx::error<cxx::Serialization::Error>(cxx::Serialization::Error::DESERIALIZATION_FAILED);
    }

    if (responseQueueFullPolicy > static_cast<QueueFullPolicyUT>(QueueFullPolicy2::DISCARD_OLDEST_DATA))
    {
        return cxx::error<cxx::Serialization::Error>(cxx::Serialization::Error::DESERIALIZATION_FAILED);
    }
    else
    {
        clientOptions.responseQueueFullPolicy = static_cast<QueueFullPolicy2>(responseQueueFullPolicy);
    }

    if (serverTooSlowPolicy > static_cast<ConsumerTooSlowPolicyUT>(ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA))
    {
        return cxx::error<cxx::Serialization::Error>(cxx::Serialization::Error::DESERIALIZATION_FAILED);
    }
    else
    {
        clientOptions.serverTooSlowPolicy = static_cast<ConsumerTooSlowPolicy>(serverTooSlowPolicy);
    }

    return cxx::success<ClientOptions>(clientOptions);
}
} // namespace popo
} // namespace iox
