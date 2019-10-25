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

#include "iceoryx_posh/internal/popo/base_port.hpp"

namespace iox
{
namespace popo
{
std::atomic<uint64_t> BasePort::MemberType_t::s_uniqueIdCounter{1};

BasePort::BasePort(BasePortData* const f_basePortDataPtr) noexcept
    : m_basePortDataPtr(f_basePortDataPtr)
{
}

BasePort::BasePort(BasePort&& rhs) noexcept
{
    *this = std::move(rhs);
}

BasePort& BasePort::operator=(BasePort&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_basePortDataPtr = rhs.m_basePortDataPtr;
        rhs.m_basePortDataPtr = nullptr;
    }
    return *this;
}

BasePort::~BasePort() noexcept
{
}

BasePortType BasePort::getPortType() const noexcept
{
    return getMembers()->m_portType;
}

capro::ServiceDescription BasePort::getCaProServiceDescription() const noexcept
{
    return getMembers()->m_serviceDescription;
}

cxx::CString100 BasePort::getApplicationName() const noexcept
{
    return getMembers()->m_processName;
}

Interfaces BasePort::getInterface() const noexcept
{
    return getMembers()->m_interface;
}

uint64_t BasePort::getUniqueID() const noexcept
{
    return getMembers()->m_uniqueId.load(std::memory_order_relaxed);
}

BasePort::operator bool() const
{
    return m_basePortDataPtr != nullptr;
}

} // namespace popo
} // namespace iox
