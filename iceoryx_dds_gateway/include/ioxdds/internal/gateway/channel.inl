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

namespace iox
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
                                                     const SubscriberPtr subscriber,
                                                     const DataWriterPtr dataWriter) noexcept
    : m_service(service),
      m_subscriber(subscriber),
      m_dataWriter(dataWriter)
{
}

template <typename subscriber_t, typename data_writer_t>
inline Channel<subscriber_t, data_writer_t>
Channel<subscriber_t, data_writer_t>::create(const iox::capro::ServiceDescription& service) noexcept
{
    // Create objects in the pool.
    auto rawSubscriberPtr = s_subscriberPool.create(std::forward<const iox::capro::ServiceDescription>(service));
    auto rawDataWriterPtr = s_dataWriterPool.create(
        service.getServiceIDString(), service.getInstanceIDString(), service.getEventIDString());

    // Wrap in smart pointer with custom deleter to ensure automatic cleanup.
    auto subscriberPtr = SubscriberPtr(rawSubscriberPtr, [](subscriber_t* p) { s_subscriberPool.free(p); });
    auto dataWriterPtr = DataWriterPtr(rawDataWriterPtr, [](data_writer_t* p) { s_dataWriterPool.free(p); });

    return Channel(service, subscriberPtr, dataWriterPtr);
}

template <typename subscriber_t, typename data_writer_t>
inline iox::capro::ServiceDescription Channel<subscriber_t, data_writer_t>::getService() const noexcept
{
    return m_service;
}

template <typename subscriber_t, typename data_writer_t>
inline std::shared_ptr<subscriber_t> Channel<subscriber_t, data_writer_t>::getSubscriber() const noexcept
{
    return m_subscriber;
}

template <typename subscriber_t, typename data_writer_t>
inline std::shared_ptr<data_writer_t> Channel<subscriber_t, data_writer_t>::getDataWriter() const noexcept
{
    return m_dataWriter;
}

} // namespace dds
} // namespace iox
