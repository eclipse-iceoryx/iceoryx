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
    uint64_t index = 0u;
    for (auto& v : values)
    {
        data[index++] = v;
        if (index == 4u)
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
    for (size_t i = 0u; i < CLASS_HASH_ELEMENT_COUNT; ++i)
    {
        if (data[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

bool ServiceDescription::ClassHash::operator!=(const ClassHash& rhs) const noexcept
{
    return !operator==(rhs);
}

ServiceDescription::ServiceDescription(const cxx::Serialization& f_serial) noexcept
{
    std::underlying_type<Scope>::type scope;
    std::underlying_type<Interfaces>::type interfaceSource;
    f_serial.extract(m_serviceString,
                     m_instanceString,
                     m_eventString,
                     m_serviceID,
                     m_instanceID,
                     m_eventID,
                     m_classHash[0u],
                     m_classHash[1u],
                     m_classHash[2u],
                     m_classHash[3u],
                     m_hasServiceOnlyDescription,
                     scope,
                     interfaceSource);
    if (scope > static_cast<std::underlying_type<Scope>::type>(Scope::INVALID))
    {
        m_scope = Scope::INVALID;
    }
    else
    {
        m_scope = static_cast<Scope>(scope);
    }
    if (interfaceSource > static_cast<std::underlying_type<Interfaces>::type>(Interfaces::INTERFACE_END))
    {
        m_interfaceSource = Interfaces::INTERFACE_END;
    }
    else
    {
        m_interfaceSource = static_cast<Interfaces>(interfaceSource);
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
    , m_serviceString(iox::cxx::TruncateToCapacity, std::to_string(f_serviceID))
    , m_instanceString(iox::cxx::TruncateToCapacity, std::to_string(f_instanceID))
    , m_eventString(iox::cxx::TruncateToCapacity, std::to_string(f_eventID))
{
}

ServiceDescription::ServiceDescription(const IdString& f_service,
                                       const IdString& f_instance,
                                       const IdString& f_event,
                                       ClassHash f_classHash,
                                       Interfaces interfaceSource) noexcept
    : m_serviceString{f_service}
    , m_instanceString{f_instance}
    , m_eventString{f_event}
    , m_classHash(f_classHash)
    , m_interfaceSource(interfaceSource)
{
    if (!(cxx::convert::stringIsNumber(m_serviceString.c_str(), cxx::convert::NumberType::UNSIGNED_INTEGER)
          && cxx::convert::fromString(m_serviceString.c_str(), m_serviceID)))
    {
        m_serviceID = InvalidID;
    }

    if (!(cxx::convert::stringIsNumber(m_instanceString.c_str(), cxx::convert::NumberType::UNSIGNED_INTEGER)
          && cxx::convert::fromString(m_instanceString.c_str(), m_instanceID)))
    {
        m_instanceID = InvalidID;
    }

    if (!(cxx::convert::stringIsNumber(m_eventString.c_str(), cxx::convert::NumberType::UNSIGNED_INTEGER)
          && cxx::convert::fromString(m_eventString.c_str(), m_eventID)))
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
    auto serviceCompare = m_serviceString.compare(rhs.m_serviceString);
    if (serviceCompare != 0)
    {
        return 0 < serviceCompare;
    }

    auto instanceCompare = m_instanceString.compare(rhs.m_instanceString);
    if (instanceCompare != 0)
    {
        return 0 < instanceCompare;
    }

    auto eventCompare = m_eventString.compare(rhs.m_eventString);
    if (eventCompare != 0)
    {
        return 0 < eventCompare;
    }

    return false;
}

ServiceDescription::operator cxx::Serialization() const
{
    std::underlying_type<Scope>::type scope = static_cast<std::underlying_type<Scope>::type>(m_scope);
    std::underlying_type<Interfaces>::type interface =
        static_cast<std::underlying_type<Interfaces>::type>(m_interfaceSource);
    return cxx::Serialization::create(m_serviceString,
                                      m_instanceString,
                                      m_eventString,
                                      m_serviceID,
                                      m_instanceID,
                                      m_eventID,
                                      m_classHash[0u],
                                      m_classHash[1u],
                                      m_classHash[2u],
                                      m_classHash[3u],
                                      m_hasServiceOnlyDescription,
                                      scope,
                                      interface);
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

IdString ServiceDescription::getServiceIDString() const noexcept
{
    return m_serviceString;
}

IdString ServiceDescription::getInstanceIDString() const noexcept
{
    return m_instanceString;
}

IdString ServiceDescription::getEventIDString() const noexcept
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

Interfaces ServiceDescription::getSourceInterface() const noexcept
{
    return m_interfaceSource;
}

bool serviceMatch(const ServiceDescription& first, const ServiceDescription& second) noexcept
{
    return (first.getServiceID() == second.getServiceID());
}

} // namespace capro
} // namespace iox
