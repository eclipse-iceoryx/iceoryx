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
        m_reader = ::dds::sub::DataReader<Mempool::Chunk>(m_subscriber, m_topic);

        LogDebug() << "[CycloneDataReader] Created data reader for topic: " << topicString;

        m_isConnected.store(true, std::memory_order_relaxed);
    }
}

iox::cxx::expected<uint8_t, iox::dds::DataReaderError> iox::dds::CycloneDataReader::read(uint8_t* const buffer, const uint64_t& size)
{
    if(!m_isConnected.load())
    {
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::NOT_CONNECTED);
    }

    // Read all available data points ...
    auto samples = m_reader.read();
    LogDebug() << "[CycloneDataReader] Read samples: " << samples.length();

    uint8_t cursor = 0;
    uint8_t numSamplesBuffered = 0;
    if(samples.length() > 0)
    {
        uint64_t sampleSize = samples.begin()->data().payload().size();
        for(const auto& sample : samples)
        {
            // Check there is another space in the buffer.
            if(cursor < size-sampleSize)
            {
                auto bytes = sample.data().payload().data();
                std::copy(bytes, bytes + sampleSize, &buffer[cursor]);
                cursor += sampleSize;
            }
            else
            {
                // Buffer full.
                // TODO - what to do with unread samples ? Probably nothing (and leave their read flag unset).
            }
        }
        numSamplesBuffered = cursor / sampleSize;
    }

    LogDebug() << "[CycloneDataReader] Buffered samples: " << numSamplesBuffered;
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
