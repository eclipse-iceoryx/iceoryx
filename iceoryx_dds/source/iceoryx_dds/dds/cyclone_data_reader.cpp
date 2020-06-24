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

#include "iceoryx_dds/dds/cyclone_context.hpp"
#include "iceoryx_dds/dds/cyclone_data_reader.hpp"
#include "iceoryx_dds/internal/log/logging.hpp"

iox::dds::CycloneDataReader::CycloneDataReader(IdString serviceId, IdString instanceId, IdString eventId) noexcept
    : m_serviceId(serviceId)
    , m_instanceId(instanceId)
    , m_eventId(eventId)
{
    LogDebug() << "[CycloneDataReader] Created CycloneDataReader.";
}

iox::dds::CycloneDataReader::~CycloneDataReader()
{
    LogDebug() << "[CycloneDataReader] Destroyed CycloneDataReader.";
}

void iox::dds::CycloneDataReader::connect() noexcept
{
    if(!m_isConnected.load(std::memory_order_relaxed))
    {
        auto topicString = "/" + std::string(m_serviceId) + "/" + std::string(m_instanceId) + "/" + std::string(m_eventId);
        m_topic = ::dds::topic::Topic<Mempool::Chunk>(CycloneContext::getParticipant(), topicString);
        m_subscriber = ::dds::sub::Subscriber(CycloneContext::getParticipant());

        auto qos = ::dds::sub::qos::DataReaderQos();
        qos << ::dds::core::policy::History::KeepAll();

        m_reader = ::dds::sub::DataReader<Mempool::Chunk>(m_subscriber, m_topic, qos);

        LogDebug() << "[CycloneDataReader] Created data reader for topic: " << topicString;

        m_isConnected.store(true, std::memory_order_relaxed);
    }
}

iox::cxx::expected<uint8_t, iox::dds::DataReaderError> iox::dds::CycloneDataReader::read(uint8_t* const buffer, const uint64_t& bufferSize, const uint64_t& sampleSize)
{
    if(!m_isConnected.load())
    {
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::NOT_CONNECTED);
    }

    // Read up to the maximum number of samples that can fit in the buffer.
    auto capacity = bufferSize / sampleSize;
    auto samples = m_reader
            .select()
            .max_samples(capacity)
            .state(::dds::sub::status::SampleState::not_read())
            .take();

    LogDebug() << "[CycloneDataReader] Total samples: " << samples.length();

    // Copy data into the provided buffer.
    uint8_t cursor = 0;
    uint8_t numSamplesBuffered = 0;
    if(samples.length() > 0)
    {
        // Validation checks
        uint64_t size = samples.begin()->data().payload().size();
        if(size != sampleSize)
        {
            // Received invalid data.
            // NOTE: This causes other data points received in this read to be lost...
            return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::INVALID_DATA);
        }

        // Do copy
        for(const auto& sample : samples)
        {
            auto bytes = sample.data().payload().data();
            std::copy(bytes, bytes + sampleSize, &buffer[cursor]);
            cursor += sampleSize;
        }
        numSamplesBuffered = cursor / sampleSize;
    }

    LogDebug() << "[CycloneDataReader] New samples: " << numSamplesBuffered;
    return iox::cxx::success<uint8_t>(numSamplesBuffered);
}

iox::dds::IdString iox::dds::CycloneDataReader::getServiceId() const noexcept
{
    return m_serviceId;
}

iox::dds::IdString iox::dds::CycloneDataReader::getInstanceId() const noexcept
{
    return m_instanceId;
}

iox::dds::IdString iox::dds::CycloneDataReader::getEventId() const noexcept
{
    return m_eventId;
}
