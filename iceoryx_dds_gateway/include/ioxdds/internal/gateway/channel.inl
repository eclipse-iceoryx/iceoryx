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

#include "ioxdds/gateway/channel.hpp"

namespace iox
{
namespace gateway
{
namespace dds
{
// Typedefs
template <typename subscriber_t>
using SubscriberPool = iox::cxx::ObjectPool<subscriber_t, MAX_CHANNEL_NUMBER>;
template <typename data_writer_t>
using DataWriterPool = iox::cxx::ObjectPool<data_writer_t, MAX_CHANNEL_NUMBER>;

// Statics
template <typename subscriber_t, typename data_writer_t>
SubscriberPool<subscriber_t> Channel<subscriber_t, data_writer_t>::s_subscriberPool = SubscriberPool();
template <typename subscriber_t, typename data_writer_t>
DataWriterPool<data_writer_t> Channel<subscriber_t, data_writer_t>::s_dataWriterPool = DataWriterPool();

template <typename subscriber_t, typename data_writer_t>
inline Channel<subscriber_t, data_writer_t>::Channel(const iox::capro::ServiceDescription& service,
                                                     SubscriberPtr subscriber,
                                                     DataWriterPtr dataWriter)
{
    this->m_service = service;
    this->m_subscriber = subscriber;
    this->m_dataWriter = dataWriter;
}

template <typename subscriber_t, typename data_writer_t>
inline Channel<subscriber_t, data_writer_t>
Channel<subscriber_t, data_writer_t>::create(const iox::capro::ServiceDescription& service)
{
    // Create objects in the pool.
    auto rawSubscriberPtr = s_subscriberPool.create(std::forward<const iox::capro::ServiceDescription>(service));
    auto rawDataWriterPtr =
        s_dataWriterPool.create(service.getServiceIDString(),
                                service.getInstanceIDString(),
                                service.getEventIDString());

    // Wrap in smart pointer with custom deleter to ensure automatic cleanup.
    auto subscriberPtr = SubscriberPtr(rawSubscriberPtr, [](subscriber_t* p) -> void { s_subscriberPool.free(p); });
    auto dataWriterPtr = DataWriterPtr(rawDataWriterPtr, [](data_writer_t* p) -> void { s_dataWriterPool.free(p); });

    return Channel(service, subscriberPtr, dataWriterPtr);
}

template <typename subscriber_t, typename data_writer_t>
inline iox::capro::ServiceDescription Channel<subscriber_t, data_writer_t>::getService()
{
    return this->m_service;
}

template <typename subscriber_t, typename data_writer_t>
inline std::shared_ptr<subscriber_t> Channel<subscriber_t, data_writer_t>::getSubscriber()
{
    return this->m_subscriber;
}

template <typename subscriber_t, typename data_writer_t>
inline std::shared_ptr<data_writer_t> Channel<subscriber_t, data_writer_t>::getDataWriter()
{
    return this->m_dataWriter;
}

} // namespace dds
} // namespace gateway
} // namespace iox
