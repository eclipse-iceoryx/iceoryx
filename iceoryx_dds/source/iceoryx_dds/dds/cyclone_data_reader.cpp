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

}

uint8_t iox::dds::CycloneDataReader::read(const uint8_t* buffer) const noexcept
{

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
