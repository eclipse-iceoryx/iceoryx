// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_PORT_POOL_INL
#define IOX_POSH_ROUDI_PORT_POOL_INL

namespace iox
{
namespace roudi
{
template <typename T, std::enable_if_t<std::is_same<T, iox::build::ManyToManyPolicy>::value>*>
inline iox::popo::SubscriberPortData* PortPool::constructSubscriber(const capro::ServiceDescription& serviceDescription,
                                                                    const RuntimeName_t& runtimeName,
                                                                    const roudi::UniqueRouDiId uniqueRouDiId,
                                                                    const popo::SubscriberOptions& subscriberOptions,
                                                                    const mepoo::MemoryInfo& memoryInfo) noexcept
{
    auto port = getSubscriberPortDataList().emplace(
        serviceDescription,
        runtimeName,
        uniqueRouDiId,
        (subscriberOptions.queueFullPolicy == popo::QueueFullPolicy::DISCARD_OLDEST_DATA)
            ? popo::VariantQueueTypes::SoFi_MultiProducerSingleConsumer
            : popo::VariantQueueTypes::FiFo_MultiProducerSingleConsumer,
        subscriberOptions,
        memoryInfo);
    if (port == getSubscriberPortDataList().end())
    {
        return nullptr;
    }

    return port.to_ptr();
}

template <typename T, std::enable_if_t<std::is_same<T, iox::build::OneToManyPolicy>::value>*>
inline iox::popo::SubscriberPortData* PortPool::constructSubscriber(const capro::ServiceDescription& serviceDescription,
                                                                    const RuntimeName_t& runtimeName,
                                                                    const roudi::UniqueRouDiId uniqueRouDiId,
                                                                    const popo::SubscriberOptions& subscriberOptions,
                                                                    const mepoo::MemoryInfo& memoryInfo) noexcept
{
    auto port = getSubscriberPortDataList().emplace(
        serviceDescription,
        runtimeName,
        uniqueRouDiId,
        (subscriberOptions.queueFullPolicy == popo::QueueFullPolicy::DISCARD_OLDEST_DATA)
            ? popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer
            : popo::VariantQueueTypes::FiFo_SingleProducerSingleConsumer,
        subscriberOptions,
        memoryInfo);
    if (port == getSubscriberPortDataList().end())
    {
        return nullptr;
    }

    return port.to_ptr();
}
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_PORT_POOL_INL
