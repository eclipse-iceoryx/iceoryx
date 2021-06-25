// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_hoofs/cxx/convert.hpp"
#include <iomanip>

namespace iox
{
namespace capro
{
ServiceDescription::ClassHash::ClassHash() noexcept
    : ClassHash{0U, 0U, 0U, 0U}
{
}

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

uint32_t& ServiceDescription::ClassHash::operator[](
    iox::cxx::range<uint64_t, 0U, CLASS_HASH_ELEMENT_COUNT - 1> index) noexcept
{
    return data[index];
}

const uint32_t&
    ServiceDescription::ClassHash::operator[](iox::cxx::range<uint64_t, 0U, CLASS_HASH_ELEMENT_COUNT - 1> index) const
    noexcept
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

ServiceDescription::ServiceDescription(const cxx::Serialization& serial) noexcept
{
    std::underlying_type<Scope>::type scope = 0;
    std::underlying_type<Interfaces>::type interfaceSource = 0;
    serial.extract(m_serviceString,
                   m_instanceString,
                   m_eventString,
                   m_classHash[0u],
                   m_classHash[1u],
                   m_classHash[2u],
                   m_classHash[3u],
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
    : ServiceDescription(InvalidIdString, InvalidIdString, InvalidIdString)
{
}

ServiceDescription::ServiceDescription(const IdString_t& service,
                                       const IdString_t& instance,
                                       const IdString_t& event,
                                       ClassHash classHash,
                                       Interfaces interfaceSource) noexcept
    : m_serviceString{service}
    , m_instanceString{instance}
    , m_eventString{event}
    , m_classHash(classHash)
    , m_interfaceSource(interfaceSource)
{
}

bool ServiceDescription::operator==(const ServiceDescription& rhs) const
{
    if (!isValid() || !rhs.isValid())
    {
        return false;
    }

    if (m_serviceString != rhs.m_serviceString)
    {
        return false;
    }

    if (m_instanceString != rhs.m_instanceString)
    {
        return false;
    }

    if (m_eventString != rhs.m_eventString)
    {
        return false;
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
                                      m_classHash[0u],
                                      m_classHash[1u],
                                      m_classHash[2u],
                                      m_classHash[3u],
                                      scope,
                                      interface);
}

IdString_t ServiceDescription::getServiceIDString() const noexcept
{
    return m_serviceString;
}

IdString_t ServiceDescription::getInstanceIDString() const noexcept
{
    return m_instanceString;
}

IdString_t ServiceDescription::getEventIDString() const noexcept
{
    return m_eventString;
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

bool ServiceDescription::isValid() const noexcept
{
    return !(m_serviceString == iox::capro::InvalidIdString || m_instanceString == iox::capro::InvalidIdString
             || m_eventString == iox::capro::InvalidIdString);
}

bool serviceMatch(const ServiceDescription& first, const ServiceDescription& second) noexcept
{
    return (first.getServiceIDString() == second.getServiceIDString());
}

} // namespace capro
} // namespace iox
