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

#include "iceoryx_posh/popo/publisher_options.hpp"
#include "iceoryx_hoofs/log/logging.hpp"

namespace iox
{
namespace popo
{
cxx::Serialization PublisherOptions::serialize() const noexcept
{
    return cxx::Serialization::create(
        historyCapacity,
        nodeName,
        offerOnCreate,
        static_cast<std::underlying_type_t<ConsumerTooSlowPolicy>>(subscriberTooSlowPolicy));
}

cxx::expected<PublisherOptions, cxx::Serialization::Error>
PublisherOptions::deserialize(const cxx::Serialization& serialized) noexcept
{
    using ConsumerTooSlowPolicyUT = std::underlying_type_t<ConsumerTooSlowPolicy>;

    PublisherOptions publisherOptions;
    ConsumerTooSlowPolicyUT subscriberTooSlowPolicy;

    auto deserializationSuccessful = serialized.extract(publisherOptions.historyCapacity,
                                                        publisherOptions.nodeName,
                                                        publisherOptions.offerOnCreate,
                                                        subscriberTooSlowPolicy);

    if (!deserializationSuccessful
        || subscriberTooSlowPolicy > static_cast<ConsumerTooSlowPolicyUT>(ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA))
    {
        return cxx::error<cxx::Serialization::Error>(cxx::Serialization::Error::DESERIALIZATION_FAILED);
    }

    publisherOptions.subscriberTooSlowPolicy = static_cast<ConsumerTooSlowPolicy>(subscriberTooSlowPolicy);
    return cxx::success<PublisherOptions>(publisherOptions);
}
} // namespace popo
} // namespace iox
