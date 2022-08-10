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

#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "iceoryx_hoofs/log/logging.hpp"

namespace iox
{
namespace popo
{
cxx::Serialization SubscriberOptions::serialize() const noexcept
{
    return cxx::Serialization::create(queueCapacity,
                                      historyRequest,
                                      nodeName,
                                      subscribeOnCreate,
                                      static_cast<std::underlying_type_t<QueueFullPolicy>>(queueFullPolicy),
                                      requiresPublisherHistorySupport);
}

cxx::expected<SubscriberOptions, cxx::Serialization::Error>
SubscriberOptions::deserialize(const cxx::Serialization& serialized) noexcept
{
    using QueueFullPolicyUT = std::underlying_type_t<QueueFullPolicy>;

    SubscriberOptions subscriberOptions;
    QueueFullPolicyUT queueFullPolicy;

    auto deserializationSuccessful = serialized.extract(subscriberOptions.queueCapacity,
                                                        subscriberOptions.historyRequest,
                                                        subscriberOptions.nodeName,
                                                        subscriberOptions.subscribeOnCreate,
                                                        queueFullPolicy,
                                                        subscriberOptions.requiresPublisherHistorySupport);

    if (!deserializationSuccessful
        || queueFullPolicy > static_cast<QueueFullPolicyUT>(QueueFullPolicy::DISCARD_OLDEST_DATA))
    {
        return cxx::error<cxx::Serialization::Error>(cxx::Serialization::Error::DESERIALIZATION_FAILED);
    }

    subscriberOptions.queueFullPolicy = static_cast<QueueFullPolicy>(queueFullPolicy);
    return cxx::success<SubscriberOptions>(subscriberOptions);
}
} // namespace popo
} // namespace iox
