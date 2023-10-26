// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iox/detail/convert.hpp"
#include "iox/std_string_support.hpp"

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
    uint64_t index = 0U;
    for (auto& v : values)
    {
        data[index++] = v;
        if (index == CLASS_HASH_ELEMENT_COUNT)
        {
            return;
        }
    }
}

uint32_t&
ServiceDescription::ClassHash::operator[](iox::range<uint64_t, 0U, CLASS_HASH_ELEMENT_COUNT - 1> index) noexcept
{
    return data[index];
}

const uint32_t&
ServiceDescription::ClassHash::operator[](iox::range<uint64_t, 0U, CLASS_HASH_ELEMENT_COUNT - 1> index) const noexcept
{
    return data[index];
}

bool ServiceDescription::ClassHash::operator==(const ClassHash& rhs) const noexcept
{
    for (size_t i = 0U; i < CLASS_HASH_ELEMENT_COUNT; ++i)
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

ServiceDescription::ServiceDescription() noexcept
    : ServiceDescription("", "", "")
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

bool ServiceDescription::operator==(const ServiceDescription& rhs) const noexcept
{
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

bool ServiceDescription::operator!=(const ServiceDescription& rhs) const noexcept
{
    return !(*this == rhs);
}

bool ServiceDescription::operator<(const ServiceDescription& rhs) const noexcept
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

ServiceDescription::operator Serialization() const noexcept
{
    std::underlying_type<Scope>::type scope = static_cast<std::underlying_type<Scope>::type>(m_scope);
    std::underlying_type<Interfaces>::type interface =
        static_cast<std::underlying_type<Interfaces>::type>(m_interfaceSource);
    return Serialization::create(m_serviceString,
                                 m_instanceString,
                                 m_eventString,
                                 m_classHash[0U],
                                 m_classHash[1U],
                                 m_classHash[2U],
                                 m_classHash[3U],
                                 scope,
                                 interface);
}

expected<ServiceDescription, Serialization::Error>
ServiceDescription::deserialize(const Serialization& serialized) noexcept
{
    ServiceDescription deserializedObject;

    using ScopeUnderlyingType = std::underlying_type<Scope>::type;
    using InterfaceUnderlyingType = std::underlying_type<Interfaces>::type;

    ScopeUnderlyingType scope{0};
    InterfaceUnderlyingType interfaceSource{0};

    auto deserializationSuccessful = serialized.extract(deserializedObject.m_serviceString,
                                                        deserializedObject.m_instanceString,
                                                        deserializedObject.m_eventString,
                                                        deserializedObject.m_classHash[0U],
                                                        deserializedObject.m_classHash[1U],
                                                        deserializedObject.m_classHash[2U],
                                                        deserializedObject.m_classHash[3U],
                                                        scope,
                                                        interfaceSource);
    if (!deserializationSuccessful || scope >= static_cast<ScopeUnderlyingType>(Scope::INVALID)
        || interfaceSource >= static_cast<InterfaceUnderlyingType>(Interfaces::INTERFACE_END))
    {
        return err(Serialization::Error::DESERIALIZATION_FAILED);
    }

    deserializedObject.m_scope = static_cast<Scope>(scope);
    deserializedObject.m_interfaceSource = static_cast<Interfaces>(interfaceSource);

    return ok(deserializedObject);
}

const IdString_t& ServiceDescription::getServiceIDString() const noexcept
{
    return m_serviceString;
}

const IdString_t& ServiceDescription::getInstanceIDString() const noexcept
{
    return m_instanceString;
}

const IdString_t& ServiceDescription::getEventIDString() const noexcept
{
    return m_eventString;
}

bool ServiceDescription::isLocal() const noexcept
{
    return m_scope == Scope::LOCAL;
}

void ServiceDescription::setLocal() noexcept
{
    m_scope = Scope::LOCAL;
}

Scope ServiceDescription::getScope() const noexcept
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
    return (first.getServiceIDString() == second.getServiceIDString());
}

std::ostream& operator<<(std::ostream& stream, const ServiceDescription& service) noexcept
{
    /// @todo iox-#1141 Add classHash, scope and interface
    stream << "Service: " << service.getServiceIDString() << ", Instance: " << service.getInstanceIDString()
           << ", Event: " << service.getEventIDString();
    return stream;
}

log::LogStream& operator<<(log::LogStream& stream, const ServiceDescription& service) noexcept
{
    /// @todo iox-#1141 Add classHash, scope and interface
    stream << "Service: " << service.getServiceIDString() << ", Instance: " << service.getInstanceIDString()
           << ", Event: " << service.getEventIDString();
    return stream;
}

} // namespace capro
} // namespace iox
