// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_PORT_POOL_INL
#define IOX_POSH_ROUDI_PORT_POOL_INL

namespace iox
{
namespace roudi
{
template <typename T, cxx::enable_if_t<std::is_same<T, iox::build::ManyToManyPolicy>::value>*>
inline iox::popo::SubscriberPortData* PortPool::constructSubscriber(const capro::ServiceDescription& serviceDescription,
                                                                    const uint64_t& historyRequest,
                                                                    const ProcessName_t& applicationName,
                                                                    const mepoo::MemoryInfo& memoryInfo) noexcept
{
    return m_portPoolData->m_subscriberPortMembers.insert(serviceDescription,
                                                          applicationName,
                                                          cxx::VariantQueueTypes::SoFi_MultiProducerSingleConsumer,
                                                          historyRequest,
                                                          memoryInfo);
}

template <typename T, cxx::enable_if_t<std::is_same<T, iox::build::OneToManyPolicy>::value>*>
inline iox::popo::SubscriberPortData* PortPool::constructSubscriber(const capro::ServiceDescription& serviceDescription,
                                                                    const uint64_t& historyRequest,
                                                                    const ProcessName_t& applicationName,
                                                                    const mepoo::MemoryInfo& memoryInfo) noexcept
{
    return m_portPoolData->m_subscriberPortMembers.insert(serviceDescription,
                                                          applicationName,
                                                          cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer,
                                                          historyRequest,
                                                          memoryInfo);
}
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_PORT_POOL_INL
