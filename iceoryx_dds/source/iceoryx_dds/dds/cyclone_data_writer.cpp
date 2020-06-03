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

#include <chrono>
#include <string>
#include <thread>

#include "Mempool_DCPS.hpp"
#include "iceoryx_dds/dds/cyclone_data_writer.hpp"
#include "iceoryx_dds/internal/log/logging.hpp"

iox::dds::CycloneDataWriter::CycloneDataWriter(IdString serviceId, IdString instanceId, IdString eventId)
    : m_serviceId(serviceId)
    , m_instanceId(instanceId)
    , m_eventId(eventId)
{
    LogDebug() << "[CycloneDataWriter] Created CycloneDataWriter.";
}

iox::dds::CycloneDataWriter::~CycloneDataWriter()
{
    m_writer.close();
    m_topic.close();
    m_publisher.close();
    LogDebug() << "[CycloneDataWriter] Destroyed CycloneDataWriter.";
}

void iox::dds::CycloneDataWriter::connect() noexcept
{
    m_publisher = ::dds::pub::Publisher(getParticipant());
    auto topic = "/" + std::string(m_serviceId) + "/" + std::string(m_instanceId) + "/" + std::string(m_eventId);
    m_topic = ::dds::topic::Topic<Mempool::Chunk>(getParticipant(), topic);
    m_writer = ::dds::pub::DataWriter<Mempool::Chunk>(m_publisher, m_topic);
    LogDebug() << "[CycloneDataWriter] Connected to topic: " << topic;
}

void iox::dds::CycloneDataWriter::write(const uint8_t* const bytes, const uint64_t size) noexcept
{
    LogDebug() << "[CycloneDataWriter] Writing " << size << " bytes.";
    auto chunk = Mempool::Chunk();
    std::copy(bytes, bytes + size, std::back_inserter(chunk.payload()));
    m_writer.write(chunk);
}

std::string iox::dds::CycloneDataWriter::getServiceId() const noexcept
{
    return m_serviceId;
};
std::string iox::dds::CycloneDataWriter::getInstanceId() const noexcept
{
    return m_instanceId;
};
std::string iox::dds::CycloneDataWriter::getEventId() const noexcept
{
    return m_eventId;
};

::dds::domain::DomainParticipant& iox::dds::CycloneDataWriter::getParticipant() noexcept
{
    static auto participant = ::dds::domain::DomainParticipant(org::eclipse::cyclonedds::domain::default_id());
    return participant;
}
