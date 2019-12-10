// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
// limitations under the License

#include "iceoryx_posh/capro/service_description.hpp"

#include <iomanip>

namespace iox
{
namespace capro
{
ServiceDescription::ClassHash::ClassHash(const std::initializer_list<uint32_t>& values) noexcept
{
    uint64_t index = 0;
    for (auto& v : values)
    {
        data[index++] = v;
        if (index == 4)
        {
            return;
        }
    }
}

uint32_t& ServiceDescription::ClassHash::operator[](const uint64_t index) noexcept
{
    return data[index];
}

const uint32_t& ServiceDescription::ClassHash::operator[](const uint64_t index) const noexcept
{
    return data[index];
}

bool ServiceDescription::ClassHash::operator==(const ClassHash& rhs) const noexcept
{
    for (size_t i = 0; i < CLASS_HASH_ELEMENT_COUNT; ++i)
    {
        if (data[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

ServiceDescription::ServiceDescription(const cxx::Serialization& f_serial) noexcept
{
    std::underlying_type<Scope>::type scope;
    f_serial.extract(m_serviceString,
                     m_instanceString,
                     m_eventString,
                     m_serviceID,
                     m_instanceID,
                     m_eventID,
                     m_classHash[0],
                     m_classHash[1],
                     m_classHash[2],
                     m_classHash[3],
                     m_hasServiceOnlyDescription,
                     scope);
    if (scope > static_cast<std::underlying_type<Scope>::type>(Scope::INVALID))
    {
        m_scope = Scope::INVALID;
    }
    else
    {
        m_scope = static_cast<Scope>(scope);
    }
}

ServiceDescription::ServiceDescription() noexcept
    : ServiceDescription(InvalidID, InvalidID, InvalidID)
{
}

ServiceDescription::ServiceDescription(uint16_t f_serviceID, uint16_t f_instanceID) noexcept
    : ServiceDescription(f_serviceID, InvalidID, f_instanceID)
{
    m_hasServiceOnlyDescription = true;
}

ServiceDescription::ServiceDescription(const IdString& f_service, const IdString& f_instance) noexcept
    : ServiceDescription(f_service, f_instance, InvalidIDString)
{
    m_hasServiceOnlyDescription = true;
}

ServiceDescription::ServiceDescription(uint16_t f_serviceID, uint16_t f_eventID, uint16_t f_instanceID) noexcept
    : m_serviceID(f_serviceID)
    , m_eventID(f_eventID)
    , m_instanceID(f_instanceID)
    , m_serviceString(std::to_string(f_serviceID).c_str())
    , m_instanceString(std::to_string(f_instanceID).c_str())
    , m_eventString(std::to_string(f_eventID).c_str())
{
}

ServiceDescription::ServiceDescription(const IdString& f_service,
                                       const IdString& f_instance,
                                       const IdString& f_event,
                                       ClassHash f_classHash) noexcept
    : m_serviceString{f_service}
    , m_instanceString{f_instance}
    , m_eventString{f_event}
    , m_classHash(f_classHash)
{
    if (cxx::convert::stringIsNumber(m_serviceString.to_cstring(), cxx::convert::NumberType::UNSIGNED_INTEGER))
    {
        cxx::convert::fromString(m_serviceString.to_cstring(), m_serviceID);
    }
    else
    {
        m_serviceID = InvalidID;
    }

    if (cxx::convert::stringIsNumber(m_instanceString.to_cstring(), cxx::convert::NumberType::UNSIGNED_INTEGER))
    {
        cxx::convert::fromString(m_instanceString.to_cstring(), m_instanceID);
    }
    else
    {
        m_instanceID = InvalidID;
    }

    if (cxx::convert::stringIsNumber(m_eventString.to_cstring(), cxx::convert::NumberType::UNSIGNED_INTEGER))
    {
        cxx::convert::fromString(m_eventString.to_cstring(), m_eventID);
    }
    else
    {
        m_eventID = InvalidID;
    }
}

bool ServiceDescription::operator==(const ServiceDescription& rhs) const
{
    if ((m_serviceID != AnyService) && (rhs.m_serviceID != AnyService))
    {
        if ((m_serviceID != rhs.m_serviceID) || (m_serviceString != rhs.m_serviceString))
        {
            return false;
        }
    }

    if ((m_instanceID != AnyInstance) && (rhs.m_instanceID != AnyInstance))
    {
        if ((m_instanceID != rhs.m_instanceID) || (m_instanceString != rhs.m_instanceString))
        {
            return false;
        }
    }

    if (m_eventID != AnyEvent && rhs.m_eventID != AnyEvent)
    {
        if ((m_eventID != rhs.m_eventID) || (m_eventString != rhs.m_eventString))
        {
            return false;
        }
    }
    return true;
}

bool ServiceDescription::operator!=(const ServiceDescription& rhs) const
{
    return !(*this == rhs);
}

bool ServiceDescription::operator<(const ServiceDescription& rhs) const
{
    int32_t serviceCompare = m_serviceString.compare(rhs.m_serviceString);
    if (serviceCompare != 0)
    {
        return 0 < serviceCompare;
    }

    int32_t instanceCompare = m_instanceString.compare(rhs.m_instanceString);
    if (instanceCompare != 0)
    {
        return 0 < instanceCompare;
    }

    int32_t eventCompare = m_eventString.compare(rhs.m_eventString);
    if (eventCompare != 0)
    {
        return 0 < eventCompare;
    }

    return false;
}

ServiceDescription& ServiceDescription::operator=(const ServiceDescription& other)
{
    m_serviceID = other.m_serviceID;
    m_instanceID = other.m_instanceID;
    m_eventID = other.m_eventID;
    m_serviceString = other.m_serviceString;
    m_instanceString = other.m_instanceString;
    m_eventString = other.m_eventString;
    m_hasServiceOnlyDescription = other.m_hasServiceOnlyDescription;
    m_classHash = other.m_classHash;
    m_scope = other.m_scope;

    return *this;
}

ServiceDescription::operator cxx::Serialization() const
{
    std::underlying_type<Scope>::type scope = static_cast<std::underlying_type<Scope>::type>(m_scope);
    return cxx::Serialization::create(m_serviceString,
                                      m_instanceString,
                                      m_eventString,
                                      m_serviceID,
                                      m_instanceID,
                                      m_eventID,
                                      m_classHash[0],
                                      m_classHash[1],
                                      m_classHash[2],
                                      m_classHash[3],
                                      m_hasServiceOnlyDescription,
                                      scope);
}

std::string ServiceDescription::getServiceString() const noexcept
{
    std::stringstream l_strStream;
    l_strStream << "Service_" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << m_serviceID << "_"
                << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << m_eventID << "_" << std::uppercase
                << std::setfill('0') << std::setw(4) << std::hex << m_instanceID;
    return l_strStream.str();
}

uint16_t ServiceDescription::getInstanceID() const noexcept
{
    return m_instanceID;
}

uint16_t ServiceDescription::getServiceID() const noexcept
{
    return m_serviceID;
}

uint16_t ServiceDescription::getEventID() const noexcept
{
    return m_eventID;
}

ServiceDescription::IdString ServiceDescription::getServiceIDString() const noexcept
{
    return m_serviceString;
}

ServiceDescription::IdString ServiceDescription::getInstanceIDString() const noexcept
{
    return m_instanceString;
}

ServiceDescription::IdString ServiceDescription::getEventIDString() const noexcept
{
    return m_eventString;
}

bool ServiceDescription::hasServiceOnlyDescription() const noexcept
{
    return m_hasServiceOnlyDescription;
}


bool ServiceDescription::isInternal() const noexcept
{
    return m_scope == Scope::INTERNAL;
}

void ServiceDescription::setInternal() noexcept
{
    m_scope = Scope::INTERNAL;
}

Scope ServiceDescription::getScope() noexcept
{
    return m_scope;
}

ServiceDescription::ClassHash ServiceDescription::getClassHash() const noexcept
{
    return m_classHash;
}

bool serviceMatch(const ServiceDescription& first, const ServiceDescription& second) noexcept
{
    return (first.getServiceID() == second.getServiceID());
}

} // namespace capro
} // namespace iox
