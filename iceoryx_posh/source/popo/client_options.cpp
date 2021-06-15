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
    return cxx::Serialization::create(
        responseQueueCapacity,
        nodeName,
        connectOnCreate,
        static_cast<std::underlying_type<QueueFullPolicy2>::type>(responseQueueFullPolicy),
        static_cast<std::underlying_type<ConsumerTooSlowPolicy>::type>(serverTooSlowPolicy));
}

ClientOptions ClientOptions::deserialize(const cxx::Serialization& serialized) noexcept
{
    using QueueFullPolicyUT = std::underlying_type<QueueFullPolicy2>::type;
    using ConsumerTooSlowPolicyUT = std::underlying_type<ConsumerTooSlowPolicy>::type;

    ClientOptions clientOptions;
    QueueFullPolicyUT responseQueueFullPolicy;
    ConsumerTooSlowPolicyUT serverTooSlowPolicy;

    serialized.extract(clientOptions.responseQueueCapacity,
                       clientOptions.nodeName,
                       clientOptions.connectOnCreate,
                       responseQueueFullPolicy,
                       serverTooSlowPolicy);

    if (responseQueueFullPolicy > static_cast<QueueFullPolicyUT>(QueueFullPolicy2::DISCARD_OLDEST_DATA))
    {
        // we cannot call cxx::Ensures here since it would terminate RouDi but the cxx::Serialization
        // currently doesn't store a failed deserialization -> log warning and use sane default value
        LogWarn() << "Deserialization of ClientOptions::responseQueueFullPolicy failed! Got '"
                  << responseQueueFullPolicy << "' Using QueueFullPolicy::DISCARD_OLDEST_DATA as fallback";
        clientOptions.responseQueueFullPolicy = QueueFullPolicy2::DISCARD_OLDEST_DATA;
    }
    else
    {
        clientOptions.responseQueueFullPolicy = static_cast<QueueFullPolicy2>(responseQueueFullPolicy);
    }

    if (serverTooSlowPolicy > static_cast<ConsumerTooSlowPolicyUT>(ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA))
    {
        // we cannot call cxx::Ensures here since it would terminate RouDi but the cxx::Serialization
        // currently doesn't store a failed deserialization -> log warning and use sane default value
        LogWarn() << "Deserialization of ClientOptions::serverTooSlowPolicy failed! Got '" << serverTooSlowPolicy
                  << "' Using ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA as fallback";
        clientOptions.serverTooSlowPolicy = ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;
    }
    else
    {
        clientOptions.serverTooSlowPolicy = static_cast<ConsumerTooSlowPolicy>(serverTooSlowPolicy);
    }

    return clientOptions;
}
} // namespace popo
} // namespace iox
