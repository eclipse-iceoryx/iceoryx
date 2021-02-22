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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_dds/dds/cyclone_data_writer.hpp"
#include "iceoryx_dds/dds/cyclone_context.hpp"
#include "iceoryx_dds/internal/log/logging.hpp"

#include <Mempool_DCPS.hpp>
#include <string>

iox::dds::CycloneDataWriter::CycloneDataWriter(const capro::IdString_t serviceId,
                                               const capro::IdString_t instanceId,
                                               const capro::IdString_t eventId) noexcept
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
    m_publisher = ::dds::pub::Publisher(CycloneContext::getParticipant());
    auto topic = "/" + std::string(m_serviceId) + "/" + std::string(m_instanceId) + "/" + std::string(m_eventId);
    m_topic = ::dds::topic::Topic<Mempool::Chunk>(CycloneContext::getParticipant(), topic);
    m_writer = ::dds::pub::DataWriter<Mempool::Chunk>(m_publisher, m_topic);
    LogDebug() << "[CycloneDataWriter] Connected to topic: " << topic;
}

void iox::dds::CycloneDataWriter::write(const uint8_t* const bytes, const uint64_t size) noexcept
{
    auto chunk = Mempool::Chunk();
    std::copy(bytes, bytes + size, std::back_inserter(chunk.payload()));
    m_writer.write(chunk);
}

iox::capro::IdString_t iox::dds::CycloneDataWriter::getServiceId() const noexcept
{
    return m_serviceId;
}

iox::capro::IdString_t iox::dds::CycloneDataWriter::getInstanceId() const noexcept
{
    return m_instanceId;
}

iox::capro::IdString_t iox::dds::CycloneDataWriter::getEventId() const noexcept
{
    return m_eventId;
}
